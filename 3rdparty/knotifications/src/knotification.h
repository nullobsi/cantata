/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2013-2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATION_H
#define KNOTIFICATION_H

#include <QList>
#include <QObject>
#include <QPair>
#include <QPixmap>
#include <QUrl>
#include <QVariant>
#include <QWindow>

#include <memory>

class KNotificationReplyAction;
class KNotificationAction;

class KNotificationActionPrivate;

/**
 * @class KNotificationAction knotification.h KNotificationAction
 *
 * This class represents a notification. This can be a button on the notification
 * popup, or triggered by clicking the notification popup itself.
 *
 * @since 6.0
 */
class KNotificationAction : public QObject
{
    Q_OBJECT
    /**
     * @copydoc label
     */
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)

public:
    explicit KNotificationAction(QObject *parent = nullptr);

    /**
     * Creates an action with given label
     * @param label The label for the action
     */
    explicit KNotificationAction(const QString &label);

    ~KNotificationAction() override;

    /**
     * The user-facing label for the action
     */
    QString label() const;

    /**
     * Set the user-facing label for the action
     */
    void setLabel(const QString &label);

Q_SIGNALS:
    /**
     * Emitted when the user activates the action
     */
    void activated();

    /**
     * Emitted when @p label changed.
     */
    void labelChanged(const QString &label);

private:
    friend class KNotification;
    friend class NotifyByPortalPrivate;
    friend class NotifyByPopup;
    friend class NotifyBySnore;
    friend class NotifyByAndroid;

    void setId(const QString &id);
    QString id() const;

    std::unique_ptr<KNotificationActionPrivate> const d;
};

/**
 * @class KNotification knotification.h KNotification
 *
 * KNotification is the main class for creating notifications.
 */
class KNotification : public QObject
{
    Q_OBJECT
    /**
     * @copydoc setEventId
     * @since 5.88
     */
    Q_PROPERTY(QString eventId READ eventId WRITE setEventId NOTIFY eventIdChanged)
    /**
     * @copydoc setTitle
     * @since 5.88
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    /**
     * @copydoc setText
     * @since 5.88
     */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    /**
     * @copydoc setIconName
     * @since 5.88
     */
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    /**
     * @copydoc setFlags
     * @since 5.88
     */
    Q_PROPERTY(NotificationFlags flags READ flags WRITE setFlags NOTIFY flagsChanged)
    /**
     * @copydoc setComponentName
     * @since 5.88
     */
    Q_PROPERTY(QString componentName READ componentName WRITE setComponentName NOTIFY componentNameChanged)
    /**
     * @copydoc setUrls
     * @since 5.88
     */
    Q_PROPERTY(QList<QUrl> urls READ urls WRITE setUrls NOTIFY urlsChanged)
    /**
     * @copydoc setUrgency
     * @since 5.88
     */
    Q_PROPERTY(Urgency urgency READ urgency WRITE setUrgency NOTIFY urgencyChanged)
    /**
     * @copydoc setAutoDelete
     * @since 5.88
     */
    Q_PROPERTY(bool autoDelete READ isAutoDelete WRITE setAutoDelete NOTIFY autoDeleteChanged)
    /**
     * @since 5.90
     */
    Q_PROPERTY(QString xdgActivationToken READ xdgActivationToken NOTIFY xdgActivationTokenChanged)
    /**
     * @copydoc setHint
     * @since 5.101
     */
    Q_PROPERTY(QVariantMap hints READ hints WRITE setHints NOTIFY hintsChanged)

public:
    /**
     * @see NotificationFlags
     */
    enum NotificationFlag {
        /**
         * The notification will be automatically closed after a timeout. (this is the default)
         */
        CloseOnTimeout = 0x00,

        /**
         * The notification will NOT be automatically closed after a timeout.
         * You will have to track the notification, and close it with the
         * close function manually when the event is done, otherwise there will be a memory leak
         */
        Persistent = 0x02,

        /**
         * The audio plugin will loop the sound until the notification is closed
         */
        LoopSound = 0x08,

