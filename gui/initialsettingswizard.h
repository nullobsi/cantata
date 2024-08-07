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

#ifndef INITIALSETTINGSWIZARD_H
#define INITIALSETTINGSWIZARD_H

#include "config.h"
#include "mpd-interface/mpdconnection.h"
#include "ui_initialsettingswizard.h"
#include <QWizard>

class InitialSettingsWizard : public QWizard, public Ui::InitialSettingsWizard {
	Q_OBJECT

public:
	InitialSettingsWizard(QWidget* p = nullptr);
	~InitialSettingsWizard() override;
	MPDConnectionDetails getDetails();

Q_SIGNALS:
	// These are for communicating with MPD object (which is in its own thread, so need to talk via signal/slots)
	void setDetails(const MPDConnectionDetails& det);

private Q_SLOTS:
	void connectToMpd();
	void mpdConnectionStateChanged(bool c);
	void showError(const QString& message);
	void dbError(const QString& message);
	void pageChanged(int p);
	void accept() override;
	void reject() override;
	void controlNextButton();
#ifdef Avahi_FOUND
	void adoptServerSettings(QString ip, QString port);
	void detectMPDs();
#endif

private:
	int nextId() const override;
#ifdef Avahi_FOUND
	QPushButton* discoveryButton;
#endif
};

#endif
