/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
 * Copyright (c) 2025 Markus Mittendrein <git@maxmitti.at>
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

#ifndef LYRICSPROVIDER_H
#define LYRICSPROVIDER_H

#include <QHash>
#include <QMap>
#include <QObject>
#include <QString>

#include "mpd-interface/song.h"

class NetworkJob;

class LyricsProvider : public QObject {
	Q_OBJECT

public:
	LyricsProvider();
	virtual ~LyricsProvider() override;

	bool isEnabled() const;
	void setEnabled(bool enable);
	void setRelevance(int newRelevance);
	void setName(const QString& newName);
	QString getName() const;
	int getRelevance() const;

	void fetchInfo(int id, Song metadata, bool removeThe = false);
	void abort();

	virtual QString displayName() const;

Q_SIGNALS:
	void lyricsReady(int id, const QString& data);

protected:
	virtual void fetchInfoImpl(int id, Song metadata) = 0;
	virtual bool processResponseImpl(int id, Song metadata, const QByteArray& response) = 0;

	void performRequest(int id, const QUrl& url);

private Q_SLOTS:
	void processResponse();

protected:
	QHash<NetworkJob*, int> requests;
	QMap<int, Song> songs;

private:
	bool enabled;
	QString name;
	int relevance;
};

#endif// LYRICSPROVIDER_H