        /**
         * Sends a hint to Plasma to skip grouping for this notification
         *
         * @since 5.18
         */
        SkipGrouping = 0x10,

        /**
         * The notification will be automatically closed if the window() becomes
         * activated.
         *
         * You need to set a window using setWindow().
         *
         * @since 6.0
         */
        CloseWhenWindowActivated = 0x20,

        /**
         * @internal
         * The event is a standard kde event, and not an event of the application
         */
        DefaultEvent = 0xF000,

    };

    /**
     * Stores a combination of #NotificationFlag values.
     */
    Q_DECLARE_FLAGS(NotificationFlags, NotificationFlag)
    Q_FLAG(NotificationFlags)

    /**
     * default events you can use in the event function
     */
    enum StandardEvent {
        Notification,
        Warning,
        Error,
        Catastrophe,
    };

    /**
     * The urgency of a notification.
     *
     * @since 5.58
     * @sa setUrgency
     */
    enum Urgency {
        DefaultUrgency = -1,
        LowUrgency = 10,
        NormalUrgency = 50,
        HighUrgency = 70,
        CriticalUrgency = 90,
    };
    Q_ENUM(Urgency)

    /**
     * Create a new notification.
     *
     * You have to use sendEvent to show the notification.
     *
     * The pointer is automatically deleted when the event is closed.
     *
     * @since 4.4
     *
     * @param eventId is the name of the event
     * @param flags is a bitmask of NotificationFlag
     * @param parent parent object
     */
    explicit KNotification(const QString &eventId, NotificationFlags flags = CloseOnTimeout, QObject *parent = nullptr);

    ~KNotification() override;

    /**
     * @return the name of the event
     */
    QString eventId() const;
    /**
     * Set the event id, if not already passed to the constructor.
     * @since 5.88
     */
    void setEventId(const QString &eventId);

    /**
     * @return the notification title
     * @see setTitle
     * @since 4.3
     */
    QString title() const;

    /**
     * Set the title of the notification popup.
     * If no title is set, the application name will be used.
     *
     * @param title The title of the notification
     * @since 4.3
     */
    void setTitle(const QString &title);

    /**
     * @return the notification text
     * @see setText
     */
    QString text() const;

    /**
     * Set the notification text that will appear in the popup.
     *
     * In Plasma workspace, the text is shown in a QML label which uses Text.StyledText,
     * ie. it supports a small subset of HTML entities (mostly just formatting tags)
     *
     * If the notifications server does not advertise "body-markup" capability,
     * all HTML tags are stripped before sending it to the server
     *
     * @param text The text to display in the notification popup
     */
    void setText(const QString &text);

    /**
     * \return the icon shown in the popup
     * \see setIconName
     * \since 5.4
     */
    QString iconName() const;

    /**
     * Set the icon that will be shown in the popup.
     *
     * @param icon the icon
     * @since 5.4
     */
    void setIconName(const QString &icon);

    /**
     * \return the pixmap shown in the popup
     * \see setPixmap
     */
    QPixmap pixmap() const;
    /**
     * Set the pixmap that will be shown in the popup. If you want to use an icon from the icon theme use setIconName instead.
     *
     * @param pix the pixmap
     */
    void setPixmap(const QPixmap &pix);

    /**
     * @return the default action, or nullptr if none is set
     * @since 6.0
     */
    KNotificationAction *defaultAction() const;

    /**
     * Add a default action that will be triggered when the notification is
     * activated (typically, by clicking on the notification popup). The default
     * action typically raises a window belonging to the application that sent it.
     *
     * The string will be used as a label for the action, so ideally it should
     * be wrapped in i18n() or tr() calls.
     *
     * The visual representation of actions depends on the notification server.
     * In Plasma and Gnome desktops, the actions are performed by clicking on
     * the notification popup, and the label is not presented to the user.
     *
     * Calling this overrides the current default action
     *
     * @since 6.0
     */
    [[nodiscard]] KNotificationAction *addDefaultAction(const QString &label);

