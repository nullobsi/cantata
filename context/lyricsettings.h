/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
 *
 */
/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LYRICSETTINGS_H
#define LYRICSETTINGS_H

#include "togglelist.h"
#include <QList>
#include <QStringList>
#include <QWidget>

class QListWidgetItem;
class UltimateLyricsProvider;

class LyricSettings : public ToggleList {
	Q_OBJECT
public:
	LyricSettings(QWidget* p);
	~LyricSettings() override {}

	void load();
	void save();

	void showEvent(QShowEvent* e) override;

private:
	bool loadedXml;
};

#endif// LYRICSETTINGS_H
