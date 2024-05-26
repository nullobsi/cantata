/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knotificationpermission.h"
#include <qnamespace.h>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>

Qt::PermissionStatus KNotificationPermission::checkPermission()
{
	if (QtAndroidPrivate::androidSdkVersion() < 33) {
		return Qt::PermissionStatus::Granted;
	}

	return QtAndroidPrivate::checkPermission(QStringLiteral("")).result() == QtAndroidPrivate::PermissionResult::Authorized ? Qt::PermissionStatus::Granted
																															: Qt::PermissionStatus::Denied;
}

void KNotificationPermission::requestPermission(QObject* context, const std::function<void(Qt::PermissionStatus)>& callback)
{
	if (QtAndroidPrivate::androidSdkVersion() < 33) {
		callback(Qt::PermissionStatus::Granted);
	}

	QtAndroidPrivate::requestPermission(QStringLiteral("android.permission.POST_NOTIFICATIONS"))
			.then(context, [&callback](QtAndroidPrivate::PermissionResult res) {
				callback(res == QtAndroidPrivate::PermissionResult::Authorized ? Qt::PermissionStatus::Granted : Qt::PermissionStatus::Denied);
			});
}

#else
Qt::PermissionStatus KNotificationPermission::checkPermission()
{
	return Qt::PermissionStatus::Granted;
}

void KNotificationPermission::requestPermission([[maybe_unused]] QObject* context, const std::function<void(Qt::PermissionStatus)>& callback)
{
	callback(Qt::PermissionStatus::Granted);
}
#endif
