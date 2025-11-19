//
// Created by bartkk on 17.10.2025.
//

#include "lrcliblyricsprovider.h"

#include "network/networkaccessmanager.h"

#include <QJsonDocument>
#include <QUrl>
#include <QUrlQuery>
#include <qjsonvalue.h>

void LRCLibLyricsProvider::fetchInfo(int id, Song metadata, bool removeThe)
{
	(void)removeThe;

	QUrl url("https://lrclib.net/api/get");

	QUrlQuery query;
	query.addQueryItem("track_name", QUrl::toPercentEncoding(metadata.title));
	query.addQueryItem("artist_name", QUrl::toPercentEncoding(metadata.artist));
	query.addQueryItem("track_duration", QString::number(metadata.time));
	url.setQuery(query);

	NetworkJob* reply = NetworkAccessManager::self()->get(url);
	requests[reply] = id;
	connect(reply, &NetworkJob::finished, this, &LRCLibLyricsProvider::lyricsFetched);
}

void LRCLibLyricsProvider::abort()
{
	QHash<NetworkJob*, int>::ConstIterator it(requests.constBegin());
	QHash<NetworkJob*, int>::ConstIterator end(requests.constEnd());

	for (; it != end; ++it) {
		it.key()->cancelAndDelete();
	}
	requests.clear();
}

LRCLibLyricsProvider::~LRCLibLyricsProvider()
{
	LRCLibLyricsProvider::abort();
}

void LRCLibLyricsProvider::lyricsFetched()
{
	NetworkJob* reply = qobject_cast<NetworkJob*>(sender());
	if (!reply) {
		return;
	}

	int id = requests.take(reply);
	reply->deleteLater();

	auto replyData = reply->readAll();
	auto json = QJsonDocument::fromJson(replyData);

	if (reply->error() == QNetworkReply::NoError) {
		auto lyrics = json["plainLyrics"].toString();
		emit lyricsReady(id, lyrics);
	} else {
		emit lyricsReady(id, "");
	}
}