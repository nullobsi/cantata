/*
    SPDX-FileCopyrightText: 2019-2020 Weixuan XIAO <veyx.shaw at gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYMACOSNOTIFICATIONCENTER_H
#define NOTIFYBYMACOSNOTIFICATIONCENTER_H

#include "knotificationplugin.h"

class NotifyByMacOSNotificationCenter : public KNotificationPlugin
{
    Q_OBJECT

public:
    NotifyByMacOSNotificationCenter(QObject *parent);
    ~NotifyByMacOSNotificationCenter() override;

    QString optionName() override
    {
        return QStringLiteral("Popup");
    }
    void notify(KNotification *notification, const KNotifyConfig &notifyConfig) override;
    void update(KNotification *notification, const KNotifyConfig &notifyConfig) override;
    void close(KNotification *notification) override;
};

#endif // NOTIFYBYMACOSNOTIFICATIONCENTER_H