    /**
     * Add an action to the notification.
     *
     * The visual representation of actions depends
     * on the notification server.
     *
     * @param label the user-visible label of the action
     *
     * @see KNotificationAction
     *
     * @since 6.0
     */
    [[nodiscard]] KNotificationAction *addAction(const QString &label);

    /**
     * Removes all actions previously added by addAction()
     * from the notification.
     *
     * @see addAction
     *
     * @since 6.0
     */
    void clearActions();

    /**
     * @return the inline reply action.
     * @since 5.81
     */
    KNotificationReplyAction *replyAction() const;

    /**
     * @brief Add an inline reply action to the notification.
     *
     * On supported platforms this lets the user type a reply to a notification,
     * such as replying to a chat message, from the notification popup, for example:
     *
     * @code{.cpp}
     * KNotification *notification = new KNotification(QStringLiteral("notification"));
     * ...
     * auto replyAction = std::make_unique<KNotificationReplyAction>(i18nc("@action:button", "Reply"));
     * replyAction->setPlaceholderText(i18nc("@info:placeholder", "Reply to Dave..."));
     * QObject::connect(replyAction.get(), &KNotificationReplyAction::replied, [](const QString &text) {
     *     qDebug() << "you replied with" << text;
     * });
     * notification->setReplyAction(std::move(replyAction));
     * @endcode
     *
     * @param replyAction the reply action to set
     * @since 5.81
     */
    void setReplyAction(std::unique_ptr<KNotificationReplyAction> replyAction);

    /**
     * @return the notification flags.
     */
    NotificationFlags flags() const;

    /**
     * Set the notification flags.
     * These must be set before calling sendEvent()
     */
    void setFlags(const NotificationFlags &flags);

    /**
     * Returns the component name used to determine the location of the configuration file.
     * @since 5.88
     */
    QString componentName() const;
    /**
     * The componentData is used to determine the location of the config file.
     *
     * If no componentName is set, the app name is used by default
     *
     * @param componentName the new component name
     */
    void setComponentName(const QString &componentName);

    /**
     * URLs associated with this notification
     * @since 5.29
     */
    QList<QUrl> urls() const;

    /**
     * Sets URLs associated with this notification
     *
     * For example, a screenshot application might want to provide the
     * URL to the file that was just taken so the notification service
     * can show a preview.
     *
     * @note This feature might not be supported by the user's notification service
     *
     * @param urls A list of URLs
     * @since 5.29
     */
    void setUrls(const QList<QUrl> &urls);

    /**
     * The urgency of the notification.
     * @since 5.58
     */
    Urgency urgency() const;

    /**
     * Sets the urgency of the notification.
     *
     * This defines the importance of the notification. For example,
     * a track change in a media player would be a low urgency.
     * "You have new mail" would be normal urgency. "Your battery level
     * is low" would be a critical urgency.
     *
     * Use critical notifications with care as they might be shown even
     * when giving a presentation or when notifications are turned off.
     *
     * @param urgency The urgency.
     * @since 5.58
     */
    void setUrgency(Urgency urgency);

    /**
     * Sets the window associated with this notification.
     * This is relevant when using the CloseWhenWindowActivated flag.
     *
     * @since 6.0
     */
    void setWindow(QWindow *window);

    /**
     * The window associated with this notification. nullptr by default.
     * @return the window set by setWindow()
     *
     * @since 6.0
     */
    QWindow *window() const;

    /**
     * @internal
     * appname used for the D-Bus object
     */
    QString appName() const;

    /**
     * Returns whether this notification object will be automatically deleted after closing.
     * @since 5.88
     */
    bool isAutoDelete() const;
    /**
     * Sets whether this notification object will be automatically deleted after closing.
     * This is on by default for C++, and off by default for QML.
     * @since 5.88
     */
    void setAutoDelete(bool autoDelete);

    /**
     * Returns the activation token to use to activate a window.
     * @since 5.90
     */
    QString xdgActivationToken() const;

Q_SIGNALS:
    /**
     * Emitted when the notification is closed.
     *
     * Can be closed either by the user clicking the close button,
     * the timeout running out or when an action was triggered.
     */
    void closed();

