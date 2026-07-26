/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
 *
 */
/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ultimatelyricsprovider.h"
#include "network/networkaccessmanager.h"
#include <QDebug>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
static bool debugEnabled = false;
#define DBUG \
	if (debugEnabled) qWarning() << "Lyrics" << __FUNCTION__
void UltimateLyricsProvider::enableDebug()
{
	debugEnabled = true;
}

static const QString constArtistArg = QLatin1String("{Artist}");
static const QString constArtistLowerArg = QLatin1String("{artist}");
static const QString constArtistLowerNoSpaceArg = QLatin1String("{artist2}");
static const QString constArtistFirstCharArg = QLatin1String("{a}");
static const QString constAlbumArg = QLatin1String("{Album}");
static const QString constAlbumLowerArg = QLatin1String("{album}");
static const QString constAlbumLowerNoSpaceArg = QLatin1String("{album2}");
static const QString constTitleLowerArg = QLatin1String("{title}");
static const QString constTitleArg = QLatin1String("{Title}");
static const QString constTitleCaseArg = QLatin1String("{Title2}");
static const QString constYearArg = QLatin1String("{year}");
static const QString constTrackNoArg = QLatin1String("{track}");
static const QString constThe = QLatin1String("The ");

static QString noSpace(const QString& text)
{
	QString ret(text);
	ret.remove(' ');
	return ret;
}

static QString firstChar(const QString& text)
{
	return text.isEmpty() ? text : text[0].toLower();
}

static QString titleCase(const QString& text)
{
	if (0 == text.length()) {
		return QString();
	}
	if (1 == text.length()) {
		return text[0].toUpper();
	}
	return text[0].toUpper() + text.right(text.length() - 1).toLower();
}

static QString doTagReplace(QString str, const Song& song)
{
	if (str.contains(QLatin1Char('{'))) {
		QString artistFixed = song.basicArtist();
		str.replace(constArtistArg, artistFixed);
		str.replace(constArtistFirstCharArg, firstChar(artistFixed));
		str.replace(constAlbumArg, song.album);
		str.replace(constTitleArg, song.basicTitle());
		str.replace(constYearArg, QString::number(song.year));
		str.replace(constTrackNoArg, QString::number(song.track));
	}
	return str;
}

static QString extract(const QString& source, const QString& begin, const QString& end, bool isTag = false)
{
	DBUG << "Looking for" << begin << end;
	int beginIdx = source.indexOf(begin, 0, Qt::CaseInsensitive);
	bool skipTagClose = false;

	if (-1 == beginIdx && isTag) {
		beginIdx = source.indexOf(QString(begin).remove(">"), 0, Qt::CaseInsensitive);
		skipTagClose = true;
	}
	if (-1 == beginIdx) {
		DBUG << "Failed to find begin";
		return QString();
	}
	if (skipTagClose) {
		int closeIdx = source.indexOf(">", beginIdx);
		if (-1 != closeIdx) {
			beginIdx = closeIdx + 1;
		}
		else {
			beginIdx += begin.length();
		}
	}
	else {
		beginIdx += begin.length();
	}

	int endIdx = source.indexOf(end, beginIdx, Qt::CaseInsensitive);
	if (-1 == endIdx && QLatin1String("null") != end) {
		DBUG << "Failed to find end";
		return QString();
	}

	DBUG << "Found match";
	return source.mid(beginIdx, endIdx - beginIdx);
}

static QRegularExpression xmlTagRegex = QRegularExpression("<(\\w+).*>");
static QString extractXmlTag(const QString& source, const QString& tag)
{
	DBUG << "Looking for" << tag;
	auto match = xmlTagRegex.match(tag);
	if (!match.hasMatch()) {
		DBUG << "Failed to find tag";
		return QString();
	}

	DBUG << "Found match";
	return extract(source, tag, "</" + match.captured(1) + ">", true);
}

