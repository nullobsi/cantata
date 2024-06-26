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

#include "thumbnailtoolbar.h"
#include "gui/settings.h"
#include "gui/stdactions.h"
#include "support/action.h"
#include "widgets/icons.h"
#include <QWinThumbnailToolButton>

ThumbnailToolBar::ThumbnailToolBar(QWidget* p)
	: QWinThumbnailToolBar(p)
{
	setWindow(p->windowHandle());
	prevButton = createButton(StdActions::self()->prevTrackAction);
	playPauseButton = createButton(StdActions::self()->playPauseTrackAction);
	stopButton = createButton(StdActions::self()->stopPlaybackAction);
	nextButton = createButton(StdActions::self()->nextTrackAction);
	readSettings();
	status = MPDStatus::self();
	update();
}

void ThumbnailToolBar::readSettings()
{
	stopButton->setVisible(Settings::self()->showStopButton());
}

void ThumbnailToolBar::updateCurrentSong(const Song& song)
{
	qint32 lastSongId = currentSong.id;
	currentSong = song;

	if (song.id != lastSongId && !status.isNull() && song.id == status->songId()) {
		update();
	}
}

void ThumbnailToolBar::updateStatus(MPDStatus* const status)
{
	if (status != this->status) {
		this->status = status;
	}
	if (status->songId() == currentSong.id || status->songId() == -1) {
		update();
	}
}

void ThumbnailToolBar::update()
{
	playPauseButton->setEnabled(!status.isNull() && status->playlistLength() > 0);

	MPDState state = status.isNull() ? MPDState_Inactive : status->state();
	switch (state) {
	case MPDState_Playing:
		playPauseButton->setIcon(Icons::self()->toolbarPauseIcon);
		stopButton->setEnabled(true);
		nextButton->setEnabled(status->nextSongId() != -1);
		prevButton->setEnabled(status->playlistLength() > 1 || status->timeTotal() > 5 || currentSong.time > 5);
		break;
	case MPDState_Inactive:
	case MPDState_Stopped:
		playPauseButton->setIcon(Icons::self()->toolbarPlayIcon);
		stopButton->setEnabled(false);
		nextButton->setEnabled(false);
		prevButton->setEnabled(false);
		break;
	case MPDState_Paused:
		playPauseButton->setIcon(Icons::self()->toolbarPlayIcon);
		stopButton->setEnabled(0 != status->playlistLength());
		nextButton->setEnabled(status->nextSongId() != -1);
		prevButton->setEnabled(status->playlistLength() > 1 || status->timeTotal() > 5 || currentSong.time > 5);
	default:
		break;
	}
}

QWinThumbnailToolButton* ThumbnailToolBar::createButton(Action* act)
{
	QWinThumbnailToolButton* btn = new QWinThumbnailToolButton(this);
	btn->setToolTip(act->text());
	btn->setIcon(act->icon());
	btn->setEnabled(false);
	QObject::connect(btn, SIGNAL(clicked()), act, SIGNAL(triggered()));
	addButton(btn);
	return btn;
}
