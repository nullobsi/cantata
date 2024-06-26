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

#include "albumview.h"
#include "artistview.h"
#include "contextengine.h"
#include "gui/covers.h"
#include "models/mpdlibrarymodel.h"
#include "mpd-interface/cuefile.h"
#include "network/networkaccessmanager.h"
#include "qtiocompressor/qtiocompressor.h"
#include "support/action.h"
#include "support/actioncollection.h"
#include "support/configuration.h"
#include "support/utils.h"
#include "widgets/icons.h"
#include "widgets/textbrowser.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QMenu>
#include <QScrollBar>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

const QLatin1String AlbumView::constCacheDir("albums/");
const QLatin1String AlbumView::constInfoExt(".html.gz");

static const QLatin1String constScheme("cantata");

static QString cacheFileName(const QString& artist, const QString& album, const QString& lang, bool createDir)
{
	return Utils::cacheDir(AlbumView::constCacheDir, createDir) + Covers::encodeName(artist) + QLatin1String(" - ") + Covers::encodeName(album) + "." + lang + AlbumView::constInfoExt;
}

enum Parts {
	Cover = 0x01,
	Details = 0x02,
	All = Cover + Details
};

AlbumView::AlbumView(QWidget* p)
	: View(p), detailsReceived(0)
{
	engine = ContextEngine::create(this);
#ifndef Q_OS_WIN
	// Full width covers not working under windows. Issue #1252
	fullWidthCoverAction = new Action(tr("Full Width Cover"), this);
	fullWidthCoverAction->setCheckable(true);
	connect(fullWidthCoverAction, SIGNAL(toggled(bool)), this, SLOT(setScaleImage(bool)));
	fullWidthCoverAction->setChecked(Configuration(metaObject()->className()).get("fullWidthCover", false));
#endif
	refreshAction = ActionCollection::get()->createAction("refreshalbum", tr("Refresh Album Information"), Icons::self()->refreshIcon);
	connect(refreshAction, SIGNAL(triggered()), this, SLOT(refresh()));
	connect(engine, SIGNAL(searchResult(QString, QString)), this, SLOT(searchResponse(QString, QString)));
	connect(Covers::self(), SIGNAL(cover(Song, QImage, QString)), SLOT(coverRetrieved(Song, QImage, QString)));
	connect(Covers::self(), SIGNAL(coverUpdated(Song, QImage, QString)), SLOT(coverUpdated(Song, QImage, QString)));
	connect(text, SIGNAL(anchorClicked(QUrl)), SLOT(playSong(QUrl)));
	text->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(text, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
	setStandardHeader(tr("Album"));
	int imageSize = fontMetrics().height() * 18;
	setPicSize(QSize(imageSize, imageSize));
	clear();
	if (ArtistView::constCacheAge > 0) {
		clearCache();
		QTimer* timer = new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(clearCache()));
		timer->start((int)((ArtistView::constCacheAge / 2.0) * 1000 * 24 * 60 * 60));
	}
}

AlbumView::~AlbumView()
{
#ifndef Q_OS_WIN
	Configuration(metaObject()->className()).set("fullWidthCover", fullWidthCoverAction->isChecked());
#endif
}

void AlbumView::showContextMenu(const QPoint& pos)
{
	QMenu* menu = text->createStandardContextMenu();
	menu->addSeparator();
	if (cancelJobAction->isEnabled()) {
		menu->addAction(cancelJobAction);
	}
	else {
		menu->addAction(refreshAction);
	}
#ifndef Q_OS_WIN
	menu->addAction(fullWidthCoverAction);
#endif
	menu->exec(text->mapToGlobal(pos));
	delete menu;
}

void AlbumView::refresh()
{
	if (currentSong.isEmpty()) {
		return;
	}
	for (const QString& lang : engine->getLangs()) {
		QFile::remove(cacheFileName(Covers::fixArtist(currentSong.albumArtistOrComposer()), currentSong.album, engine->getPrefix(lang), false));
	}
	update(currentSong, true);
}

void AlbumView::update(const Song& song, bool force)
{
	QString streamName = song.isStandardStream() && song.album.isEmpty() ? song.name() : QString();
	if (!streamName.isEmpty() && streamName != currentSong.name()) {
		abort();
		currentSong = song;
		clearDetails();
		setHeader(streamName);
		needToUpdate = false;
		detailsReceived = All;
		pic = createPicTag(QImage(), CANTATA_SYS_ICONS_DIR + QLatin1String("stream.png"));
		updateDetails();
		return;
	}

	if (song.isEmpty() || song.albumArtistOrComposer().isEmpty() || song.album.isEmpty()) {
		currentSong = song;
		clearDetails();
		abort();
		return;
	}

	if (force || song.albumArtistOrComposer() != currentSong.albumArtistOrComposer() || song.album != currentSong.album) {
		currentSong = song;
		currentArtist = currentSong.basicArtist();
		abort();
		if (!isVisible()) {
			needToUpdate = true;
			return;
		}
		clearDetails();
		setHeader(song.album.isEmpty() ? stdHeader : song.album);
		Covers::Image cImg = Covers::self()->requestImage(song, true);
		detailsReceived |= Cover;// Sometimes cover download fails, and no error?
		if (!cImg.img.isNull()) {
			detailsReceived |= Cover;
			pic = createPicTag(cImg.img, cImg.fileName);
		}
		getTrackListing();
		getDetails();

		if (All == detailsReceived) {
			hideSpinner();
		}
		else {
			showSpinner();
		}
	}
	else if (song.title != currentSong.title) {
		currentSong = song;
		getTrackListing();
		updateDetails(true);
	}
}

