/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2006 Thiago Macieira <thiago@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONMANAGER_H
#define KNOTIFICATIONMANAGER_H

#include <knotification.h>

#include <memory>

class KNotification;
class KNotificationPlugin;

/**
 * @internal
 * @author Olivier Goffart
 */
class KNotificationManager : public QObject {
	Q_OBJECT
public:
	static KNotificationManager* self();
	~KNotificationManager() override;

	KNotificationPlugin* pluginForAction(const QString& action);

	/**
     * send the dbus call to the knotify server
     */
	void notify(KNotification* n);

	/**
     * send the close dcop call to the knotify server for the notification with the identifier @p id .
     * And remove the notification from the internal map
     * @param id the id of the notification
     * @param force if false, only close registered notification
     */
	void close(int id);

	/**
     * update one notification text and image and actions
     */
	void update(KNotification* n);

	/**
     * re-emit the notification
     */
	void reemit(KNotification* n);

private Q_SLOTS:
	void notificationClosed();
	void xdgActivationTokenReceived(int id, const QString& token);
	void notificationActivated(int id, const QString& action);
	void notificationReplied(int id, const QString& text);
	void notifyPluginFinished(KNotification* notification);
	void reparseConfiguration(const QString& app);

private:
	bool isInsideSandbox();

	struct Private;
	std::unique_ptr<Private> const d;
	KNotificationManager();

	friend class KNotificationManagerSingleton;
};

#endif
