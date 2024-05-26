/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2008 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYPOPUP_H
#define NOTIFYBYPOPUP_H

#include "knotificationplugin.h"

#include "knotifyconfig.h"
#include <QStringList>

#include "notifications_interface.h"

class KNotification;
class QDBusPendingCallWatcher;

class NotifyByPopup : public KNotificationPlugin {
	Q_OBJECT
public:
	explicit NotifyByPopup(QObject* parent = nullptr);
	~NotifyByPopup() override;

	QString optionName() override
	{
		return QStringLiteral("Popup");
	}
	void notify(KNotification* notification, const KNotifyConfig& notifyConfig) override;
	void close(KNotification* notification) override;
	void update(KNotification* notification, const KNotifyConfig& notifyConfig) override;

private Q_SLOTS:
	// slot which gets called when DBus signals that some notification action was invoked
	void onNotificationActionInvoked(uint notificationId, const QString& actionKey);
	void onNotificationActionTokenReceived(uint notificationId, const QString& xdgActionToken);
	// slot which gets called when DBus signals that some notification was closed
	void onNotificationClosed(uint, uint);
	void onNotificationReplied(uint notificationId, const QString& text);

private:
	/**
     * Sends notification to DBus "org.freedesktop.notifications" interface.
     * @param id knotify-sid identifier of notification
     * @param config notification data
     * @param update If true, will request the DBus service to update
                     the notification with new data from \c notification
     *               Otherwise will put new notification on screen
     * @return true for success or false if there was an error.
     */
	bool sendNotificationToServer(KNotification* notification, const KNotifyConfig& config, bool update = false);

	/**
     * Find the caption and the icon name of the application
     */
	void getAppCaptionAndIconName(const KNotifyConfig& config, QString* appCaption, QString* iconName);
	/*
     * Query the dbus server for notification capabilities
     */
	void queryPopupServerCapabilities();

	/**
     * DBus notification daemon capabilities cache.
     * Do not use this variable. Use #popupServerCapabilities() instead.
     * @see popupServerCapabilities
     */
	QStringList m_popupServerCapabilities;

	/**
     * In case we still don't know notification server capabilities,
     * we need to query those first. That's done in an async way
     * so we queue all notifications while waiting for the capabilities
     * to return, then process them from this queue
     */
	QList<QPair<KNotification*, KNotifyConfig>> m_notificationQueue;
	/**
     * Whether the DBus notification daemon capability cache is up-to-date.
     */
	bool m_dbusServiceCapCacheDirty;

	/*
     * As we communicate with the notification server over dbus
     * we use only ids, this is for fast KNotifications lookup
     */
	QHash<uint, QPointer<KNotification>> m_notifications;

	org::freedesktop::Notifications m_dbusInterface;

	Q_DISABLE_COPY_MOVE(NotifyByPopup)
};

#endif
