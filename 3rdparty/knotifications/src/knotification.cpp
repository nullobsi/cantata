/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2013-2014 Martin Klapetek <mklapetek@kde.org>

    code from KNotify/KNotifyClient
    SPDX-FileCopyrightText: 1997 Christian Esken <esken@kde.org>
    SPDX-FileCopyrightText: 2000 Charles Samuels <charles@kde.org>
    SPDX-FileCopyrightText: 2000 Stefan Schimanski <1Stein@gmx.de>
    SPDX-FileCopyrightText: 2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2005 Allan Sandfeld Jensen <kde@carewolf.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "knotification.h"
#include "knotification_p.h"
#include "knotificationmanager_p.h"
#include "knotificationreplyaction.h"

#include <config-knotifications.h>

#include <QGuiApplication>

#include <QStringList>
#include <QUrl>

// incremental notification ID
static int notificationIdCounter = 0;

class KNotificationActionPrivate {
public:
	QString label;
	QString id;
};

KNotificationAction::KNotificationAction(QObject* parent)
	: QObject(parent), d(new KNotificationActionPrivate)
{
}

KNotificationAction::KNotificationAction(const QString& label)
	: QObject(), d(new KNotificationActionPrivate)
{
	d->label = label;
}

KNotificationAction::~KNotificationAction()
{
}

QString KNotificationAction::label() const
{
	return d->label;
}

void KNotificationAction::setLabel(const QString& label)
{
	if (d->label != label) {
		d->label = label;
		Q_EMIT labelChanged(label);
	}
}

QString KNotificationAction::id() const
{
	return d->id;
}

void KNotificationAction::setId(const QString& id)
{
	d->id = id;
}

KNotification::KNotification(const QString& eventId, NotificationFlags flags, QObject* parent)
	: QObject(parent), d(new Private)
{
	d->eventId = eventId;
	d->flags = flags;
	connect(&d->updateTimer, &QTimer::timeout, this, &KNotification::update);
	d->updateTimer.setSingleShot(true);
	d->updateTimer.setInterval(100);
	d->id = ++notificationIdCounter;

	if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"))) {
		setHint(QStringLiteral("x-kde-xdgTokenAppId"), QGuiApplication::desktopFileName());
	}
}

KNotification::~KNotification()
{
	if (d->ownsActions) {
		qDeleteAll(d->actions);
		delete d->defaultAction;
	}

	if (d->id >= 0) {
		KNotificationManager::self()->close(d->id);
	}
}

QString KNotification::eventId() const
{
	return d->eventId;
}

void KNotification::setEventId(const QString& eventId)
{
	if (d->eventId != eventId) {
		d->eventId = eventId;
		Q_EMIT eventIdChanged();
	}
}

QString KNotification::title() const
{
	return d->title;
}

QString KNotification::text() const
{
	return d->text;
}

