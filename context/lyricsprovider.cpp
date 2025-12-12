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

#include "lyricsprovider.h"

#include "network/networkaccessmanager.h"

static constexpr QLatin1String constThe("The ");

static bool tryWithoutThe(const Song& s)
{
	return 0 == s.priority && s.basicArtist().startsWith(constThe);
}

LyricsProvider::LyricsProvider()
	: enabled(true), relevance(0)
{
}

LyricsProvider::~LyricsProvider()
{
	abort();
}

bool LyricsProvider::isEnabled() const
{
	return enabled;
}

void LyricsProvider::setEnabled(bool enable)
{
	enabled = enable;
}

int LyricsProvider::getRelevance() const
{
	return relevance;
}

QString LyricsProvider::getName() const
{
	return name;
}

void LyricsProvider::setName(const QString& newName)
{
	name = newName;
}

void LyricsProvider::setRelevance(int newRelevance)
{
	relevance = newRelevance;
}

void LyricsProvider::abort()
{
	for (const auto& request : requests.keys()) {
		request->cancelAndDelete();
	}
	requests.clear();
	songs.clear();
}

QString LyricsProvider::displayName() const
{
	return getName();
}

void LyricsProvider::fetchInfo(int id, Song metadata, bool removeThe)
{
	QString artistFixed = metadata.basicArtist();
	QString titleFixed = metadata.basicTitle();

	if (removeThe && artistFixed.startsWith(constThe)) {
		artistFixed = artistFixed.mid(constThe.length());
	}

	metadata.priority = removeThe ? 1 : 0;// HACK Use this to indicate if searching without 'The '
	songs.insert(id, metadata);

	fetchInfoImpl(id, metadata);
}

void LyricsProvider::performRequest(int id, const QUrl& url)
{
	QNetworkRequest req(url);
	req.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux i686; rv:6.0) Gecko/20100101 Firefox/6.0");
	NetworkJob* reply = NetworkAccessManager::self()->get(req);
	requests[reply] = id;
	connect(reply, &NetworkJob::finished, this, &LyricsProvider::processResponse);
}

void LyricsProvider::processResponse()
{
	NetworkJob* response = qobject_cast<NetworkJob*>(sender());
	if (!response) {
		return;
	}

	int id = requests.take(response);
	response->deleteLater();
	Song song = songs.take(id);

	if (!response->ok() || !processResponseImpl(id, song, response->readAll())) {
		if (tryWithoutThe(song)) {
			fetchInfo(id, song, true);
		}
		else {
			emit lyricsReady(id, QString());
		}
	}
}

#include "moc_lyricsprovider.cpp"
