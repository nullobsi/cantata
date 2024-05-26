/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2008 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybyportal.h"

#include "knotification.h"
#include "knotifyconfig.h"

#include <QBuffer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusServiceWatcher>
#include <QGuiApplication>
#include <QHash>
#include <QIcon>
#include <QMap>
#include <QPointer>

static const char portalDbusServiceName[] = "org.freedesktop.portal.Desktop";
static const char portalDbusInterfaceName[] = "org.freedesktop.portal.Notification";
static const char portalDbusPath[] = "/org/freedesktop/portal/desktop";

class NotifyByPortalPrivate {
public:
	struct PortalIcon {
		QString str;
		QDBusVariant data;
	};

	NotifyByPortalPrivate(NotifyByPortal* parent)
		: dbusServiceExists(false), q(parent)
	{
	}

	/**
     * Sends notification to DBus "org.freedesktop.notifications" interface.
     * @param id knotify-sid identifier of notification
     * @param config notification data
     * @param update If true, will request the DBus service to update
                     the notification with new data from \c notification
     *               Otherwise will put new notification on screen
     * @return true for success or false if there was an error.
     */
	bool sendNotificationToPortal(KNotification* notification, const KNotifyConfig& config);

	/**
     * Sends request to close Notification with id to DBus "org.freedesktop.notifications" interface
     *  @param id knotify-side notification ID to close
     */

	void closePortalNotification(KNotification* notification);
	/**
     * Find the caption and the icon name of the application
     */

	void getAppCaptionAndIconName(const KNotifyConfig& config, QString* appCaption, QString* iconName);

	/**
     * Specifies if DBus Notifications interface exists on session bus
     */
	bool dbusServiceExists;

	/*
     * As we communicate with the notification server over dbus
     * we use only ids, this is for fast KNotifications lookup
     */
	QHash<uint, QPointer<KNotification>> portalNotifications;

	/*
     * Holds the id that will be assigned to the next notification source
     * that will be created
     */
	uint nextId;

	NotifyByPortal* const q;
};

QDBusArgument& operator<<(QDBusArgument& argument, const NotifyByPortalPrivate::PortalIcon& icon)
{
	argument.beginStructure();
	argument << icon.str << icon.data;
	argument.endStructure();
	return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, NotifyByPortalPrivate::PortalIcon& icon)
{
	argument.beginStructure();
	argument >> icon.str >> icon.data;
	argument.endStructure();
	return argument;
}

Q_DECLARE_METATYPE(NotifyByPortalPrivate::PortalIcon)

//---------------------------------------------------------------------------------------

NotifyByPortal::NotifyByPortal(QObject* parent)
	: KNotificationPlugin(parent), d(new NotifyByPortalPrivate(this))
{
	// check if service already exists on plugin instantiation
	QDBusConnectionInterface* interface = QDBusConnection::sessionBus().interface();
	d->dbusServiceExists = interface && interface->isServiceRegistered(QString::fromLatin1(portalDbusServiceName));

	if (d->dbusServiceExists) {
		onServiceOwnerChanged(QString::fromLatin1(portalDbusServiceName), QString(), QStringLiteral("_"));// connect signals
	}

	// to catch register/unregister events from service in runtime
	QDBusServiceWatcher* watcher = new QDBusServiceWatcher(this);
	watcher->setConnection(QDBusConnection::sessionBus());
	watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
	watcher->addWatchedService(QString::fromLatin1(portalDbusServiceName));
	connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &NotifyByPortal::onServiceOwnerChanged);
}

NotifyByPortal::~NotifyByPortal() = default;

void NotifyByPortal::notify(KNotification* notification, const KNotifyConfig& notifyConfig)
{
	if (d->portalNotifications.contains(notification->id())) {
		// notification is already on the screen, do nothing
		finish(notification);
		return;
	}

	// check if Notifications DBus service exists on bus, use it if it does
	if (d->dbusServiceExists) {
		if (!d->sendNotificationToPortal(notification, notifyConfig)) {
			finish(notification);// an error occurred.
		}
	}
}