// Decode a JSON string literal. Sites that render client-side (e.g. Musixmatch) embed the
// lyrics in a JSON blob rather than in markup, so what the extract rules pull out still has
// its escapes intact.
static QString jsonUnescape(const QString& source)
{
	QString ret;
	ret.reserve(source.length());

	for (int i = 0; i < source.length(); ++i) {
		if (QLatin1Char('\\') != source.at(i) || i + 1 >= source.length()) {
			ret += source.at(i);
			continue;
		}

		QChar esc = source.at(++i);
		switch (esc.toLatin1()) {
		case 'n': ret += QLatin1Char('\n'); break;
		case 't': ret += QLatin1Char('\t'); break;
		case 'r': ret += QLatin1Char('\r'); break;
		case 'b': ret += QLatin1Char('\b'); break;
		case 'f': ret += QLatin1Char('\f'); break;
		case 'u': {
			bool ok = false;
			ushort code = i + 4 < source.length() ? source.mid(i + 1, 4).toUShort(&ok, 16) : 0;
			if (ok) {
				ret += QChar(code);
				i += 4;
			}
			else {
				ret += esc;
			}
			break;
		}
		// Covers \" \\ \/ - and leaves anything unrecognised as-is.
		default: ret += esc; break;
		}
	}
	return ret;
}

// Matches the opening tag of an element carrying the named attribute. The attribute must be a
// whitespace-separated name followed by '=', so that a name is not matched by a longer one that
// merely starts with it - a word boundary would not do, as '-' ends a word.
static QRegularExpression containerRegex(const QString& attribute)
{
	return QRegularExpression("<(\\w+)\\b[^>]*\\s" + QRegularExpression::escape(attribute) + "\\s*=[^>]*>",
	                          QRegularExpression::CaseInsensitiveOption);
}

// Given the opening tag matched by containerRegex(), walks the same tag in and out until the
// element's own closing tag is reached, so that nested markup does not terminate it early.
// Sets contentEnd to the start of that closing tag, and elementEnd to just past it.
static bool findContainerEnd(const QString& source, const QRegularExpressionMatch& open, int& contentEnd, int& elementEnd)
{
	QRegularExpression nestRegex("<(/?)" + open.captured(1) + "\\b[^>]*>", QRegularExpression::CaseInsensitiveOption);
	int scan = open.capturedEnd();
	int depth = 1;

	while (depth > 0) {
		auto nest = nestRegex.match(source, scan);
		if (!nest.hasMatch()) {
			DBUG << "Failed to find end of container";
			return false;
		}
		if (nest.captured(1).isEmpty()) {
			++depth;
		}
		else if (0 == --depth) {
			contentEnd = nest.capturedStart();
			elementEnd = nest.capturedEnd();
		}
		scan = nest.capturedEnd();
	}
	return true;
}

// Extract the contents of every element carrying the named attribute. Used for sites (e.g.
// Genius) that split the lyrics over several containers whose class names are build-specific,
// and whose contents hold nested markup - so neither a literal tag match nor a naive begin/end
// pair works.
static QString extractContainers(const QString& source, const QString& attribute)
{
	DBUG << "Looking for containers with" << attribute;
	QRegularExpression openRegex = containerRegex(attribute);
	QStringList parts;
	int pos = 0;

	while (pos < source.length()) {
		auto open = openRegex.match(source, pos);
		if (!open.hasMatch()) {
			break;
		}

		int contentEnd = -1;
		int elementEnd = -1;
		if (!findContainerEnd(source, open, contentEnd, elementEnd)) {
			break;
		}

		// Empty containers are used as placeholders around ad slots - skip them, so that
		// joining does not introduce stray breaks between the sections that do have lyrics.
		QString content = source.mid(open.capturedEnd(), contentEnd - open.capturedEnd());
		if (!content.trimmed().isEmpty()) {
			parts << content;
		}
		pos = elementEnd;
	}

	if (parts.isEmpty()) {
		DBUG << "Failed to find container";
		return QString();
	}

	DBUG << "Found" << parts.length() << "container(s)";
	return parts.join(QLatin1String("<br/>"));
}

// Remove every element carrying the named attribute, contents included. Genius marks the
// page furniture it embeds within the lyrics containers (contributor count, song title,
// summary) with such an attribute, and it has to go along with the markup it sits in.
static QString excludeContainers(const QString& source, const QString& attribute)
{
	DBUG << "Looking for containers with" << attribute;
	QRegularExpression openRegex = containerRegex(attribute);
	QString ret = source;
	int removed = 0;
	int pos = 0;

	while (pos < ret.length()) {
		auto open = openRegex.match(ret, pos);
		if (!open.hasMatch()) {
			break;
		}

		int contentEnd = -1;
		int elementEnd = -1;
		if (!findContainerEnd(ret, open, contentEnd, elementEnd)) {
			break;
		}

		// Any nested match is removed along with this one, so resume from where it started.
		ret.remove(open.capturedStart(), elementEnd - open.capturedStart());
		pos = open.capturedStart();
		++removed;
	}

	DBUG << "Removed" << removed << "container(s)";
	return ret;
}