    /**
     * The notification has been ignored
     */
    void ignored();

    /**
     * Emitted when @c eventId changed.
     * @since 5.88
     */
    void eventIdChanged();
    /**
     * Emitted when @c title changed.
     * @since 5.88
     */
    void titleChanged();
    /**
     * Emitted when @c text changed.
     * @since 5.88
     */
    void textChanged();
    /**
     * Emitted when @c iconName changed.
     * @since 5.88
     */
    void iconNameChanged();
    /**
     * Emitted when @c defaultAction changed.
     * @since 5.88
     */
    void defaultActionChanged();
    /**
     * Emitted when @c actions changed.
     * @since 5.88
     */
    void actionsChanged();
    /**
     * Emitted when @p flags changed.
     * @since 5.88
     */
    void flagsChanged();
    /**
     * Emitted when @p componentName changed.
     * @since 5.88
     */
    void componentNameChanged();
    /**
     * Emitted when @p urls changed.
     * @since 5.88
     */
    void urlsChanged();
    /**
     * Emitted when @p urgency changed.
     * @since 5.88
     */
    void urgencyChanged();
    /**
     * Emitted when @p autoDelete changed.
     * @since 5.88
     */
    void autoDeleteChanged();
    /**
     * Emitted when @p xdgActivationToken changes.
     * @since 5.90
     */
    void xdgActivationTokenChanged();
    /**
     * Emitted when @p hints changes.
     * @since 5.101
     */
    void hintsChanged();

public Q_SLOTS:
    /**
     * Close the notification without activating it.
     *
     * This will delete the notification.
     */
    void close();

    /**
     * Send the notification to the server.
     *
     * This will cause all the configured plugins to execute their actions on this notification
     * (eg. a sound will play, a popup will show, a command will be executed etc).
     */
    void sendEvent();

    /**
     * @since 5.57
     * Adds a custom hint to the notification. Those are key-value pairs that can be interpreted by the respective notification backend to trigger additional,
     * non-standard features.
     * @param hint the hint's key
     * @param value the hint's value
     */
    Q_INVOKABLE void setHint(const QString &hint, const QVariant &value);

    /**
     * @since 5.57
     * Returns the custom hints set by setHint()
     */
    QVariantMap hints() const;

    /**
     * @since 5.101
     * Set custom hints on the notification.
     * @sa setHint
     */
    void setHints(const QVariantMap &hints);

private:
    friend class KNotificationManager;
    friend class NotificationWrapper;
    friend class NotifyByPopup;
    friend class NotifyByPortal;
    friend class NotifyByPortalPrivate;
    friend class NotifyByExecute;
    friend class NotifyBySnore;
    friend class NotifyByAndroid;
    friend class NotifyByMacOSNotificationCenter;
    struct Private;

    void slotWindowActiveChanged();

    /**
     * @brief Activate the action specified action
     * If the action is zero, then the default action is activated
     */
    void activate(const QString &action);

    /**
     * @internal
     * the id given by the notification manager
     */
    int id();

    /**
     * @internal
     * update the texts, the icon, and the actions of one existing notification
     */
    void update();

    /**
     * The notification will automatically be closed if all presentations are finished.
     * if you want to show your own presentation in your application, you should use this
     * function, so it will not be automatically closed when there is nothing to show.
     *
     * Don't forgot to deref, or the notification may be never closed if there is no timeout.
     *
     * @see deref
     */
    void ref();

    /**
     * Remove a reference made with ref(). If the ref counter hits zero,
     * the notification will be closed and deleted.
     *
     * @see ref
     */
    void deref();

    // Like setActions, but doesn't take ownership
    void setActionsQml(QList<KNotificationAction *> actions);
    void setDefaultActionQml(KNotificationAction *action);
    QList<KNotificationAction *> actions() const;

    static QString standardEventToEventId(StandardEvent event);
    static QString standardEventToIconName(StandardEvent event);

