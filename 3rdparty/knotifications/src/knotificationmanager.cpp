/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2013-2015 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2017 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "knotification.h"
#include "knotification_p.h"
#include "knotificationmanager_p.h"

#include <config-knotifications.h>

#include <QFileInfo>
#include <QHash>

#ifdef QT_DBUS_LIB
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

#include "knotificationplugin.h"
#include "knotificationreplyaction.h"
#include "knotifyconfig.h"

#if defined(Q_OS_ANDROID)
#include "notifybyandroid.h"
#elif defined(Q_OS_MACOS)
#include "notifybymacosnotificationcenter.h"
#elif defined(WITH_SNORETOAST)
#include "notifybysnore.h"
#else
#include "notifybypopup.h"
#include "notifybyportal.h"
#endif

#if defined(HAVE_CANBERRA)
#include "notifybyaudio.h"
#endif

typedef QHash<QString, QString> Dict;

struct Q_DECL_HIDDEN KNotificationManager::Private {
	QHash<int, KNotification*> notifications;
	QHash<QString, KNotificationPlugin*> notifyPlugins;

	QStringList dirtyConfigCache;
	bool portalDBusServiceExists = false;
};

class KNotificationManagerSingleton {
public:
	KNotificationManager instance;
};

Q_GLOBAL_STATIC(KNotificationManagerSingleton, s_self)

KNotificationManager* KNotificationManager::self()
{
	return &s_self()->instance;
}

KNotificationManager::KNotificationManager()
	: d(new Private)
{
	qDeleteAll(d->notifyPlugins);
	d->notifyPlugins.clear();

#ifdef QT_DBUS_LIB
	if (isInsideSandbox()) {
		QDBusConnectionInterface* interface = QDBusConnection::sessionBus().interface();
		d->portalDBusServiceExists = interface->isServiceRegistered(QStringLiteral("org.freedesktop.portal.Desktop"));
	}

	QDBusConnection::sessionBus().connect(QString(),
	                                      QStringLiteral("/Config"),
	                                      QStringLiteral("org.kde.knotification"),
	                                      QStringLiteral("reparseConfiguration"),
	                                      this,
	                                      SLOT(reparseConfiguration(QString)));
#endif
}

KNotificationManager::~KNotificationManager() = default;

KNotificationPlugin* KNotificationManager::pluginForAction(const QString& action)
{
	KNotificationPlugin* plugin = d->notifyPlugins.value(action);

	// We already loaded a plugin for this action.
	if (plugin) {
		return plugin;
	}

	auto addPlugin = [this](KNotificationPlugin* plugin) {
		d->notifyPlugins[plugin->optionName()] = plugin;
		connect(plugin, &KNotificationPlugin::finished, this, &KNotificationManager::notifyPluginFinished);
		connect(plugin, &KNotificationPlugin::xdgActivationTokenReceived, this, &KNotificationManager::xdgActivationTokenReceived);
		connect(plugin, &KNotificationPlugin::actionInvoked, this, &KNotificationManager::notificationActivated);
		connect(plugin, &KNotificationPlugin::replied, this, &KNotificationManager::notificationReplied);
	};

	// Load plugin.
	// We have a series of built-ins up first, and fall back to trying
	// to instantiate an externally supplied plugin.
	if (action == QLatin1String("Popup")) {
#if defined(Q_OS_ANDROID)
		plugin = new NotifyByAndroid(this);
#elif defined(WITH_SNORETOAST)
		plugin = new NotifyBySnore(this);
#elif defined(Q_OS_MACOS)
		plugin = new NotifyByMacOSNotificationCenter(this);
#else
		if (d->portalDBusServiceExists) {
			plugin = new NotifyByPortal(this);
		}
		else {
			plugin = new NotifyByPopup(this);
		}
#endif
		addPlugin(plugin);
	}
	else if (action == QLatin1String("Sound")) {
#if defined(HAVE_CANBERRA)
		plugin = new NotifyByAudio(this);
		addPlugin(plugin);
#endif
	}

	return plugin;
}

void KNotificationManager::notifyPluginFinished(KNotification* notification)
{
	if (!notification || !d->notifications.contains(notification->id())) {
		return;
	}

	notification->deref();
}

void KNotificationManager::notificationActivated(int id, const QString& actionId)
{
	if (d->notifications.contains(id)) {
		//qCDebug(LOG_KNOTIFICATIONS) << id << " " << actionId;
		KNotification* n = d->notifications[id];
		n->activate(actionId);

		// Resident actions delegate control over notification lifetime to the client
		if (!n->hints().value(QStringLiteral("resident")).toBool()) {
			close(id);
		}
	}
}

void KNotificationManager::xdgActivationTokenReceived(int id, const QString& token)
{
	KNotification* n = d->notifications.value(id);
	if (n) {
		//qCDebug(LOG_KNOTIFICATIONS) << "Token received for" << id << token;
		n->d->xdgActivationToken = token;
		Q_EMIT n->xdgActivationTokenChanged();
	}
}

