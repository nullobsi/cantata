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

#include "powermanagement.h"
#include "inhibitinterface.h"
#include "login1interface.h"
#include "mpd-interface/mpdstatus.h"
#include "policyagentinterface.h"
#include "support/globalstatic.h"
#include "upowerinterface.h"
#include <QGuiApplication>
#include <QString>

GLOBAL_STATIC(PowerManagement, instance)

PowerManagement::PowerManagement()
	: inhibitSuspendWhilstPlaying(false), cookie(-1)
{
	policy = new OrgKdeSolidPowerManagementPolicyAgentInterface(OrgKdeSolidPowerManagementPolicyAgentInterface::staticInterfaceName(),
	                                                            QLatin1String("/org/kde/Solid/PowerManagement/PolicyAgent"),
	                                                            QDBusConnection::sessionBus(), this);
	inhibit = new OrgFreedesktopPowerManagementInhibitInterface("org.freedesktop.PowerManagement",
	                                                            QLatin1String("/org/freedesktop/PowerManagement/Inhibit"),
	                                                            QDBusConnection::sessionBus(), this);
	upower = new OrgFreedesktopUPowerInterface(OrgFreedesktopUPowerInterface::staticInterfaceName(),
	                                           QLatin1String("/org/freedesktop/UPower"), QDBusConnection::systemBus(), this);
	login1 = new OrgFreedesktopLogin1ManagerInterface("org.freedesktop.login1",
	                                                  QLatin1String("/org/freedesktop/login1"), QDBusConnection::systemBus(), this);
	connect(upower, SIGNAL(Resuming()), this, SIGNAL(resuming()));
	connect(login1, SIGNAL(PrepareForSleep(bool)), this, SLOT(prepareForSleep(bool)));
}

void PowerManagement::setInhibitSuspend(bool i)
{
	if (i == inhibitSuspendWhilstPlaying) {
		return;
	}
	inhibitSuspendWhilstPlaying = i;

	if (inhibitSuspendWhilstPlaying) {
		connect(MPDStatus::self(), SIGNAL(updated()), this, SLOT(mpdStatusUpdated()));
		if (MPDState_Playing == MPDStatus::self()->state()) {
			beginSuppressingSleep();
		}
	}
	else {
		disconnect(MPDStatus::self(), SIGNAL(updated()), this, SLOT(mpdStatusUpdated()));
		stopSuppressingSleep();
	}
}

void PowerManagement::beginSuppressingSleep()
{
	if (-1 != cookie || descriptor.isValid()) {
		return;
	}

	QString reason = tr("Cantata is playing a track");
	QDBusReply<uint> reply;
	if (policy->isValid()) {
		reply = policy->AddInhibition((uint)1, QGuiApplication::applicationDisplayName(), reason);
	}
	else {
		// Fallback to the fd.o Inhibit interface
		reply = inhibit->Inhibit(QGuiApplication::applicationDisplayName(), reason);
	}
	cookie = reply.isValid() ? reply : -1;

	QString types = QStringLiteral("sleep");
	QString mode = QStringLiteral("block");
	QDBusPendingReply<QDBusUnixFileDescriptor> futureReply;
	futureReply = login1->Inhibit(types, QGuiApplication::applicationDisplayName(), reason, mode);
	futureReply.waitForFinished();
	if (futureReply.isValid()) {
		descriptor = futureReply.value();
	}
}

void PowerManagement::stopSuppressingSleep()
{
	if (-1 != cookie) {
		if (policy->isValid()) {
			policy->ReleaseInhibition(cookie);
		}
		else {
			// Fallback to the fd.o Inhibit interface
			inhibit->UnInhibit(cookie);
		}
		cookie = -1;
	}

	if (descriptor.isValid()) {
		QDBusUnixFileDescriptor invalidDescriptor;
		descriptor.swap(invalidDescriptor);
	}
}

void PowerManagement::mpdStatusUpdated()
{
	if (inhibitSuspendWhilstPlaying) {
		if (MPDState_Playing == MPDStatus::self()->state()) {
			beginSuppressingSleep();
		}
		else {
			stopSuppressingSleep();
		}
	}
}

void PowerManagement::prepareForSleep(bool s)
{
	if (!s) {
		emit resuming();
	}
}

#include "moc_powermanagement.cpp"
