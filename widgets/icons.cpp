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

#include "icons.h"
#include "gui/settings.h"
#include "support/globalstatic.h"
#include "support/utils.h"
#if !defined Q_OS_WIN && !defined Q_OS_MAC
#include "support/gtkstyle.h"
#endif
#ifdef Q_OS_MAC
#include "support/osxstyle.h"
#endif
#include "gui/application.h"
#include "support/QtAwesome.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPalette>

GLOBAL_STATIC(Icons, instance)

#if defined Q_OS_MAC || defined Q_OS_WIN
#define ALWAYS_USE_MONO_ICONS
#endif

using namespace fa;

Icons::Icons()
{
	initIcons();

	albumIconSmall.addFile(":album32.svg");
	albumIconLarge.addFile(":album.svg");
#if defined Q_OS_MAC || defined Q_OS_WIN
	appIcon.addFile(":cantata.svg");
	appIcon.addFile(":cantata16.png");
	appIcon.addFile(":cantata22.png");
	appIcon.addFile(":cantata24.png");
	appIcon.addFile(":cantata32.png");
	appIcon.addFile(":cantata48.png");
	appIcon.addFile(":cantata64.png");
	appIcon.addFile(":cantata128.png");
#else
	appIcon = QIcon::fromTheme(PROJECT_REV_ID);
#endif
}

const QIcon& Icons::albumIcon(int size, bool mono) const
{
	return !mono || albumMonoIcon.isNull()
			? size < 48 ? albumIconSmall : albumIconLarge
			: albumMonoIcon;
}

void Icons::initIcons()
{
	QString iconFile = QString(CANTATA_SYS_ICONS_DIR + "stream.png");
	if (QFile::exists(iconFile)) {
		streamIcon.addFile(iconFile);
	}
	if (streamIcon.isNull()) {
		streamIcon = Icon::fa(fa_solid, fa_globe);
	}

	QVariantMap redOpt;
	redOpt.insert("color", Icon::constRed);
	cutIcon = Icon::fa(fa_solid, fa_remove, redOpt);
	stopDynamicIcon = Icon::fa(fa_solid, fa_stop, redOpt);
	addToFavouritesIcon = Icon::fa(fa_solid, fa_heart, redOpt);
	removeIcon = Icon::fa(fa_solid, fa_minus_square, redOpt);
	minusIcon = Icon::fa(fa_solid, fa_minus, redOpt);
	cancelIcon = Icon::fa(fa_solid, fa_close, redOpt);
	quitIcon = Icon::fa(fa_solid, fa_power_off, redOpt);
	clearIcon = Icon::fa(fa_solid, fa_times);
	trashIcon = Icon::fa(fa_solid, fa_trash);

	albumMonoIcon = Icon::fa(fa_solid, fa_compact_disc);
	artistIcon = Icon::fa(fa_solid, fa_user);
	genreIcon = Icon::fa(fa_solid, fa_guitar);
	podcastIcon = Icon::fa(fa_solid, fa_podcast);
	repeatIcon = Icon::fa(fa_solid, fa_repeat);
	shuffleIcon = Icon::fa(fa_solid, fa_random);
	replacePlayQueueIcon = Icon::fa(fa_solid, fa_play);
	appendToPlayQueueIcon = Icon::fa(fa_solid, fa_plus);
	savePlayQueueIcon = Icon::fa(fa_solid, fa_save);
	addNewItemIcon = Icon::fa(fa_solid, fa_plus_square);
	editIcon = Icon::fa(fa_solid, fa_edit);
	searchIcon = Icon::fa(fa_solid, fa_search);

	reloadIcon = Icon::fa(fa_solid, fa_repeat);
	configureIcon = Icon::fa(fa_solid, fa_cogs);
	connectIcon = Icon::fa(fa_solid, fa_plug);
	disconnectIcon = Icon::fa(fa_solid, fa_eject);
	downloadIcon = Icon::fa(fa_solid, fa_download);

	addIcon = Icon::fa(fa_solid, fa_plus);
	addBookmarkIcon = Icon::fa(fa_solid, fa_bookmark);
	audioListIcon = Icon::fa(fa_solid, fa_music);
	playlistListIcon = Icon::fa(fa_solid, fa_list);
	smartPlaylistIcon = Icon::fa(fa_solid, fa_graduation_cap);
	rssListIcon = Icon::fa(fa_solid, fa_rss);
	savedRssListIcon = Icon::fa(fa_solid, fa_rss_square);
	clockIcon = Icon::fa(fa_regular, fa_clock);
	folderListIcon = Icon::fa(fa_regular, fa_folder);
	refreshIcon = Icon::fa(fa_solid, fa_refresh);

#ifdef ENABLE_HTTP_STREAM_PLAYBACK
	httpStreamIcon = Icon::fa(fa_solid, fa_headphones);
#endif
	leftIcon = Icon::fa(fa_solid, fa_chevron_left);
	rightIcon = Icon::fa(fa_solid, fa_chevron_right);
	upIcon = Icon::fa(fa_solid, fa_chevron_up);
	downIcon = Icon::fa(fa_solid, fa_chevron_down);

	singleIcon = Icon::fa(fa_solid, fa_1);
	consumeIcon = Icon::fa(":consume.svg");
	menuIcon = Icon::fa(fa_solid, fa_bars);

	toolbarPauseIcon = Icon::fa(fa_solid, fa_pause);
	toolbarStopIcon = Icon::fa(fa_solid, fa_stop);
	infoIcon = Icon::fa(fa_solid, fa_circle_info);
	toolbarMenuIcon = Icon::fa(fa_solid, fa_bars);

	playqueueIcon = Icon::fa(fa_solid, fa_play);
	libraryIcon = Icon::fa(fa_solid, fa_music);
	foldersIcon = Icon::fa(fa_solid, fa_folder_open);
	playlistsIcon = Icon::fa(fa_solid, fa_list);
	onlineIcon = Icon::fa(fa_solid, fa_globe);
	infoSidebarIcon = Icon::fa(fa_solid, fa_info_circle);

#ifdef ENABLE_DEVICES_SUPPORT
	devicesIcon = Icon::fa(fa_solid, fa_mobile_screen);
#endif
	searchTabIcon = Icon::fa(fa_solid, fa_search);

	QVariantMap rtlMap;
	rtlMap.insert("rtl", true);
	toolbarPrevIcon = Icon::fa(fa_solid, fa_backward_fast, rtlMap);
	toolbarPlayIcon = Icon::fa(fa_solid, fa_play, rtlMap);
	toolbarNextIcon = Icon::fa(fa_solid, fa_forward_fast, rtlMap);
	centrePlayQueueOnTrackIcon = Icon::fa(fa_solid, fa_chevron_right, rtlMap);

	streamListIcon = audioListIcon;
	streamCategoryIcon = folderListIcon;
}