void KNotificationManager::notificationReplied(int id, const QString& text)
{
	if (KNotification* n = d->notifications.value(id)) {
		if (auto* replyAction = n->replyAction()) {
			// cannot really send out a "activate inline-reply" signal from plugin to manager
			// so we instead assume empty reply is not supported and means normal invocation
			if (text.isEmpty() && replyAction->fallbackBehavior() == KNotificationReplyAction::FallbackBehavior::UseRegularAction) {
				Q_EMIT replyAction->activated();
			}
			else {
				Q_EMIT replyAction->replied(text);
			}
			close(id);
		}
	}
}

void KNotificationManager::notificationClosed()
{
	KNotification* notification = qobject_cast<KNotification*>(sender());
	if (!notification) {
		return;
	}
	// We cannot do d->notifications.find(notification->id()); here because the
	// notification->id() is -1 or -2 at this point, so we need to look for value
	for (auto iter = d->notifications.begin(); iter != d->notifications.end(); ++iter) {
		if (iter.value() == notification) {
			d->notifications.erase(iter);
			break;
		}
	}
}

void KNotificationManager::close(int id)
{
	if (d->notifications.contains(id)) {
		KNotification* n = d->notifications.value(id);
		//qCDebug(LOG_KNOTIFICATIONS) << "Closing notification" << id;

		// Find plugins that are actually acting on this notification
		// call close() only on those, otherwise each KNotificationPlugin::close()
		// will call finish() which may close-and-delete the KNotification object
		// before it finishes calling close on all the other plugins.
		// For example: Action=Popup is a single actions but there is 5 loaded
		// plugins, calling close() on the second would already close-and-delete
		// the notification
		KNotifyConfig notifyConfig(n->eventId());
		// HACK: autocode popup
		QString notifyActions = "Popup";// notifyConfig.readEntry(QStringLiteral("Action"));

		const auto listActions = notifyActions.split(QLatin1Char('|'));
		for (const QString& action : listActions) {
			if (!d->notifyPlugins.contains(action)) {
				//qCDebug(LOG_KNOTIFICATIONS) << "No plugin for action" << action;
				continue;
			}

			d->notifyPlugins[action]->close(n);
		}
	}
}

void KNotificationManager::notify(KNotification* n)
{
	KNotifyConfig notifyConfig(n->eventId());

	if (d->dirtyConfigCache.contains(n->appName())) {
		d->dirtyConfigCache.removeOne(n->appName());
	}

	// HACK: hardcode Popup
	const QString notifyActions = "Popup";

	if (notifyActions.isEmpty() || notifyActions == QLatin1String("None")) {
		// this will cause KNotification closing itself fast
		n->ref();
		n->deref();
		return;
	}

	d->notifications.insert(n->id(), n);

	// TODO KF6 d-pointer KNotifyConfig and add this there
	if (n->urgency() == KNotification::DefaultUrgency) {
		const QString urgency = notifyConfig.readEntry(QStringLiteral("Urgency"));
		if (urgency == QLatin1String("Low")) {
			n->setUrgency(KNotification::LowUrgency);
		}
		else if (urgency == QLatin1String("Normal")) {
			n->setUrgency(KNotification::NormalUrgency);
		}
		else if (urgency == QLatin1String("High")) {
			n->setUrgency(KNotification::HighUrgency);
		}
		else if (urgency == QLatin1String("Critical")) {
			n->setUrgency(KNotification::CriticalUrgency);
		}
	}

	const auto actionsList = notifyActions.split(QLatin1Char('|'));

	// Make sure all plugins can ref the notification
	// otherwise a plugin may finish and deref before everyone got a chance to ref
	for (const QString& action : actionsList) {
		KNotificationPlugin* notifyPlugin = pluginForAction(action);

		if (!notifyPlugin) {
			//qCDebug(LOG_KNOTIFICATIONS) << "No plugin for action" << action;
			continue;
		}

		n->ref();
	}

	for (const QString& action : actionsList) {
		KNotificationPlugin* notifyPlugin = pluginForAction(action);

		if (!notifyPlugin) {
			//qCDebug(LOG_KNOTIFICATIONS) << "No plugin for action" << action;
			continue;
		}

		//qCDebug(LOG_KNOTIFICATIONS) << "Calling notify on" << notifyPlugin->optionName();
		notifyPlugin->notify(n, notifyConfig);
	}

	connect(n, &KNotification::closed, this, &KNotificationManager::notificationClosed);
}

void KNotificationManager::update(KNotification* n)
{
	KNotifyConfig notifyConfig(n->eventId());

	for (KNotificationPlugin* p : std::as_const(d->notifyPlugins)) {
		p->update(n, notifyConfig);
	}
}

void KNotificationManager::reemit(KNotification* n)
{
	notify(n);
}

void KNotificationManager::reparseConfiguration(const QString& app)
{
	if (!d->dirtyConfigCache.contains(app)) {
		d->dirtyConfigCache << app;
	}
}

bool KNotificationManager::isInsideSandbox()
{
	// logic is taken from KSandbox::isInside()
	static const bool isFlatpak = QFileInfo::exists(QStringLiteral("/.flatpak-info"));
	static const bool isSnap = qEnvironmentVariableIsSet("SNAP");

	return isFlatpak || isSnap;
}

#include "moc_knotificationmanager_p.cpp"
