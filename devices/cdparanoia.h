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

#ifndef CDPARANOIA_H
#define CDPARANOIA_H

#include "config.h"
#include <QString>

extern "C" {
#ifdef LibCDIOParanoia_FOUND
#include <cdda.h>
#include <paranoia.h>
#else
#include <cdda_interface.h>
#include <cdda_paranoia.h>
#endif
}

class CdParanoia {
public:
	explicit CdParanoia(const QString& device, bool full, bool noSkip, bool playback, int offset);
	~CdParanoia();

	inline operator bool() const { return !dev.isEmpty(); }

	void setParanoiaMode(int mode);
	void setFullParanoiaMode(bool f) { setParanoiaMode(f ? 3 : 0); }
	void setMaxRetries(int m) { maxRetries = m; }

	qint16* read();
	int seek(long sector, int mode);

	int firstSectorOfTrack(int track);
	int lastSectorOfTrack(int track);
	int length();

	int lengthOfTrack(int n);
	int numOfFramesOfTrack(int n);
	double sizeOfTrack(int n);//in MiB
	int frameOffsetOfTrack(int n);
	bool isAudioTrack(int n);
	void reset() { init(); }

private:
	bool init();
	void free();

private:
	QString dev;
#ifdef LibCDIOParanoia_FOUND
	cdrom_drive_t* drive;
	cdrom_paranoia_t* paranoia;
#else
	cdrom_drive* drive;
	cdrom_paranoia* paranoia;
#endif
	int paranoiaMode;
	bool neverSkip;
	int maxRetries;
	int seekOffst;
};

#endif