void NotifyByPortal::close(KNotification* notification)
{
	if (d->dbusServiceExists) {
		d->closePortalNotification(notification);
	}
}

void NotifyByPortal::update(KNotification* notification, const KNotifyConfig& notifyConfig)
{
	// TODO not supported by portals
	Q_UNUSED(notification);
	Q_UNUSED(notifyConfig);
}

void NotifyByPortal::onServiceOwnerChanged(const QString& serviceName, const QString& oldOwner, const QString& newOwner)
{
	Q_UNUSED(serviceName);
	// close all notifications we currently hold reference to
	for (KNotification* n : std::as_const(d->portalNotifications)) {
		if (n) {
			Q_EMIT finished(n);
		}
	}

	d->portalNotifications.clear();

	if (newOwner.isEmpty()) {
		d->dbusServiceExists = false;
	}
	else if (oldOwner.isEmpty()) {
		d->dbusServiceExists = true;
		d->nextId = 1;

		// connect to action invocation signals
		bool connected = QDBusConnection::sessionBus().connect(QString(),// from any service
		                                                       QString::fromLatin1(portalDbusPath),
		                                                       QString::fromLatin1(portalDbusInterfaceName),
		                                                       QStringLiteral("ActionInvoked"),
		                                                       this,
		                                                       SLOT(onPortalNotificationActionInvoked(QString, QString, QVariantList)));
		if (!connected) {
			//qCWarning(LOG_KNOTIFICATIONS) << "warning: failed to connect to ActionInvoked dbus signal";
		}
	}
}

void NotifyByPortal::onPortalNotificationActionInvoked(const QString& id, const QString& action, const QVariantList& parameter)
{
	Q_UNUSED(parameter);

	auto iter = d->portalNotifications.find(id.toUInt());
	if (iter == d->portalNotifications.end()) {
		return;
	}

	KNotification* n = *iter;
	if (n) {
		Q_EMIT actionInvoked(n->id(), action);
	}
	else {
		d->portalNotifications.erase(iter);
	}
}

void NotifyByPortalPrivate::getAppCaptionAndIconName(const KNotifyConfig& notifyConfig, QString* appCaption, QString* iconName)
{
	*appCaption = notifyConfig.readGlobalEntry(QStringLiteral("Name"));
	if (appCaption->isEmpty()) {
		*appCaption = notifyConfig.readGlobalEntry(QStringLiteral("Comment"));
	}
	if (appCaption->isEmpty()) {
		*appCaption = qApp->applicationName();
	}

	*iconName = notifyConfig.readEntry(QStringLiteral("IconName"));
	if (iconName->isEmpty()) {
		*iconName = notifyConfig.readGlobalEntry(QStringLiteral("IconName"));
	}
	if (iconName->isEmpty()) {
		*iconName = qGuiApp->windowIcon().name();
	}
	if (iconName->isEmpty()) {
		*iconName = qApp->applicationName();
	}
}

