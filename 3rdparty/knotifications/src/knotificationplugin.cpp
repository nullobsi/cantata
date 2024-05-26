/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "knotificationplugin.h"

class KNotificationPluginPrivate
{
};

KNotificationPlugin::KNotificationPlugin(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(nullptr)
{
    Q_UNUSED(args);
}

KNotificationPlugin::~KNotificationPlugin()
{
}

void KNotificationPlugin::update(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    Q_UNUSED(notification);
    Q_UNUSED(notifyConfig);
}

void KNotificationPlugin::close(KNotification *notification)
{
    Q_EMIT finished(notification);
}

void KNotificationPlugin::finish(KNotification *notification)
{
    Q_EMIT finished(notification);
}

#include "moc_knotificationplugin.cpp"
