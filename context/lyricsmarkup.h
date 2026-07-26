/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
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

#ifndef LYRICSMARKUP_H
#define LYRICSMARKUP_H

#include <QHash>
#include <QList>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>

/**
 * Turns what a lyrics provider returned into something we can display, for the providers whose
 * output is structured enough to be worth more than plain text.
 *
 * Genius wraps the phrases people have annotated in links carrying the annotation's id. Those links
 * survive the scraping rules intact, so the annotated ranges are already known here - they only
 * need to be kept rather than flattened away with the rest of the page's markup.
 */
namespace LyricsMarkup {

/**
 * Lyrics broken into display ready lines, with the annotated phrases linked.
 */
struct Prepared {
	/** One entry per lyric line, holding the small subset of HTML we allow through. */
	QStringList lines;

	/** Annotation id, paired with the index of the line the annotated phrase ends on. */
	QList<QPair<QString, int>> refs;

	bool hasAnnotations() const { return !refs.isEmpty(); }

	void clear()
	{
		lines.clear();
		refs.clear();
	}
};

/** The scheme and query a click on an annotated phrase arrives with. */
extern const QLatin1String constAnnotationUrl;
extern const QLatin1String constAnnotationQuery;

/**
 * Reduces the provider's markup to line, emphasis and annotation links, dropping everything else
 * while keeping its text.
 *
 * @param providerText What the extract and exclude rules produced.
 *
 * @return The lyrics, and where in them the annotations belong.
 */
Prepared prepare(const QString& providerText);

/**
 * Builds the page for the lyrics, showing the annotations the user has opened.
 *
 * @param prepared The lyrics, as returned by prepare().
 * @param bodies The annotation texts fetched so far, keyed by id. An id that is missing is still
 *               on its way, and is shown as such.
 * @param expanded The ids the user has opened.
 *
 * @return The body of the lyrics page.
 */
QString render(const Prepared& prepared, const QHash<QString, QString>& bodies, const QSet<QString>& expanded);

/**
 * Emphasises the section markers - '[Verse 1]', '[Chorus]' - that some providers put on a line of
 * their own, so that they read as structure rather than as part of the lyric. Only a line that is
 * nothing but a marker qualifies; bracketed asides within a line ('[sic]', '[?]') are left alone.
 *
 * For the plain text path - prepare() emphasises the markers it comes across itself, keeping the
 * markup they carry.
 *
 * @param lyrics One or more newline separated lines of plain text.
 *
 * @return The lines, with any section markers emphasised.
 */
QString styleSectionHeaders(const QString& lyrics);

}// namespace LyricsMarkup

#endif// LYRICSMARKUP_H