bool NotifyByPortalPrivate::sendNotificationToPortal(KNotification* notification, const KNotifyConfig& notifyConfig_nocheck)
{
	QDBusMessage dbusNotificationMessage;
	dbusNotificationMessage = QDBusMessage::createMethodCall(QString::fromLatin1(portalDbusServiceName),
	                                                         QString::fromLatin1(portalDbusPath),
	                                                         QString::fromLatin1(portalDbusInterfaceName),
	                                                         QStringLiteral("AddNotification"));

	QVariantList args;
	// Will be used only with xdg-desktop-portal
	QVariantMap portalArgs;

	QString appCaption;
	QString iconName;
	getAppCaptionAndIconName(notifyConfig_nocheck, &appCaption, &iconName);

	// did the user override the icon name?
	if (!notification->iconName().isEmpty()) {
		iconName = notification->iconName();
	}

	QString title = notification->title().isEmpty() ? appCaption : notification->title();
	QString text = notification->text();

	if (notification->defaultAction()) {
		portalArgs.insert(QStringLiteral("default-action"), QStringLiteral("default"));
		portalArgs.insert(QStringLiteral("default-action-target"), QStringLiteral("0"));
	}

	QString priority;
	switch (notification->urgency()) {
	case KNotification::DefaultUrgency:
		break;
	case KNotification::LowUrgency:
		priority = QStringLiteral("low");
		break;
	case KNotification::NormalUrgency:
		priority = QStringLiteral("normal");
		break;
	case KNotification::HighUrgency:
		priority = QStringLiteral("high");
		break;
	case KNotification::CriticalUrgency:
		priority = QStringLiteral("urgent");
		break;
	}

	if (!priority.isEmpty()) {
		portalArgs.insert(QStringLiteral("priority"), priority);
	}

	// freedesktop.org spec defines action list to be list like
	// (act_id1, action1, act_id2, action2, ...)
	//
	// assign id's to actions like it's done in fillPopup() method
	// (i.e. starting from 1)
	QList<QVariantMap> buttons;
	buttons.reserve(notification->actions().count());

	const auto listActions = notification->actions();
	for (KNotificationAction* action : listActions) {
		QVariantMap button = {{QStringLiteral("action"), action->id()},//
		                      {QStringLiteral("label"), action->label()}};
		buttons << button;
	}

	qDBusRegisterMetaType<QList<QVariantMap>>();
	qDBusRegisterMetaType<PortalIcon>();

	if (!notification->pixmap().isNull()) {
		QByteArray pixmapData;
		QBuffer buffer(&pixmapData);
		buffer.open(QIODevice::WriteOnly);
		notification->pixmap().save(&buffer, "PNG");
		buffer.close();

		PortalIcon icon;
		icon.str = QStringLiteral("bytes");
		icon.data.setVariant(pixmapData);
		portalArgs.insert(QStringLiteral("icon"), QVariant::fromValue<PortalIcon>(icon));
	}
	else {
		// Use this for now for backwards compatibility, we can as well set the variant to be (sv) where the
		// string is keyword "themed" and the variant is an array of strings with icon names
		portalArgs.insert(QStringLiteral("icon"), iconName);
	}

	portalArgs.insert(QStringLiteral("title"), title);
	portalArgs.insert(QStringLiteral("body"), text);
	portalArgs.insert(QStringLiteral("buttons"), QVariant::fromValue<QList<QVariantMap>>(buttons));

	args.append(QString::number(nextId));
	args.append(portalArgs);

	dbusNotificationMessage.setArguments(args);

	QDBusPendingCall notificationCall = QDBusConnection::sessionBus().asyncCall(dbusNotificationMessage, -1);

	// If we are in sandbox we don't need to wait for returned notification id
	portalNotifications.insert(nextId++, notification);

	return true;
}

void NotifyByPortalPrivate::closePortalNotification(KNotification* notification)
{
	uint id = portalNotifications.key(notification, 0);

	// qCDebug(LOG_KNOTIFICATIONS) << "ID: " << id;

	if (id == 0) {
		// qCDebug(LOG_KNOTIFICATIONS) << "not found dbus id to close" << notification->id();
		return;
	}

	QDBusMessage m = QDBusMessage::createMethodCall(QString::fromLatin1(portalDbusServiceName),
	                                                QString::fromLatin1(portalDbusPath),
	                                                QString::fromLatin1(portalDbusInterfaceName),
	                                                QStringLiteral("RemoveNotification"));
	m.setArguments({QString::number(id)});

	// send(..) does not block
	bool queued = QDBusConnection::sessionBus().send(m);

	if (!queued) {
		//// qCWarning(LOG_KNOTIFICATIONS) << "Failed to queue dbus message for closing a notification";
	}
}

#include "moc_notifybyportal.cpp"
