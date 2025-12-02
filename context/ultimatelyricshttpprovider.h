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

#ifndef ULTIMATELYRICSHTTPPROVIDER_H
#define ULTIMATELYRICSHTTPPROVIDER_H

#include "mpd-interface/song.h"
#include "ultimatelyricsprovider.h"
#include <QHash>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QStringList>

class QXmlStreamReader;
class NetworkJob;

class UltimateLyricsHttpProvider : public UltimateLyricsProvider {
	Q_OBJECT

public:
	static void enableDebug();

	UltimateLyricsHttpProvider();
	~UltimateLyricsHttpProvider() override;

	typedef QPair<QString, QString> RuleItem;
	typedef QList<RuleItem> Rule;
	typedef QPair<QString, QString> UrlFormat;

	QString displayName() const override;
	void fetchInfo(int id, Song metadata, bool removeThe = false) override;
	void abort() override;

	void setUrl(const QString& u) { url = u; }
	void setCharset(const QString& c) { charset = c; }
	void addUrlFormat(const QString& replace, const QString& with) { urlFormats << UrlFormat(replace, with); }
	void addExtractRule(const Rule& rule) { extractRules << rule; }
	void addExcludeRule(const Rule& rule) { excludeRules << rule; }
	void addInvalidIndicator(const QString& indicator) { invalidIndicators << indicator; }

	static UltimateLyricsProvider* parseProvider(QXmlStreamReader* reader);

Q_SIGNALS:
	void lyricsReady(int id, const QString& data);

private Q_SLOTS:
	void wikiMediaSearchResponse();
	void wikiMediaLyricsFetched();
	void lyricsFetched();

private:
	QString doTagReplace(QString str, const Song& song, bool doAll = true);
	void doUrlReplace(const QString& tag, const QString& value, QString& u) const;

private:
	QHash<NetworkJob*, int> requests;
	QMap<int, Song> songs;
	QString url;
	QString charset;
	QList<UrlFormat> urlFormats;
	QList<Rule> extractRules;
	QList<Rule> excludeRules;
	QStringList invalidIndicators;
};

#endif// ULTIMATELYRICSHTTPPROVIDER_H
