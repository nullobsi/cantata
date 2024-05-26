/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2008 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybypopup.h"

#include "imageconverter.h"
#include "knotification.h"
#include "knotificationreplyaction.h"

#include <QBuffer>
#include <QDBusConnection>
#include <QGuiApplication>
#include <QHash>
#include <QIcon>
#include <QMutableListIterator>
#include <QPointer>
#include <QUrl>


NotifyByPopup::NotifyByPopup(QObject *parent)
    : KNotificationPlugin(parent)
    , m_dbusInterface(QStringLiteral("org.freedesktop.Notifications"), QStringLiteral("/org/freedesktop/Notifications"), QDBusConnection::sessionBus())
{
    m_dbusServiceCapCacheDirty = true;

    connect(&m_dbusInterface, &org::freedesktop::Notifications::ActionInvoked, this, &NotifyByPopup::onNotificationActionInvoked);
    connect(&m_dbusInterface, &org::freedesktop::Notifications::ActivationToken, this, &NotifyByPopup::onNotificationActionTokenReceived);

    // TODO can we check if this actually worked?
    // probably not as this just does a DBus filter which will work but the signal might still get caught in apparmor :/
    connect(&m_dbusInterface, &org::freedesktop::Notifications::NotificationReplied, this, &NotifyByPopup::onNotificationReplied);

    connect(&m_dbusInterface, &org::freedesktop::Notifications::NotificationClosed, this, &NotifyByPopup::onNotificationClosed);
}

NotifyByPopup::~NotifyByPopup()
{
    if (!m_notificationQueue.isEmpty()) {
        //qCWarning(LOG_KNOTIFICATIONS) << "Had queued notifications on destruction. Was the eventloop running?";
    }
}

void NotifyByPopup::notify(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    if (m_dbusServiceCapCacheDirty) {
        // if we don't have the server capabilities yet, we need to query for them first;
        // as that is an async dbus operation, we enqueue the notification and process them
        // when we receive dbus reply with the server capabilities
        m_notificationQueue.append(qMakePair(notification, notifyConfig));
        queryPopupServerCapabilities();
    } else {
        if (!sendNotificationToServer(notification, notifyConfig)) {
            finish(notification); // an error occurred.
        }
    }
}

void NotifyByPopup::update(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    sendNotificationToServer(notification, notifyConfig, true);
}

void NotifyByPopup::close(KNotification *notification)
{
    QMutableListIterator<QPair<KNotification *, KNotifyConfig>> iter(m_notificationQueue);
    while (iter.hasNext()) {
        auto &item = iter.next();
        if (item.first == notification) {
            iter.remove();
        }
    }

    uint id = m_notifications.key(notification, 0);

    if (id == 0) {
        //qCDebug(LOG_KNOTIFICATIONS) << "not found dbus id to close" << notification->id();
        return;
    }

    m_dbusInterface.CloseNotification(id);
}

void NotifyByPopup::onNotificationActionTokenReceived(uint notificationId, const QString &xdgActivationToken)
{
    auto iter = m_notifications.find(notificationId);
    if (iter == m_notifications.end()) {
        return;
    }

    KNotification *n = *iter;
    if (n) {
        Q_EMIT xdgActivationTokenReceived(n->id(), xdgActivationToken);
    }
}

void NotifyByPopup::onNotificationActionInvoked(uint notificationId, const QString &actionKey)
{
    auto iter = m_notifications.find(notificationId);
    if (iter == m_notifications.end()) {
        return;
    }

    KNotification *n = *iter;
    if (n) {
        if (actionKey == QLatin1String("inline-reply") && n->replyAction()) {
            Q_EMIT replied(n->id(), QString());
        } else {
            Q_EMIT actionInvoked(n->id(), actionKey);
        }
    } else {
        m_notifications.erase(iter);
    }
}

void NotifyByPopup::onNotificationClosed(uint dbus_id, uint reason)
{
    auto iter = m_notifications.find(dbus_id);
    if (iter == m_notifications.end()) {
        return;
    }
    KNotification *n = *iter;
    m_notifications.remove(dbus_id);

    if (n) {
        Q_EMIT finished(n);
        // The popup bubble is the only user facing part of a notification,
        // if the user closes the popup, it means he wants to get rid
        // of the notification completely, including playing sound etc
        // Therefore we close the KNotification completely after closing
        // the popup, but only if the reason is 2, which means "user closed"
        if (reason == 2) {
            n->close();
        }
    }
}

void NotifyByPopup::onNotificationReplied(uint notificationId, const QString &text)
{
    auto iter = m_notifications.find(notificationId);
    if (iter == m_notifications.end()) {
        return;
    }

    KNotification *n = *iter;
    if (n) {
        if (n->replyAction()) {
            Q_EMIT replied(n->id(), text);
        }
    } else {
        m_notifications.erase(iter);
    }
}

void NotifyByPopup::getAppCaptionAndIconName(const KNotifyConfig &notifyConfig, QString *appCaption, QString *iconName)
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

