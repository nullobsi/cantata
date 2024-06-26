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

#ifndef ARTIST_VIEW_H
#define ARTIST_VIEW_H

#include "db/librarydb.h"
#include "mpd-interface/song.h"
#include "view.h"
#include <QMap>

class ComboBox;
class QLabel;
class NetworkJob;
class QIODevice;
class QImage;
class QUrl;
class ContextEngine;
class Action;

class ArtistView : public View {
	Q_OBJECT

public:
	static const int constCacheAge;
	static const QLatin1String constCacheDir;
	static const QLatin1String constInfoExt;
	static const QLatin1String constSimilarInfoExt;

	ArtistView(QWidget* parent);
	~ArtistView() override { abort(); }

	void update(const Song& s, bool force = false) override;
	const QList<LibraryDb::Album>& getArtistAlbums();

Q_SIGNALS:
	void findArtist(const QString& artist);
	void findAlbum(const QString& artist, const QString& album);

public Q_SLOTS:
	void artistImage(const Song& song, const QImage& i, const QString& f);
	void artistImageUpdated(const Song& song, const QImage& i, const QString& f);

private Q_SLOTS:
	void showContextMenu(const QPoint& pos);
	void refresh();
	void setBio();
	void handleSimilarReply();
	void show(const QUrl& url);
	void clearCache();
	void searchResponse(const QString& resp, const QString& lang);

private:
	void loadBio();
	void loadSimilar();
	void requestSimilar();
	QStringList parseSimilarResponse(const QByteArray& resp);
	void buildSimilar(const QStringList& artists);
	void abort() override;

private:
	Action* refreshAction;
	ContextEngine* engine;
	QString pic;
	QString biography;
	QString similarArtists;
	NetworkJob* currentSimilarJob;
	QString provider;
	QString webLinks;
	QString albums;
	QList<LibraryDb::Album> artistAlbums;
};

#endif
