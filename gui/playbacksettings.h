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

#ifndef PLAYBACKSETTINGS_H
#define PLAYBACKSETTINGS_H

#include "mpd-interface/output.h"
#include "ui_playbacksettings.h"
#include <QList>
#include <QSet>

class PlaybackSettings : public QWidget, private Ui::PlaybackSettings {
	Q_OBJECT

public:
	PlaybackSettings(QWidget* p);
	~PlaybackSettings() override {}

	void load();
	void save();

Q_SIGNALS:
	// These are for communicating with MPD object (which is in its own thread, so need to talk via signal/slots)
	void getReplayGain();
	void setReplayGain(const QString&);
	void setCrossFade(int secs);
	void outputs();
	void enableOutput(quint32 id, bool);

private Q_SLOTS:
	void replayGainSetting(const QString& rg);
	void updateOutputs(const QList<Output>& outputs);
	void mpdConnectionStateChanged(bool c);
	void showAboutReplayGain();

private:
	QSet<int> enabledOutputs;
	QString rgSetting;
};

#endif
