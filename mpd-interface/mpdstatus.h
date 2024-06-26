/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
 *
 */
/*
 * Copyright (c) 2008 Sander Knopper (sander AT knopper DOT tk) and
 *                    Roeland Douma (roeland AT rullzer DOT com)
 *
 * This file is part of QtMPC.
 *
 * QtMPC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * QtMPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QtMPC.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MPD_STATUS_H
#define MPD_STATUS_H

#include <QObject>

enum MPDState {
	MPDState_Inactive,
	MPDState_Playing,
	MPDState_Stopped,
	MPDState_Paused
};

struct MPDStatusValues {
	MPDStatusValues()
		: volume(0), consume(false), repeat(false), single(false), random(false), partition(""), playlist(0), playlistLength(0), crossFade(0), state(MPDState_Inactive), song(-1), songId(-1), nextSong(-1), nextSongId(-1), timeElapsed(0), timeTotal(0), bitrate(0), samplerate(0), bits(0), channels(0), updatingDb(-1)
	{
	}
	qint8 volume;
	bool consume;
	bool repeat;
	bool single;
	bool random;
	QString partition;
	quint32 playlist;
	quint32 playlistLength;
	qint32 crossFade;
	MPDState state;
	qint32 song;
	qint32 songId;
	qint32 nextSong;
	qint32 nextSongId;
	quint16 timeElapsed;
	quint16 timeTotal;
	quint32 bitrate;
	quint32 samplerate;
	quint8 bits;
	quint8 channels;
	qint32 updatingDb;
	QString error;
};

class MPDStatus : public QObject {
	Q_OBJECT

public:
	MPDStatus();
	~MPDStatus() override {}

	static MPDStatus* self();

	// NOTE: There are no read/write locks aroud these values as they are read/written only fro the GUI thread...
	qint8 volume() const { return values.volume; }
	bool consume() const { return values.consume; }
	bool repeat() const { return values.repeat; }
	bool single() const { return values.single; }
	bool random() const { return values.random; }
	const QString& partition() const { return values.partition; }
	quint32 playlist() const { return values.playlist; }
	quint32 playlistLength() const { return values.playlistLength; }
	qint32 crossFade() const { return values.crossFade; }
	MPDState state() const { return values.state; }
	qint32 song() const { return values.song; }
	qint32 songId() const { return values.songId; }
	qint32 nextSong() const { return values.nextSong; }
	qint32 nextSongId() const { return values.nextSongId; }
	quint16 timeElapsed() const { return values.timeElapsed; }
	quint16 timeTotal() const { return values.timeTotal; }
	quint32 bitrate() const { return values.bitrate; }
	quint32 samplerate() const { return values.samplerate; }
	quint8 bits() const { return values.bits; }
	quint8 channels() const { return values.channels; }
	qint32 updatingDb() const { return values.updatingDb; }
	const QString& error() const { return values.error; }
	MPDStatusValues getValues() const { return values; }

	// Cantata does not poll MPD for current position, but instead used a timer
	// This timer will update its value here - so this can be used elsewhere...
	void setGuessedElapsed(quint16 v) { guessed = v; }
	quint16 guessedElapsed() const { return guessed; }

public Q_SLOTS:
	void update(const MPDStatusValues& v);

Q_SIGNALS:
	void updated();

private:
	MPDStatus(const MPDStatus&);
	MPDStatus& operator=(const MPDStatus& other);

private:
	quint16 guessed;
	MPDStatusValues values;
};

#endif
