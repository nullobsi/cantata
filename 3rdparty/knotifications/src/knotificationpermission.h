/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONPERMISSION_H
#define KNOTIFICATIONPERMISSION_H

#include <qnamespace.h>

#include <functional>

/** Check or request permissions to show notifications on platforms where
 *  that is necessary.
 *
 *  @since 6.0
 */
namespace KNotificationPermission
{

/** Check if the current application has permissions to show notifications. */
Qt::PermissionStatus checkPermission();

/** Request notification permissions. */
void requestPermission(QObject *context, const std::function<void(Qt::PermissionStatus)> &callback);

}

#endif
