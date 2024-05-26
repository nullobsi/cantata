/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2008 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYPORTAL_H
#define NOTIFYBYPORTAL_H

#include "knotificationplugin.h"

#include <QVariantList>

#include <memory>

class KNotification;
class NotifyByPortalPrivate;

class NotifyByPortal : public KNotificationPlugin {
	Q_OBJECT
public:
	explicit NotifyByPortal(QObject* parent = nullptr);
	~NotifyByPortal() override;

	QString optionName() override
	{
		return QStringLiteral("Popup");
	}
	void notify(KNotification* notification, const KNotifyConfig& notifyConfig) override;
	void close(KNotification* notification) override;
	void update(KNotification* notification, const KNotifyConfig& notifyConfig) override;

private Q_SLOTS:

	// slot to catch appearance or disappearance of org.freedesktop.Desktop DBus service
	void onServiceOwnerChanged(const QString&, const QString&, const QString&);

	void onPortalNotificationActionInvoked(const QString&, const QString&, const QVariantList&);

private:
	std::unique_ptr<NotifyByPortalPrivate> const d;
};

#endif
