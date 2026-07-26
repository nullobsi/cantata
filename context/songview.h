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

#ifndef SONG_VIEW_H
#define SONG_VIEW_H

#include "config.h"
#include "view.h"
#include <QUrl>
#include <QWidget>

class UltimateLyricsProvider;
class QImage;
class Action;
class NetworkJob;
class QTimer;
class ContextEngine;

class SongView : public View {
	Q_OBJECT

	enum Mode {
		Mode_Blank,
		Mode_Display
	};

	enum Pages {
		Page_Lyrics,
		Page_Information,
		Page_Metadata
	};

public:
	static const QLatin1String constLyricsDir;
	static const QLatin1String constExtension;
	static const QLatin1String constSourceExt;
	static const QLatin1String constCacheDir;
	static const QLatin1String constInfoExt;

	SongView(QWidget* p);
	~SongView() override;

	void update(const Song& s, bool force = false) override;
	void saveConfig();

Q_SIGNALS:
	void providersUpdated();

public Q_SLOTS:
	void downloadFinished();
	void lyricsReady(int, QString lyrics, QUrl source);
	void update();
	void search();
	void edit();
	void del();
	void showContextMenu(const QPoint& pos);
	void showInfoContextMenu(const QPoint& pos);

private Q_SLOTS:
	void toggleScroll();
	void songPosition();
	void scroll();
	void curentViewChanged();
	void refreshInfo();
	void infoSearchResponse(const QString& resp, const QString& lang);
	void abortInfoSearch();
	void showMoreInfo(const QUrl& url);

private:
	void loadLyrics();
	void loadLyricsFromFile();
	void loadInfo();
	void loadMetadata();
	void searchForInfo();
	void hideSpinner();
	void abort() override;
	QString mpdFileName() const;
	QString cacheFileName() const;
	void getLyrics();
	void setMode(Mode m);
	bool saveFile(const QString& fileName);

	/**
     * Builds the lyrics page from lyricsPlain, appending a link to where the lyrics came from
     * if we know it. All lyrics display goes through here, so that whatever we add to the page
     * can never end up in the file we save.
     */
	void renderLyrics();

	/**
     * Records, alongside the cached lyrics, which provider supplied them and the page they can be
     * read on. The lyrics file itself is left alone - other MPD clients read it too.
     */
	void saveSourceInfo();

	/**
     * Restores what saveSourceInfo() recorded, provided it is no older than the lyrics it
     * describes.
     *
     * @param lyricsFilePath The lyrics file the source information has to match.
     */
	void loadSourceInfo(const QString& lyricsFilePath);

	/**
     * Reads the lyrics from the given filePath and updates
     * the UI with those lyrics.
     *
     * @param filePath The path to the lyrics file which will be read.
     *
     * @param useSourceInfo Whether the file is one we wrote ourselves, and may therefore have
     *                      recorded source information for. False for user supplied files.
     *
     * @return Returns true if the file could be read; otherwise false.
     */
	bool setLyricsFromFile(const QString& filePath, bool useSourceInfo = false);

private:
	QTimer* scrollTimer;
	qint32 songPos;
	int currentProvider;
	int currentRequest;
	Action* scrollAction;
	Action* refreshAction;
	Action* editAction;
	Action* delAction;
	Mode mode;
	QString lyricsFile;
	QString lyricsPlain;
	QUrl lyricsSource;
	QString lyricsSourceName;
	bool sourceProvidesMarkup;
	QString preEdit;
	NetworkJob* job;
	UltimateLyricsProvider* currentProv;

	bool lyricsNeedsUpdating;
	bool infoNeedsUpdating;
	bool metadataNeedsUpdating;
	Action* refreshInfoAction;
	Action* cancelInfoJobAction;
	ContextEngine* engine;
};

#endif
