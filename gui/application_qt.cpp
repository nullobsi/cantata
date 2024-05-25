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

#include "application_qt.h"
#include "config.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>

Application::Application(int& argc, char** argv)
	: QApplication(argc, argv)
{
	setApplicationDisplayName(QLatin1String("Cantata"));
#if QT_VERSION >= 0x050700
	setDesktopFileName(PROJECT_REV_ID);
#endif
}

bool Application::start(const QStringList& files)
{
	// TODO: Add an option to try to run anyway when DBUS binding fails.
	if (QDBusConnection::sessionBus().registerService(PROJECT_REV_ID)) {
		if (Utils::KDE != Utils::currentDe()) {
			setAttribute(Qt::AA_DontShowIconsInMenus, true);
		}
		return true;
	}
	loadFiles(files);
	// ...and activate window!
	QDBusConnection::sessionBus().send(QDBusMessage::createMethodCall(PROJECT_REV_ID, "/org/mpris/MediaPlayer2", "", "Raise"));
	return false;
}

void Application::loadFiles(const QStringList& files)
{
	if (!files.isEmpty()) {
		QDBusMessage m = QDBusMessage::createMethodCall(PROJECT_REV_ID, "/cantata", "", "load");
		QList<QVariant> a;
		a.append(files);
		m.setArguments(a);
		QDBusConnection::sessionBus().send(m);
	}
}