void KNotification::setTitle(const QString& title)
{
	if (title == d->title) {
		return;
	}

	d->needUpdate = true;
	d->title = title;
	Q_EMIT titleChanged();
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

void KNotification::setText(const QString& text)
{
	if (text == d->text) {
		return;
	}

	d->needUpdate = true;
	d->text = text;
	Q_EMIT textChanged();
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

void KNotification::setIconName(const QString& icon)
{
	if (icon == d->iconName) {
		return;
	}

	d->needUpdate = true;
	d->iconName = icon;
	Q_EMIT iconNameChanged();
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

QString KNotification::iconName() const
{
	return d->iconName;
}

const QImage& KNotification::image() const
{
	return d->image;
}

void KNotification::setImage(const QImage& img)
{
	d->needUpdate = true;
	d->image = img;
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

QList<KNotificationAction*> KNotification::actions() const
{
	return d->actions;
}

void KNotification::clearActions()
{
	if (d->ownsActions) {
		qDeleteAll(d->actions);
	}
	d->actions.clear();
	d->actionIdCounter = 1;

	d->needUpdate = true;
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

KNotificationAction* KNotification::addAction(const QString& label)
{
	d->needUpdate = true;

	KNotificationAction* action = new KNotificationAction(label);
	action->setId(QString::number(d->actionIdCounter));
	d->actionIdCounter++;

	d->actions << action;
	d->ownsActions = true;
	Q_EMIT actionsChanged();

	if (d->id >= 0) {
		d->updateTimer.start();
	}

	return action;
}

void KNotification::setActionsQml(QList<KNotificationAction*> actions)
{
	if (actions == d->actions) {
		return;
	}

	d->actions.clear();

	d->needUpdate = true;
	d->actions = actions;
	d->ownsActions = false;
	Q_EMIT actionsChanged();

	int idCounter = 1;

	for (KNotificationAction* action : d->actions) {
		action->setId(QString::number(idCounter));
		++idCounter;
	}

	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

KNotificationReplyAction* KNotification::replyAction() const
{
	return d->replyAction.get();
}

void KNotification::setReplyAction(std::unique_ptr<KNotificationReplyAction> replyAction)
{
	if (replyAction == d->replyAction) {
		return;
	}

	d->needUpdate = true;
	d->replyAction = std::move(replyAction);
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

KNotificationAction* KNotification::addDefaultAction(const QString& label)
{
	if (d->ownsActions) {
		delete d->defaultAction;
	}

	d->needUpdate = true;
	d->ownsActions = true;
	d->defaultAction = new KNotificationAction(label);

	d->defaultAction->setId(QStringLiteral("default"));

	Q_EMIT defaultActionChanged();
	if (d->id >= 0) {
		d->updateTimer.start();
	}

	return d->defaultAction;
}

void KNotification::setDefaultActionQml(KNotificationAction* defaultAction)
{
	if (defaultAction == d->defaultAction) {
		return;
	}

	d->needUpdate = true;
	d->defaultAction = defaultAction;
	d->ownsActions = false;

	d->defaultAction->setId(QStringLiteral("default"));

	Q_EMIT defaultActionChanged();
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

KNotificationAction* KNotification::defaultAction() const
{
	return d->defaultAction;
}

KNotification::NotificationFlags KNotification::flags() const
{
	return d->flags;
}

void KNotification::setFlags(const NotificationFlags& flags)
{
	if (d->flags == flags) {
		return;
	}

	d->needUpdate = true;
	d->flags = flags;
	Q_EMIT flagsChanged();
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

QString KNotification::componentName() const
{
	return d->componentName;
}

void KNotification::setComponentName(const QString& c)
{
	if (d->componentName != c) {
		d->componentName = c;
		Q_EMIT componentNameChanged();
	}
}

QList<QUrl> KNotification::urls() const
{
	return QUrl::fromStringList(d->hints[QStringLiteral("x-kde-urls")].toStringList());
}

void KNotification::setUrls(const QList<QUrl>& urls)
{
	setHint(QStringLiteral("x-kde-urls"), QUrl::toStringList(urls));
	Q_EMIT urlsChanged();
}

KNotification::Urgency KNotification::urgency() const
{
	return d->urgency;
}

void KNotification::setUrgency(Urgency urgency)
{
	if (d->urgency == urgency) {
		return;
	}

	d->needUpdate = true;
	d->urgency = urgency;
	Q_EMIT urgencyChanged();
	if (d->id >= 0) {
		d->updateTimer.start();
	}
}

void KNotification::activate(const QString& actionId)
{
	if (d->defaultAction && actionId == QLatin1String("default")) {
		Q_EMIT d->defaultAction->activated();
	}

	for (KNotificationAction* action : d->actions) {
		if (action->id() == actionId) {
			Q_EMIT action->activated();
		}
	}
}

void KNotification::close()
{
	if (d->id >= 0) {
		KNotificationManager::self()->close(d->id);
	}

	if (d->id == -1) {
		d->id = -2;
		Q_EMIT closed();
		if (d->autoDelete) {
			deleteLater();
		}
		else {
			// reset for being reused
			d->isNew = true;
			d->id = ++notificationIdCounter;
		}
	}
}

static QString defaultComponentName()
{
#if defined(Q_OS_ANDROID)
	return QStringLiteral("android_defaults");
#else
	return QStringLiteral("plasma_workspace");
#endif
}

KNotification* KNotification::event(const QString& eventid,
                                    const QString& title,
                                    const QString& text,
                                    const QImage& image,
                                    const NotificationFlags& flags,
                                    const QString& componentName)
{
	KNotification* notify = new KNotification(eventid, flags);
	notify->setTitle(title);
	notify->setText(text);
	notify->setImage(image);
	notify->setComponentName((flags & DefaultEvent) ? defaultComponentName() : componentName);

	QTimer::singleShot(0, notify, &KNotification::sendEvent);

	return notify;
}

KNotification*
KNotification::event(const QString& eventid, const QString& text, const QImage& image, const NotificationFlags& flags, const QString& componentName)
{
	return event(eventid, QString(), text, image, flags, componentName);
}

KNotification* KNotification::event(StandardEvent eventid, const QString& title, const QString& text, const QImage& image, const NotificationFlags& flags)
{
	return event(standardEventToEventId(eventid), title, text, image, flags | DefaultEvent);
}

KNotification* KNotification::event(StandardEvent eventid, const QString& text, const QImage& image, const NotificationFlags& flags)
{
	return event(eventid, QString(), text, image, flags);
}

KNotification* KNotification::event(const QString& eventid,
                                    const QString& title,
                                    const QString& text,
                                    const QString& iconName,
                                    const NotificationFlags& flags,
                                    const QString& componentName)
{
	KNotification* notify = new KNotification(eventid, flags);
	notify->setTitle(title);
	notify->setText(text);
	notify->setIconName(iconName);
	notify->setComponentName((flags & DefaultEvent) ? defaultComponentName() : componentName);

	QTimer::singleShot(0, notify, &KNotification::sendEvent);

	return notify;
}

KNotification* KNotification::event(StandardEvent eventid, const QString& title, const QString& text, const QString& iconName, const NotificationFlags& flags)
{
	return event(standardEventToEventId(eventid), title, text, iconName, flags | DefaultEvent);
}

KNotification* KNotification::event(StandardEvent eventid, const QString& title, const QString& text, const NotificationFlags& flags)
{
	return event(standardEventToEventId(eventid), title, text, standardEventToIconName(eventid), flags | DefaultEvent);
}

void KNotification::ref()
{
	d->ref++;
}
void KNotification::deref()
{
	Q_ASSERT(d->ref > 0);
	d->ref--;
	if (d->ref == 0) {
		d->id = -1;
		close();
	}
}

void KNotification::beep(const QString& reason)
{
	event(QStringLiteral("beep"), reason, QImage(), CloseOnTimeout | DefaultEvent);
}

void KNotification::sendEvent()
{
	d->needUpdate = false;
	if (d->isNew) {
		d->isNew = false;
		KNotificationManager::self()->notify(this);
	}
	else {
		KNotificationManager::self()->reemit(this);
	}
}

int KNotification::id()
{
	if (!d) {
		return -1;
	}
	return d->id;
}

QString KNotification::appName() const
{
	QString appname;

	if (d->flags & DefaultEvent) {
		appname = defaultComponentName();
	}
	else if (!d->componentName.isEmpty()) {
		appname = d->componentName;
	}
	else {
		appname = QCoreApplication::applicationName();
	}

	return appname;
}

bool KNotification::isAutoDelete() const
{
	return d->autoDelete;
}

void KNotification::setAutoDelete(bool autoDelete)
{
	if (d->autoDelete != autoDelete) {
		d->autoDelete = autoDelete;
		Q_EMIT autoDeleteChanged();
	}
}

void KNotification::update()
{
	if (d->needUpdate) {
		KNotificationManager::self()->update(this);
	}
}

QString KNotification::standardEventToEventId(KNotification::StandardEvent event)
{
	QString eventId;
	switch (event) {
	case Warning:
		eventId = QStringLiteral("warning");
		break;
	case Error:
		eventId = QStringLiteral("fatalerror");
		break;
	case Catastrophe:
		eventId = QStringLiteral("catastrophe");
		break;
	case Notification:// fall through
	default:
		eventId = QStringLiteral("notification");
		break;
	}
	return eventId;
}

QString KNotification::standardEventToIconName(KNotification::StandardEvent event)
{
	QString iconName;
	switch (event) {
	case Warning:
		iconName = QStringLiteral("dialog-warning");
		break;
	case Error:
		iconName = QStringLiteral("dialog-error");
		break;
	case Catastrophe:
		iconName = QStringLiteral("dialog-error");
		break;
	case Notification:// fall through
	default:
		iconName = QStringLiteral("dialog-information");
		break;
	}
	return iconName;
}

void KNotification::setHint(const QString& hint, const QVariant& value)
{
	if (value == d->hints.value(hint)) {
		return;
	}

	d->needUpdate = true;
	d->hints[hint] = value;
	if (d->id >= 0) {
		d->updateTimer.start();
	}
	Q_EMIT hintsChanged();
}

QVariantMap KNotification::hints() const
{
	return d->hints;
}

void KNotification::setHints(const QVariantMap& hints)
{
	if (hints == d->hints) {
		return;
	}

	d->needUpdate = true;
	d->hints = hints;
	if (d->id >= 0) {
		d->updateTimer.start();
	}
	Q_EMIT hintsChanged();
}

QString KNotification::xdgActivationToken() const
{
	return d->xdgActivationToken;
}

void KNotification::setWindow(QWindow* window)
{
	if (window == d->window) {
		return;
	}

	disconnect(d->window, &QWindow::activeChanged, this, &KNotification::slotWindowActiveChanged);
	d->window = window;
	connect(d->window, &QWindow::activeChanged, this, &KNotification::slotWindowActiveChanged);
}

void KNotification::slotWindowActiveChanged()
{
	if (d->window->isActive() && (d->flags & CloseWhenWindowActivated)) {
		close();
	}
}

QWindow* KNotification::window() const
{
	return d->window;
}

#include "moc_knotification.cpp"
