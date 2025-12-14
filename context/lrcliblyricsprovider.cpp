/*
 * Cantata
 *
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

#include "lrcliblyricsprovider.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>

LrclibLyricsProvider::LrclibLyricsProvider()
{
	setName("lrclib.net");
}

void LrclibLyricsProvider::fetchInfoImpl(int id, Song metadata)
{
	QUrlQuery query;
	query.addQueryItem("artist_name", metadata.basicArtist());
	query.addQueryItem("track_name", metadata.basicTitle());
	if (metadata.time > 0) {
		query.addQueryItem("duration", QString::number(metadata.time));
	}
	if (!metadata.album.isEmpty()) {
		query.addQueryItem("album_name", metadata.album);
	}
	QUrl url("https://lrclib.net/api/get");
	url.setQuery(query);

	performRequest(id, url);
}

void LrclibLyricsProvider::processResponseImpl(int id, Song song, const QByteArray& response)
{
	QJsonDocument result = QJsonDocument::fromJson(response);
	if (result.isNull() || !result["id"].isDouble()) {
		gotNoLyricsRetryWithoutAlbum(id, song);
		return;
	}

	QJsonValue instrumental = result["instrumental"];
	if (instrumental.isBool() && instrumental.toBool()) {
		emit lyricsReady(id, "{Instrumental}");
		return;
	}

	QJsonValue lyrics = result["plainLyrics"];
	if (!lyrics.isString()) {
		gotNoLyricsRetryWithoutAlbum(id, song);
		return;
	}

	emit lyricsReady(id, lyrics.toString());
}

void LrclibLyricsProvider::processRequestFailed(int id, Song song)
{
	gotNoLyricsRetryWithoutAlbum(id, song);
}

void LrclibLyricsProvider::gotNoLyricsRetryWithoutAlbum(int id, Song song)
{
	if (!song.album.isEmpty()) {
		song.album.clear();
		fetchInfo(id, song);
	}
	else {
		gotNoLyrics(id, song);
	}
}