void AlbumView::playSong(const QUrl& url)
{
	if (url.scheme() == constScheme) {
		emit playSong(url.path().mid(1));// Remove leading /
	}
	else if (CueFile::isCue(url.toString())) {
		emit playSong(url.toString());
	}
	else {
		QDesktopServices::openUrl(url);
	}
}

void AlbumView::getTrackListing()
{
	if (currentSong.isNonMPD()) {
		if (!pic.isEmpty()) {
			updateDetails();
		}
		return;
	}

	if (songs.isEmpty()) {
		songs = MpdLibraryModel::self()->getAlbumTracks(currentSong, 500);
	}

	if (!songs.isEmpty()) {
		trackList = View::subHeader(tr("Tracks")) + QLatin1String("<p><table>");
		for (const Song& s : songs) {
			if (CueFile::isCue(s.file)) {
				QUrl u(s.file);
				QUrlQuery q(u);

				q.addQueryItem("artist", s.artist);
				q.addQueryItem("albumartist", s.albumartist);
				q.addQueryItem("album", s.album);
				q.addQueryItem("title", s.title);
				q.addQueryItem("disc", QString::number(s.disc));
				q.addQueryItem("track", QString::number(s.track));
				q.addQueryItem("time", QString::number(s.time));
				q.addQueryItem("year", QString::number(s.year));
				q.addQueryItem("origYear", QString::number(s.origYear));
				u.setQuery(q);

				trackList += QLatin1String("<tr><td align='right'>") + QString::number(s.track) + QLatin1String("</td><td><a href=\"") + u.toString() + QLatin1String("\">") + ((s.albumartist == currentSong.albumartist && s.album == currentSong.album && s.title == currentSong.title) ? "<b>" + s.displayTitle() + "</b>" : s.displayTitle()) + QLatin1String("</a></td></tr>");
			}
			else {
				trackList += QLatin1String("<tr><td align='right'>") + QString::number(s.track) + QLatin1String("</td><td><a href=\"") + constScheme + QLatin1String(":///") + s.file + QLatin1String("\">") + (s.file == currentSong.file ? "<b>" + s.displayTitle() + "</b>" : s.displayTitle()) + QLatin1String("</a></td></tr>");
			}
		}

		trackList += QLatin1String("</table></p>");
		updateDetails();
	}
}

void AlbumView::getDetails()
{
	engine->cancel();
	for (const QString& lang : engine->getLangs()) {
		QString prefix = engine->getPrefix(lang);
		QString cachedFile = cacheFileName(Covers::fixArtist(currentSong.albumArtistOrComposer()), currentSong.album, prefix, false);
		if (QFile::exists(cachedFile)) {
			QFile f(cachedFile);
			QtIOCompressor compressor(&f);
			compressor.setStreamFormat(QtIOCompressor::GzipFormat);
			if (compressor.open(QIODevice::ReadOnly)) {
				QByteArray data = compressor.readAll();

				if (!data.isEmpty()) {
					searchResponse(QString::fromUtf8(data), QString());
					Utils::touchFile(cachedFile);
					return;
				}
			}
		}
	}
	engine->search(QStringList() << currentSong.albumArtistOrComposer() << currentSong.album, ContextEngine::Album);
}

void AlbumView::coverRetrieved(const Song& s, const QImage& img, const QString& file)
{
	if (!s.isArtistImageRequest() && (s == currentSong && pic.isEmpty())) {
		detailsReceived |= Cover;
		if (All == detailsReceived) {
			hideSpinner();
		}
		pic = createPicTag(img, file);
		if (!pic.isEmpty()) {
			updateDetails();
		}
	}
}

void AlbumView::coverUpdated(const Song& s, const QImage& img, const QString& file)
{
	if (!s.isArtistImageRequest() && s == currentSong) {
		detailsReceived |= Cover;
		if (All == detailsReceived) {
			hideSpinner();
		}
		pic = createPicTag(img, file);
		if (!pic.isEmpty()) {
			updateDetails();
		}
	}
}

void AlbumView::searchResponse(const QString& resp, const QString& lang)
{
	detailsReceived |= Details;
	if (All == detailsReceived) {
		hideSpinner();
	}

	if (!resp.isEmpty()) {
		details = engine->translateLinks(resp);
		if (!lang.isEmpty()) {
			QFile f(cacheFileName(Covers::fixArtist(currentSong.albumArtistOrComposer()), currentSong.album, lang, true));
			QtIOCompressor compressor(&f);
			compressor.setStreamFormat(QtIOCompressor::GzipFormat);
			if (compressor.open(QIODevice::WriteOnly)) {
				compressor.write(resp.toUtf8().constData());
			}
		}
		updateDetails();
	}
}

void AlbumView::updateDetails(bool preservePos)
{
	int pos = preservePos ? text->verticalScrollBar()->value() : 0;
	if (!details.isEmpty()) {
		setHtml(pic + "<br>" + details + "<br>" + trackList);
	}
	else {
		setHtml(pic + trackList);
	}
	if (preservePos) {
		text->verticalScrollBar()->setValue(pos);
	}
}

void AlbumView::abort()
{
	engine->cancel();
	hideSpinner();
}

void AlbumView::clearCache()
{
	Utils::clearOldCache(constCacheDir, ArtistView::constCacheAge);
}

void AlbumView::clearDetails()
{
	details.clear();
	trackList.clear();
	bio.clear();
	pic.clear();
	songs.clear();
	clear();
	engine->cancel();
	detailsReceived = 0;
}

#include "moc_albumview.cpp"