static QString exclude(const QString& source, const QString& begin, const QString& end)
{
	int beginIdx = source.indexOf(begin, 0, Qt::CaseInsensitive);
	if (-1 == beginIdx) {
		return source;
	}

	int endIdx = source.indexOf(end, beginIdx + begin.length(), Qt::CaseInsensitive);
	if (-1 == endIdx) {
		return source;
	}

	return source.left(beginIdx) + source.right(source.length() - endIdx - end.length());
}

static QString excludeXmlTag(const QString& source, const QString& tag)
{
	auto match = xmlTagRegex.match(tag);
	if (!match.hasMatch()) {
		return source;
	}

	return exclude(source, tag, "</" + match.captured(1) + ">");
}

static void applyExtractRule(const UltimateLyricsProvider::Rule& rule, QString& content, const Song& song)
{
	for (const UltimateLyricsProvider::RuleItem& item : rule) {
		switch (item.type) {
		case UltimateLyricsProvider::RuleItem::XmlTag:
			content = extractXmlTag(content, doTagReplace(item.begin, song));
			break;
		case UltimateLyricsProvider::RuleItem::Range:
			content = extract(content, doTagReplace(item.begin, song), doTagReplace(item.end, song));
			break;
		case UltimateLyricsProvider::RuleItem::Container:
			content = extractContainers(content, doTagReplace(item.begin, song));
			break;
		case UltimateLyricsProvider::RuleItem::Unescape:
			if (QLatin1String("json") == item.begin) {
				content = jsonUnescape(content);
			}
			break;
		}
	}
}

static void applyExcludeRule(const UltimateLyricsProvider::Rule& rule, QString& content, const Song& song)
{
	for (const UltimateLyricsProvider::RuleItem& item : rule) {
		switch (item.type) {
		case UltimateLyricsProvider::RuleItem::XmlTag:
			content = excludeXmlTag(content, doTagReplace(item.begin, song));
			break;
		case UltimateLyricsProvider::RuleItem::Range:
			content = exclude(content, doTagReplace(item.begin, song), doTagReplace(item.end, song));
			break;
		case UltimateLyricsProvider::RuleItem::Container:
			content = excludeContainers(content, doTagReplace(item.begin, song));
			break;
		case UltimateLyricsProvider::RuleItem::Unescape:
			// Not meaningful as an exclusion - ignored.
			break;
		}
	}
}

static QString urlEncode(QString str)
{
	str.replace(QLatin1Char('&'), QLatin1String("%26"));
	str.replace(QLatin1Char('?'), QLatin1String("%3f"));
	str.replace(QLatin1Char('+'), QLatin1String("%2b"));
	return str;
}

static bool tryWithoutThe(const Song& s)
{
	return 0 == s.priority && s.basicArtist().startsWith(constThe);
}

UltimateLyricsProvider::UltimateLyricsProvider()
	: enabled(true), relevance(0), markup(false)
{
}

UltimateLyricsProvider::~UltimateLyricsProvider()
{
	abort();
}

QString UltimateLyricsProvider::displayName() const
{
	QString n(name);
	n.replace("(POLISH)", tr("(Polish Translations)"));
	n.replace("(PORTUGUESE)", tr("(Portuguese Translations)"));
	return n;
}

