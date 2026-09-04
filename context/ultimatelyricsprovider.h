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

#ifndef ULTIMATELYRICSPROVIDER_H
#define ULTIMATELYRICSPROVIDER_H

#include "lyricsprovider.h"
#include "mpd-interface/song.h"
#include <QHash>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QStringList>

class NetworkJob;

class UltimateLyricsProvider : public LyricsProvider {
	Q_OBJECT

public:
	static void enableDebug();

	typedef QPair<QString, QString> RuleItem;
	typedef QList<RuleItem> Rule;
	typedef QPair<QString, QString> UrlFormat;

	void setUrl(const QString& u) { url = u; }
	void setCharset(const QString& c) { charset = c; }
	void addUrlFormat(const QString& replace, const QString& with) { urlFormats << UrlFormat(replace, with); }
	void addExtractRule(const Rule& rule) { extractRules << rule; }
	void addExcludeRule(const Rule& rule) { excludeRules << rule; }
	void addInvalidIndicator(const QString& indicator) { invalidIndicators << indicator; }
	QString displayName() const override;

protected:
	void fetchInfoImpl(int id, Song metadata) override;
	void processResponseImpl(int id, Song metadata, const QByteArray& response) override;

private Q_SLOTS:
	void wikiMediaSearchResponse();
	void wikiMediaLyricsFetched();

private:
	QString doTagReplace(QString str, const Song& song, bool doAll = true);
	void doUrlReplace(const QString& tag, const QString& value, QString& u) const;

private:
	QString url;
	QString charset;
	QList<UrlFormat> urlFormats;
	QList<Rule> extractRules;
	QList<Rule> excludeRules;
	QStringList invalidIndicators;
};

#endif// ULTIMATELYRICSPROVIDER_H
