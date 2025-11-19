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

#include "ultimatelyricshttpprovider.h"
#include "network/networkaccessmanager.h"
#include <QDebug>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
static bool debugEnabled = false;
#define DBUG \
	if (debugEnabled) qWarning() << "Lyrics" << __FUNCTION__
void UltimateLyricsHttpProvider::enableDebug()
{
	debugEnabled = true;
}

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
	return source.mid(beginIdx, endIdx - beginIdx - 1);
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

static void applyExtractRule(const UltimateLyricsHttpProvider::Rule& rule, QString& content, const Song& song)
{
	for (const UltimateLyricsHttpProvider::RuleItem& item : rule) {
		if (item.second.isNull()) {
			content = extractXmlTag(content, doTagReplace(item.first, song));
		}
		else {
			content = extract(content, doTagReplace(item.first, song), doTagReplace(item.second, song));
		}
	}
}

static void applyExcludeRule(const UltimateLyricsHttpProvider::Rule& rule, QString& content, const Song& song)
{
	for (const UltimateLyricsHttpProvider::RuleItem& item : rule) {
		if (item.second.isNull()) {
			content = excludeXmlTag(content, doTagReplace(item.first, song));
		}
		else {
			content = exclude(content, doTagReplace(item.first, song), doTagReplace(item.second, song));
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

UltimateLyricsHttpProvider::UltimateLyricsHttpProvider()
{
	setEnabled(true);
	setRelevance(0);
}

UltimateLyricsHttpProvider::~UltimateLyricsHttpProvider()
{
	abort();
}

QString UltimateLyricsHttpProvider::displayName() const
{
	QString n(name);
	n.replace("(POLISH)", tr("(Polish Translations)"));
	n.replace("(PORTUGUESE)", tr("(Portuguese Translations)"));
	return n;
}

void UltimateLyricsHttpProvider::fetchInfo(int id, Song metadata, bool removeThe)
{
	auto converter = QStringDecoder(charset.toLatin1().constData(), QStringConverter::Flag::Default);

	if (!converter.isValid()) {
		emit lyricsReady(id, QString());
		return;
	}

	QString artistFixed = metadata.basicArtist();
	QString titleFixed = metadata.basicTitle();
	QString urlText(url);

	if (removeThe && artistFixed.startsWith(constThe)) {
		artistFixed = artistFixed.mid(constThe.length());
	}

	if (QLatin1String("lyrics.wikia.com") == name) {
		QUrl url(urlText);
		QUrlQuery query;

		query.addQueryItem(QLatin1String("artist"), artistFixed);
		query.addQueryItem(QLatin1String("song"), titleFixed);
		query.addQueryItem(QLatin1String("func"), QLatin1String("getSong"));
		query.addQueryItem(QLatin1String("fmt"), QLatin1String("xml"));
		url.setQuery(query);

		NetworkJob* reply = NetworkAccessManager::self()->get(url);
		requests[reply] = id;
		connect(reply, SIGNAL(finished()), this, SLOT(wikiMediaSearchResponse()));
		return;
	}

	metadata.priority = removeThe ? 1 : 0;// HACK Use this to indicate if searching without 'The '
	songs.insert(id, metadata);

	// Fill in fields in the URL
	bool urlContainsDetails = urlText.contains(QLatin1Char('{'));
	if (urlContainsDetails) {
		doUrlReplace(constArtistArg, artistFixed, urlText);
		doUrlReplace(constArtistLowerArg, artistFixed.toLower(), urlText);
		doUrlReplace(constArtistLowerNoSpaceArg, noSpace(artistFixed.toLower()), urlText);
		doUrlReplace(constArtistFirstCharArg, firstChar(artistFixed), urlText);
		doUrlReplace(constAlbumArg, metadata.album, urlText);
		doUrlReplace(constAlbumLowerArg, metadata.album.toLower(), urlText);
		doUrlReplace(constAlbumLowerNoSpaceArg, noSpace(metadata.album.toLower()), urlText);
		doUrlReplace(constTitleArg, titleFixed, urlText);
		doUrlReplace(constTitleLowerArg, titleFixed.toLower(), urlText);
		doUrlReplace(constTitleCaseArg, titleCase(titleFixed), urlText);
		doUrlReplace(constYearArg, QString::number(metadata.year), urlText);
		doUrlReplace(constTrackNoArg, QString::number(metadata.track), urlText);
		doUrlReplace(constDuration, QString::number(metadata.time), urlText);
	}

	// For some reason Qt messes up the ? -> %3F and & -> %26 conversions - by placing 25 after the %
	// So, try and revert this...
	QUrl url(urlText);

	if (urlContainsDetails) {
		QByteArray data = url.toEncoded();
		data.replace("%253F", "%3F");
		data.replace("%253f", "%3f");
		data.replace("%2526", "%26");
		url = QUrl::fromEncoded(data, QUrl::StrictMode);
	}

	QNetworkRequest req(url);
	req.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux i686; rv:6.0) Gecko/20100101 Firefox/6.0");
	NetworkJob* reply = NetworkAccessManager::self()->get(req);
	requests[reply] = id;
	connect(reply, SIGNAL(finished()), this, SLOT(lyricsFetched()));
}

void UltimateLyricsHttpProvider::abort()
{
	QHash<NetworkJob*, int>::ConstIterator it(requests.constBegin());
	QHash<NetworkJob*, int>::ConstIterator end(requests.constEnd());

	for (; it != end; ++it) {
		it.key()->cancelAndDelete();
	}
	requests.clear();
	songs.clear();
}

void UltimateLyricsHttpProvider::wikiMediaSearchResponse()
{
	NetworkJob* reply = qobject_cast<NetworkJob*>(sender());
	if (!reply) {
		return;
	}

	int id = requests.take(reply);
	reply->deleteLater();

	if (!reply->ok()) {
		Song song = songs.take(id);
		if (tryWithoutThe(song)) {
			fetchInfo(id, song, true);
		}
		else {
			emit lyricsReady(id, QString());
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
		emit lyricsReady(id, QString());
	}
}

void UltimateLyricsHttpProvider::wikiMediaLyricsFetched()
{
	NetworkJob* reply = qobject_cast<NetworkJob*>(sender());
	if (!reply) {
		return;
	}

	int id = requests.take(reply);
	reply->deleteLater();

	if (!reply->ok()) {
		Song song = songs.take(id);
		if (tryWithoutThe(song)) {
			fetchInfo(id, song, true);
		}
		else {
			emit lyricsReady(id, QString());
		}
		return;
	}

	auto fromCharset = QStringDecoder(charset.toLatin1().constData(), QStringConverter::Flag::Default);
	QString contents = fromCharset(reply->readAll());
	contents = contents.replace("<br />", "<br/>");
	DBUG << name << "response" << contents;
	emit lyricsReady(id, extract(contents, QLatin1String("&lt;lyrics&gt;"), QLatin1String("&lt;/lyrics&gt;")));
}

void UltimateLyricsHttpProvider::lyricsFetched()
{
	NetworkJob* reply = qobject_cast<NetworkJob*>(sender());
	if (!reply) {
		return;
	}

	int id = requests.take(reply);
	reply->deleteLater();
	Song song = songs.take(id);

	if (!reply->ok()) {
		//emit Finished(id);
		if (tryWithoutThe(song)) {
			fetchInfo(id, song, true);
		}
		else {
			emit lyricsReady(id, QString());
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
				emit lyricsReady(id, QString());
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
		emit lyricsReady(id, lyrics);
	}
}

void UltimateLyricsHttpProvider::doUrlReplace(const QString& tag, const QString& value, QString& u) const
{
	if (!u.contains(tag)) {
		return;
	}

	// Apply URL character replacement
	QString valueCopy(value);
	for (const UltimateLyricsHttpProvider::UrlFormat& format : urlFormats) {
		QRegularExpression re("[" + QRegularExpression::escape(format.first) + "]");
		valueCopy.replace(re, format.second);
	}
	u.replace(tag, urlEncode(valueCopy), Qt::CaseInsensitive);
}

static QString parseInvalidIndicator(QXmlStreamReader* reader)
{
	QString ret = reader->attributes().value("value").toString();
	reader->skipCurrentElement();
	return ret;
}

static UltimateLyricsHttpProvider::Rule parseRule(QXmlStreamReader* reader)
{
	UltimateLyricsHttpProvider::Rule ret;

	while (!reader->atEnd()) {
		reader->readNext();

		if (QXmlStreamReader::EndElement == reader->tokenType()) {
			break;
		}

		if (QXmlStreamReader::StartElement == reader->tokenType()) {
			if (QLatin1String("item") == reader->name()) {
				QXmlStreamAttributes attr = reader->attributes();
				if (attr.hasAttribute("tag")) {
					ret << UltimateLyricsHttpProvider::RuleItem(attr.value("tag").toString(), QString());
				}
				else if (attr.hasAttribute("begin")) {
					ret << UltimateLyricsHttpProvider::RuleItem(attr.value("begin").toString(), attr.value("end").toString());
				}
			}
			reader->skipCurrentElement();
		}
	}
	return ret;
}

UltimateLyricsProvider* UltimateLyricsHttpProvider::parseProvider(QXmlStreamReader* reader)
{
	QXmlStreamAttributes attributes = reader->attributes();

	auto* scraper = new UltimateLyricsHttpProvider;
	scraper->setName(attributes.value("name").toString());
	scraper->setCharset(attributes.value("charset").toString());
	scraper->setUrl(attributes.value("url").toString());

	while (!reader->atEnd()) {
		reader->readNext();

		if (QXmlStreamReader::EndElement == reader->tokenType()) {
			break;
		}

		if (QXmlStreamReader::StartElement == reader->tokenType()) {
			if (QLatin1String("extract") == reader->name()) {
				scraper->addExtractRule(parseRule(reader));
			}
			else if (QLatin1String("exclude") == reader->name()) {
				scraper->addExcludeRule(parseRule(reader));
			}
			else if (QLatin1String("invalidIndicator") == reader->name()) {
				scraper->addInvalidIndicator(parseInvalidIndicator(reader));
			}
			else if (QLatin1String("urlFormat") == reader->name()) {
				scraper->addUrlFormat(reader->attributes().value("replace").toString(), reader->attributes().value("with").toString());
				reader->skipCurrentElement();
			}
			else {
				reader->skipCurrentElement();
			}
		}
	}
	return scraper;
}

#include "moc_ultimatelyricshttpprovider.cpp"