QUrl UltimateLyricsProvider::buildUrl(const QString& templateUrl, const QString& artist, const QString& title, const Song& metadata) const
{
	QString urlText(templateUrl);

	// Fill in fields in the URL
	bool urlContainsDetails = urlText.contains(QLatin1Char('{'));
	if (urlContainsDetails) {
		doUrlReplace(constArtistArg, artist, urlText);
		doUrlReplace(constArtistLowerArg, artist.toLower(), urlText);
		doUrlReplace(constArtistLowerNoSpaceArg, noSpace(artist.toLower()), urlText);
		doUrlReplace(constArtistFirstCharArg, firstChar(artist), urlText);
		doUrlReplace(constAlbumArg, metadata.album, urlText);
		doUrlReplace(constAlbumLowerArg, metadata.album.toLower(), urlText);
		doUrlReplace(constAlbumLowerNoSpaceArg, noSpace(metadata.album.toLower()), urlText);
		doUrlReplace(constTitleArg, title, urlText);
		doUrlReplace(constTitleLowerArg, title.toLower(), urlText);
		doUrlReplace(constTitleCaseArg, titleCase(title), urlText);
		doUrlReplace(constYearArg, QString::number(metadata.year), urlText);
		doUrlReplace(constTrackNoArg, QString::number(metadata.track), urlText);
	}

	// For some reason Qt messes up the ? -> %3F and & -> %26 conversions - by placing 25 after the %
	// So, try and revert this...
	QUrl built(urlText);

	if (urlContainsDetails) {
		QByteArray data = built.toEncoded();
		data.replace("%253F", "%3F");
		data.replace("%253f", "%3f");
		data.replace("%2526", "%26");
		built = QUrl::fromEncoded(data, QUrl::StrictMode);
	}

	return built;
}

void UltimateLyricsProvider::fetchInfo(int id, Song metadata, bool removeThe)
{
	auto converter = QStringDecoder(charset.toLatin1().constData(), QStringConverter::Flag::Default);

	if (!converter.isValid()) {
		emit lyricsReady(id, QString(), QUrl());
		return;
	}

	QString artistFixed = metadata.basicArtist();
	QString titleFixed = metadata.basicTitle();

	if (removeThe && artistFixed.startsWith(constThe)) {
		artistFixed = artistFixed.mid(constThe.length());
	}

	metadata.priority = removeThe ? 1 : 0;// HACK Use this to indicate if searching without 'The '
	songs.insert(id, metadata);

	QUrl fetchUrl = buildUrl(url, artistFixed, titleFixed, metadata);

	// Remember where these lyrics came from, so that we can offer a link to the source. For
	// providers queried through an API the fetched url is of no use to the user, so those supply a
	// separate, human readable, page url instead.
	sourceUrls.insert(id, pageUrl.isEmpty() ? fetchUrl : buildUrl(pageUrl, artistFixed, titleFixed, metadata));

	QNetworkRequest req(fetchUrl);
	req.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux i686; rv:6.0) Gecko/20100101 Firefox/6.0");
	NetworkJob* reply = NetworkAccessManager::self()->get(req);
	requests[reply] = id;
	connect(reply, SIGNAL(finished()), this, SLOT(lyricsFetched()));
}

void UltimateLyricsProvider::abort()
{
	QHash<NetworkJob*, int>::ConstIterator it(requests.constBegin());
	QHash<NetworkJob*, int>::ConstIterator end(requests.constEnd());

	for (; it != end; ++it) {
		it.key()->cancelAndDelete();
	}
	requests.clear();
	songs.clear();
	sourceUrls.clear();
}

void UltimateLyricsProvider::wikiMediaSearchResponse()
{
	NetworkJob* reply = qobject_cast<NetworkJob*>(sender());
	if (!reply) {
		return;
	}

	int id = requests.take(reply);
	reply->deleteLater();

	if (!reply->ok()) {
		Song song = songs.take(id);
		sourceUrls.remove(id);
		if (tryWithoutThe(song)) {
			fetchInfo(id, song, true);
		}
		else {
			emit lyricsReady(id, QString(), QUrl());
		}
		return;
	}

	QUrl url;
	QXmlStreamReader doc(reply->actualJob());
	while (!doc.atEnd()) {
		doc.readNext();
		if (doc.isStartElement() && QLatin1String("url") == doc.name()) {
			QString lyricsUrl = doc.readElementText();
			if (!lyricsUrl.contains(QLatin1String("action=edit"))) {
				url = QUrl::fromEncoded(lyricsUrl.toUtf8()).toString();
			}
			break;
		}
	}

	if (url.isValid()) {
		QString path = url.path();
		QByteArray u = url.scheme().toLatin1() + "://" + url.host().toLatin1() + "/api.php?action=query&prop=revisions&rvprop=content&format=xml&titles=";
		QByteArray titles = QUrl::toPercentEncoding(path.startsWith(QLatin1Char('/')) ? path.mid(1) : path).replace('+', "%2b");
		NetworkJob* reply = NetworkAccessManager::self()->get(QUrl::fromEncoded(u + titles));
		requests[reply] = id;
		connect(reply, SIGNAL(finished()), this, SLOT(wikiMediaLyricsFetched()));
	}
	else {
		sourceUrls.remove(id);
		emit lyricsReady(id, QString(), QUrl());
	}
}

