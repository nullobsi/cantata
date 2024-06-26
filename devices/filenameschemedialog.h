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

#ifndef FILENAMESCHEMEDIALOG_H
#define FILENAMESCHEMEDIALOG_H

#include "deviceoptions.h"
#include "mpd-interface/song.h"
#include "support/dialog.h"
#include "ui_filenameschemedialog.h"

class FilenameSchemeDialog : public Dialog, Ui::FilenameSchemeDialog {
	Q_OBJECT

public:
	FilenameSchemeDialog(QWidget* parent);
	void show(const DeviceOptions& opts);

Q_SIGNALS:
	void scheme(const QString& text);

private Q_SLOTS:
	void showHelp();
	void enableOkButton();
	void insertVariable();
	void updateExample();

private:
	void slotButtonClicked(int button) override;

private:
	DeviceOptions origOpts;
	Song exampleSong;
};

#endif