bool NotifyByPopup::sendNotificationToServer(KNotification *notification, const KNotifyConfig &notifyConfig_nocheck, bool update)
{
    uint updateId = m_notifications.key(notification, 0);

    if (update) {
        if (updateId == 0) {
            // we have nothing to update; the notification we're trying to update
            // has been already closed
            return false;
        }
    }

    QString appCaption;
    QString iconName;
    getAppCaptionAndIconName(notifyConfig_nocheck, &appCaption, &iconName);

    // did the user override the icon name?
    if (!notification->iconName().isEmpty()) {
        iconName = notification->iconName();
    }

    QString title = notification->title().isEmpty() ? appCaption : notification->title();
    QString text = notification->text();

    if (!m_popupServerCapabilities.contains(QLatin1String("body-markup"))) {
        text = stripRichText(text);
    }

    QVariantMap hintsMap;

    // freedesktop.org spec defines action list to be list like
    // (act_id1, action1, act_id2, action2, ...)
    //
    // assign id's to actions like it's done in fillPopup() method
    // (i.e. starting from 1)
    QStringList actionList;
    if (m_popupServerCapabilities.contains(QLatin1String("actions"))) {
        if (notification->defaultAction()) {
            actionList.append(QStringLiteral("default"));
            actionList.append(notification->defaultAction()->label());
        }
        int actId = 0;
        const auto listActions = notification->actions();
        for (const KNotificationAction *action : listActions) {
            actId++;
            actionList.append(action->id());
            actionList.append(action->label());
        }

        if (auto *replyAction = notification->replyAction()) {
            const bool supportsInlineReply = m_popupServerCapabilities.contains(QLatin1String("inline-reply"));

            if (supportsInlineReply || replyAction->fallbackBehavior() == KNotificationReplyAction::FallbackBehavior::UseRegularAction) {
                actionList.append(QStringLiteral("inline-reply"));
                actionList.append(replyAction->label());

                if (supportsInlineReply) {
                    if (!replyAction->placeholderText().isEmpty()) {
                        hintsMap.insert(QStringLiteral("x-kde-reply-placeholder-text"), replyAction->placeholderText());
                    }
                    if (!replyAction->submitButtonText().isEmpty()) {
                        hintsMap.insert(QStringLiteral("x-kde-reply-submit-button-text"), replyAction->submitButtonText());
                    }
                    if (replyAction->submitButtonIconName().isEmpty()) {
                        hintsMap.insert(QStringLiteral("x-kde-reply-submit-button-icon-name"), replyAction->submitButtonIconName());
                    }
                }
            }
        }
    }

    // Add the application name to the hints.
    // According to freedesktop.org spec, the app_name is supposed to be the application's "pretty name"
    // but in some places it's handy to know the application name itself
    if (!notification->appName().isEmpty()) {
        hintsMap[QStringLiteral("x-kde-appname")] = notification->appName();
    }

    if (!notification->eventId().isEmpty()) {
        hintsMap[QStringLiteral("x-kde-eventId")] = notification->eventId();
    }

    if (notification->flags() & KNotification::SkipGrouping) {
        hintsMap[QStringLiteral("x-kde-skipGrouping")] = 1;
    }

    QString desktopFileName = QGuiApplication::desktopFileName();
    if (!desktopFileName.isEmpty()) {
        // handle apps which set the desktopFileName property with filename suffix,
        // due to unclear API dox (https://bugreports.qt.io/browse/QTBUG-75521)
        if (desktopFileName.endsWith(QLatin1String(".desktop"))) {
            desktopFileName.chop(8);
        }
        hintsMap[QStringLiteral("desktop-entry")] = desktopFileName;
    }

    int urgency = -1;
    switch (notification->urgency()) {
    case KNotification::DefaultUrgency:
        break;
    case KNotification::LowUrgency:
        urgency = 0;
        break;
    case KNotification::NormalUrgency:
        Q_FALLTHROUGH();
    // freedesktop.org m_notifications only know low, normal, critical
    case KNotification::HighUrgency:
        urgency = 1;
        break;
    case KNotification::CriticalUrgency:
        urgency = 2;
        break;
    }

    if (urgency > -1) {
        hintsMap[QStringLiteral("urgency")] = urgency;
    }

    const QVariantMap hints = notification->hints();
    for (auto it = hints.constBegin(); it != hints.constEnd(); ++it) {
        hintsMap[it.key()] = it.value();
    }

    // FIXME - re-enable/fix
    // let's see if we've got an image, and store the image in the hints map
    if (!notification->pixmap().isNull()) {
        QByteArray pixmapData;
        QBuffer buffer(&pixmapData);
        buffer.open(QIODevice::WriteOnly);
        notification->pixmap().save(&buffer, "PNG");
        buffer.close();
        hintsMap[QStringLiteral("image_data")] = ImageConverter::variantForImage(QImage::fromData(pixmapData));
    }

    // Persistent     => 0  == infinite timeout
    // CloseOnTimeout => -1 == let the server decide
    int timeout = (notification->flags() & KNotification::Persistent) ? 0 : -1;

    const QDBusPendingReply<uint> reply = m_dbusInterface.Notify(appCaption, updateId, iconName, title, text, actionList, hintsMap, timeout);

    // parent is set to the notification so that no-one ever accesses a dangling pointer on the notificationObject property
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, notification);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, notification](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        QDBusPendingReply<uint> reply = *watcher;
        m_notifications.insert(reply.argumentAt<0>(), notification);
    });

    return true;
}

void NotifyByPopup::queryPopupServerCapabilities()
{
    if (!m_dbusServiceCapCacheDirty) {
        return;
    }

    QDBusPendingReply<QStringList> call = m_dbusInterface.GetCapabilities();

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        const QDBusPendingReply<QStringList> reply = *watcher;
        const QStringList capabilities = reply.argumentAt<0>();
        m_popupServerCapabilities = capabilities;
        m_dbusServiceCapCacheDirty = false;

        // re-run notify() on all enqueued m_notifications
        for (const QPair<KNotification *, KNotifyConfig> &noti : std::as_const(m_notificationQueue)) {
            notify(noti.first, noti.second);
        }

        m_notificationQueue.clear();
    });
}

#include "moc_notifybypopup.cpp"