void UltimateLyricsProvider::wikiMediaLyricsFetched()
{
	NetworkJob* reply = qobject_cast<NetworkJob*>(sender());
	if (!reply) {
		return;
	}

	int id = requests.take(reply);
	reply->deleteLater();

	if (!reply->ok()) {
		Song song = songs.take(id);
		sourceUrls.remove(id);
		if (tryWithoutThe(song)) {
			fetchInfo(id, song, true);
		}
		else {
			emit lyricsReady(id, QString(), QUrl());
		}
		return;
	}

	auto fromCharset = QStringDecoder(charset.toLatin1().constData(), QStringConverter::Flag::Default);
	QString contents = fromCharset(reply->readAll());
	contents = contents.replace("<br />", "<br/>");
	DBUG << name << "response" << contents;
	emit lyricsReady(id, extract(contents, QLatin1String("&lt;lyrics&gt;"), QLatin1String("&lt;/lyrics&gt;")), sourceUrls.take(id));
}

void UltimateLyricsProvider::lyricsFetched()
{
	NetworkJob* reply = qobject_cast<NetworkJob*>(sender());
	if (!reply) {
		return;
	}

	int id = requests.take(reply);
	reply->deleteLater();
	Song song = songs.take(id);
	QUrl source = sourceUrls.take(id);

	if (!reply->ok()) {
		//emit Finished(id);
		if (tryWithoutThe(song)) {
			fetchInfo(id, song, true);
		}
		else {
			emit lyricsReady(id, QString(), QUrl());
		}
		return;
	}

	auto decode = QStringDecoder(charset.toLatin1().constData());
	QString originalContent = decode(reply->readAll());
	originalContent = originalContent.replace("<br />", "<br/>");

	DBUG << name << "response" << originalContent;
	// Check for invalid indicators
	for (const QString& indicator : invalidIndicators) {
		if (originalContent.contains(indicator)) {
			//emit Finished(id);
			DBUG << name << "invalid";
			if (tryWithoutThe(song)) {
				fetchInfo(id, song, true);
			}
			else {
				emit lyricsReady(id, QString(), QUrl());
			}
			return;
		}
	}

	QString lyrics;

	// Apply extract rules
	for (const Rule& rule : extractRules) {
		QString content = originalContent;
		applyExtractRule(rule, content, song);
#ifndef Q_OS_WIN
		content.replace(QLatin1String("\r"), QLatin1String(""));
#endif
		content = content.trimmed();

		if (!content.isEmpty()) {
			lyrics = content;
			break;
		}
	}

	// Apply exclude rules
	for (const Rule& rule : excludeRules) {
		applyExcludeRule(rule, lyrics, song);
	}

	lyrics = lyrics.trimmed();
	lyrics.replace("<br/>\n", "<br/>");
	lyrics.replace("<br>\n", "<br/>");
	DBUG << name << (lyrics.isEmpty() ? "empty" : "succeeded");
	if (lyrics.isEmpty() && tryWithoutThe(song)) {
		fetchInfo(id, song, true);
	}
	else {
		emit lyricsReady(id, lyrics, lyrics.isEmpty() ? QUrl() : source);
	}
}

void UltimateLyricsProvider::doUrlReplace(const QString& tag, const QString& value, QString& u) const
{
	if (!u.contains(tag)) {
		return;
	}

	// Apply URL character replacement
	QString valueCopy(value);
	for (const UltimateLyricsProvider::UrlFormat& format : urlFormats) {
		QRegularExpression re("[" + QRegularExpression::escape(format.first) + "]");
		valueCopy.replace(re, format.second);
	}
	u.replace(tag, urlEncode(valueCopy), Qt::CaseInsensitive);
}

#include "moc_ultimatelyricsprovider.cpp"
