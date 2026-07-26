/*
 * Cantata
 *
 * Copyright (c) Bernd Steinhauser <berniyh@exherbo.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "lyricsmarkup.h"
#include <QMap>
#include <QObject>
#include <QRegularExpression>

namespace LyricsMarkup {

const QLatin1String constAnnotationUrl("cantata:///?annotation=");
const QLatin1String constAnnotationQuery("annotation");

// The name of the element a tag opens or closes, lower cased.
static QString tagName(const QString& tag)
{
	int pos = 1;

	if (pos < tag.length() && QLatin1Char('/') == tag.at(pos)) {
		pos++;
	}

	int start = pos;
	while (pos < tag.length() && tag.at(pos).isLetterOrNumber()) {
		pos++;
	}

	return tag.mid(start, pos - start).toLower();
}

static bool isClosingTag(const QString& tag)
{
	return tag.length() > 1 && QLatin1Char('/') == tag.at(1);
}

// Genius points an annotated phrase at '/<id>/<song>', sometimes fully qualified. Anything else is
// an ordinary link, which we have no annotation for.
static QString referentId(const QString& tag)
{
	static const QRegularExpression constHref(QLatin1String("\\bhref\\s*=\\s*[\"']([^\"']*)[\"']"), QRegularExpression::CaseInsensitiveOption);
	static const QRegularExpression constReferent(QLatin1String("^(?:https?://(?:www\\.)?genius\\.com)?/(\\d+)/"));

	QRegularExpressionMatch href = constHref.match(tag);

	if (!href.hasMatch()) {
		return QString();
	}

	QRegularExpressionMatch referent = constReferent.match(href.captured(1));
	return referent.hasMatch() ? referent.captured(1) : QString();
}

QString styleSectionHeaders(const QString& lyrics)
{
	static const QRegularExpression constHeader(QLatin1String("^\\s*\\[([^\\]]{1,80})\\]\\s*$"));

	QStringList lines = lyrics.split(QLatin1Char('\n'));

	for (QString& line : lines) {
		QRegularExpressionMatch match = constHeader.match(line);
		if (match.hasMatch()) {
			line = QLatin1String("<b>[") + match.captured(1).toHtmlEscaped() + QLatin1String("]</b>");
		}
	}

	return lines.join(QLatin1Char('\n'));
}

Prepared prepare(const QString& providerText)
{
	Prepared prepared;

	// Normalised the same way the plain text path normalises it, so that both end up with the same
	// lines. A newline in the markup is a line break here just as it is there.
	QString source(providerText);
	source.replace(QLatin1String("\\n"), QLatin1String("\n"));
	source.replace(QLatin1String("\\t"), QLatin1String(" "));
	source.replace(QLatin1Char('\t'), QLatin1Char(' '));
	source.replace(QLatin1String("\n\n\n"), QLatin1String("\n\n"));

	QString line;
	QString openId;

	for (int pos = 0; pos < source.length();) {
		QChar ch = source.at(pos);

		if (QLatin1Char('\n') == ch) {
			prepared.lines.append(styleSectionHeaders(line));
			line.clear();
			pos++;
			continue;
		}

		if (QLatin1Char('<') != ch) {
			line += ch;
			pos++;
			continue;
		}

		int close = source.indexOf(QLatin1Char('>'), pos);

		if (-1 == close) {
			// Not a tag, just a '<' the lyric happens to contain.
			line += QLatin1String("&lt;");
			pos++;
			continue;
		}

		QString tag = source.mid(pos, close - pos + 1);
		QString name = tagName(tag);
		pos = close + 1;

		if (QLatin1String("br") == name) {
			prepared.lines.append(styleSectionHeaders(line));
			line.clear();
		}
		else if (QLatin1String("script") == name || QLatin1String("style") == name) {
			// These are the only elements whose text we drop along with the tag.
			if (!isClosingTag(tag)) {
				int end = source.indexOf(QLatin1String("</") + name, pos, Qt::CaseInsensitive);
				pos = -1 == end ? source.length() : end;
			}
		}
		else if (QLatin1String("b") == name || QLatin1String("strong") == name) {
			line += isClosingTag(tag) ? QLatin1String("</b>") : QLatin1String("<b>");
		}
		else if (QLatin1String("i") == name || QLatin1String("em") == name) {
			line += isClosingTag(tag) ? QLatin1String("</i>") : QLatin1String("<i>");
		}
		else if (QLatin1String("a") == name) {
			if (isClosingTag(tag)) {
				if (!openId.isEmpty()) {
					line += QLatin1String("</a>");
					// Recorded against the line the phrase ends on, not the one it starts on, so
					// that a phrase running over a line break is annotated below all of it.
					prepared.refs.append(qMakePair(openId, prepared.lines.count()));
					openId.clear();
				}
			}
			else if (openId.isEmpty()) {
				openId = referentId(tag);
				if (!openId.isEmpty()) {
					line += QLatin1String("<a href=\"") + constAnnotationUrl + openId + QLatin1String("\">");
				}
			}
		}
		// Everything else is page furniture. Drop the tag, but keep whatever text it wraps.
	}

	prepared.lines.append(styleSectionHeaders(line));
	return prepared;
}

QString render(const Prepared& prepared, const QHash<QString, QString>& bodies, const QSet<QString>& expanded)
{
	QMap<int, QStringList> opened;

	for (const QPair<QString, int>& ref : prepared.refs) {
		if (expanded.contains(ref.first)) {
			opened[ref.second].append(ref.first);
		}
	}

	QString html;
	// An annotation is a block of its own, so the line after it needs no separator.
	bool afterBlock = true;

	for (int i = 0; i < prepared.lines.count(); ++i) {
		if (!afterBlock) {
			html += QLatin1String("<br/>");
		}
		html += prepared.lines.at(i);
		afterBlock = false;

		for (const QString& id : opened.value(i)) {
			html += QLatin1String("<blockquote>")
					+ (bodies.contains(id) ? bodies.value(id) : QObject::tr("Loading annotation..."))
					+ QLatin1String("</blockquote>");
			afterBlock = true;
		}
	}

	return html;
}

}// namespace LyricsMarkup