    std::unique_ptr<Private> const d;

public:
    /**
     * @brief emit an event
     *
     * This method creates the KNotification, setting every parameter, and fire the event.
     * You don't need to call sendEvent
     *
     * A popup may be displayed or a sound may be played, depending the config.
     *
     * @return a KNotification .  You may use that pointer to connect some signals or slot.
     * the pointer is automatically deleted when the event is closed.
     *
     * @param eventId is the name of the event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup.
     * @param pixmap is a picture which may be shown in the popup.
     * @param flags is a bitmask of NotificationFlag
     * @param componentName used to determine the location of the config file.  by default, appname is used
     * @since 4.4
     */
    static KNotification *event(const QString &eventId,
                                const QString &title,
                                const QString &text,
                                const QPixmap &pixmap = QPixmap(),
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /**
     * @brief emit a standard event
     *
     * @overload
     *
     * This will emit a standard event
     *
     * @param eventId is the name of the event
     * @param text is the text of the notification to show in the popup.
     * @param pixmap is a picture which may be shown in the popup.
     * @param flags is a bitmask of NotificationFlag
     * @param componentName used to determine the location of the config file.  by default, plasma_workspace is used
     */
    static KNotification *event(const QString &eventId,
                                const QString &text = QString(),
                                const QPixmap &pixmap = QPixmap(),
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /**
     * @brief emit a standard event
     *
     * @overload
     *
     * This will emit a standard event
     *
     * @param eventId is the name of the event
     * @param text is the text of the notification to show in the popup
     * @param pixmap is a picture which may be shown in the popup
     * @param flags is a bitmask of NotificationFlag
     */
    static KNotification *
    event(StandardEvent eventId, const QString &text = QString(), const QPixmap &pixmap = QPixmap(), const NotificationFlags &flags = CloseOnTimeout);

    /**
     * @brief emit a standard event
     *
     * @overload
     *
     * This will emit a standard event
     *
     * @param eventId is the name of the event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup
     * @param pixmap is a picture which may be shown in the popup
     * @param flags is a bitmask of NotificationFlag
     * @since 4.4
     */
    static KNotification *
    event(StandardEvent eventId, const QString &title, const QString &text, const QPixmap &pixmap, const NotificationFlags &flags = CloseOnTimeout);

    /**
     * @brief emit a standard event with the possibility of setting an icon by icon name
     *
     * @overload
     *
     * This will emit a standard event
     *
     * @param eventId is the name of the event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup
     * @param iconName a Freedesktop compatible icon name to be shown in the popup
     * @param flags is a bitmask of NotificationFlag
     * @param componentName used to determine the location of the config file.  by default, plasma_workspace is used
     * @since 5.4
     */
    static KNotification *event(const QString &eventId,
                                const QString &title,
                                const QString &text,
                                const QString &iconName,
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /**
     * @brief emit a standard event with the possibility of setting an icon by icon name
     *
     * @overload
     *
     * This will emit a standard event with a custom icon
     *
     * @param eventId the type of the standard (not app-defined) event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup
     * @param iconName a Freedesktop compatible icon name to be shown in the popup
     * @param flags is a bitmask of NotificationFlag
     * @since 5.9
     */
    static KNotification *
    event(StandardEvent eventId, const QString &title, const QString &text, const QString &iconName, const NotificationFlags &flags = CloseOnTimeout);

    /**
     * @brief emit a standard event
     *
     * @overload
     *
     * This will emit a standard event with its standard icon
     *
     * @param eventId the type of the standard (not app-defined) event
     * @param title is title of the notification to show in the popup.
     * @param text is the text of the notification to show in the popup
     * @param flags is a bitmask of NotificationFlag
     * @since 5.9
     */
    static KNotification *event(StandardEvent eventId, const QString &title, const QString &text, const NotificationFlags &flags = CloseOnTimeout);

    /**
     * This is a simple substitution for QApplication::beep()
     *
     * @param reason a short text explaining what has happened (may be empty)
     */
    static void beep(const QString &reason = QString());

    // prevent warning
    using QObject::event;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KNotification::NotificationFlags)

#endif
