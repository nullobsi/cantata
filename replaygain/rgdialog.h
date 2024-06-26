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

#ifndef _RGDIALOG_H_
#define _RGDIALOG_H_

#include "albumscanner.h"
#include "config.h"
#include "tags/tags.h"
#include "widgets/songdialog.h"
#include <QFont>

class QComboBox;
class QTreeWidget;
class QLabel;
class QProgressBar;
#ifdef ENABLE_DEVICES_SUPPORT
class Device;
#endif
class TagReader;
class Action;

class RgDialog : public SongDialog {
	Q_OBJECT

public:
	static int instanceCount();

	RgDialog(QWidget* parent);
	virtual ~RgDialog();

	void show(const QList<Song>& songs, const QString& udi, bool autoScan = false);

Q_SIGNALS:
	// These are for communicating with MPD object (which is in its own thread, so need to talk via signal/slots)
	void update();

private:
	void slotButtonClicked(int button);
	void startScanning();
	void stopScanning();
	void createScanner(const QList<int>& indexes);
	void clearScanners();
	void startReadingTags();
	void stopReadingTags();
	bool saveTags();
	void updateView();
#ifdef ENABLE_DEVICES_SUPPORT
	Device* getDevice(const QString& udi, QWidget* p);
#endif
	void closeEvent(QCloseEvent* event);

private Q_SLOTS:
	void scannerProgress(int p);
	void scannerDone();
	void songTags(int index, Tags::ReplayGain tags);
	void tagReaderDone();
	void toggleDisplay();
	void controlRemoveAct();
	void removeItems();

private:
	enum State {
		State_Idle,
		State_ScanningTags,
		State_ScanningFiles,
		State_Saving
	};

	QComboBox* combo;
	QTreeWidget* view;
	QLabel* statusLabel;
	QProgressBar* progress;
	Action* removeAct;
	State state;
	QString base;
	QList<Song> origSongs;

	QMap<AlbumScanner*, int> scanners;
	int totalToScan;

	QMap<int, Tags::ReplayGain> origTags;
	QMap<int, Tags::ReplayGain> tagsToSave;
	TagReader* tagReader;

	bool autoScanTags;

	QSet<int> removedItems;

	QFont italic;
};

#endif
