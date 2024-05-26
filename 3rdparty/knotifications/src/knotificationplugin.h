/*
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNOTIFICATIONPLUGIN_H
#define KNOTIFICATIONPLUGIN_H

#include <QObject>
#include <QTextDocumentFragment>

#include <memory>

class KNotification;
class KNotificationPluginPrivate;
class KNotifyConfig;

/**
 * @class KNotificationPlugin knotificationplugin.h KNotificationPlugin
 *
 * @brief abstract class for KNotification actions
 *
 * A KNotificationPlugin is responsible of notification presentation.
 * You can subclass it to have your own presentation of a notification.
 *
 * You should reimplement the KNotificationPlugin::notify method to display the notification.
 *
 * Porting from KF5 to KF6:
 *
 * The signature of the virtual method
 * KNotificationPlugin::notify(KNotification *notification, KNotifyConfig *)
 * was changed to
 * KNotificationPlugin::notify(KNotification *notification, const KNotifyConfig &).
 *
 * The signature of the virtual method
 * KNotificationPlugin::update(KNotification *notification, KNotifyConfig *)
 * was changed to
 * KNotificationPlugin::update(KNotification *notification, const KNotifyConfig &).
 *
 * @author Olivier Goffart <ogoffart at kde.org>
 */
class KNotificationPlugin : public QObject
{
    Q_OBJECT

public:
    KNotificationPlugin(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~KNotificationPlugin() override;

    /**
     * @brief return the name of this plugin.
     *
     * this is the name that should appear in the .notifyrc file,
     * in the field Action=... if a notification is set to use this plugin
     */
    virtual QString optionName() = 0;

    /**
     * This function is called when the notification is sent.
     * (or re-sent)
     * You should implement this function to display a notification
     *
     * for each call to this function (even for re-notification), you MUST call finish(KNotification*)
     *
     * @param notification is the KNotification object
     * @param notifyConfig is the configuration of the notification
     */
    virtual void notify(KNotification *notification, const KNotifyConfig &notifyConfig) = 0;

    /**
     * This function is called when the notification has changed (such as the text or the icon)
     */
    virtual void update(KNotification *notification, const KNotifyConfig &notifyConfig);

    /**
     * This function is called when the notification has been closed
     */
    virtual void close(KNotification *notification);

protected:
    /**
     * emit the finished signal
     * you MUST call this function for each call to notify(), even if you do nothing there
     *
     * call it when the presentation is finished (because the user closed the popup or the sound is finished)
     *
     * If your presentation is synchronous, you can even call this function from the notify() call itself
     */
    void finish(KNotification *notification);

    static inline QString stripRichText(const QString &s)
    {
        return QTextDocumentFragment::fromHtml(s).toPlainText();
    }

Q_SIGNALS:
    /**
     * the presentation is finished.
     */
    void finished(KNotification *notification);
    /**
     * emit this signal if one action was invoked
     * @param id is the id of the notification
     * @param action is the action number.  zero for the default action
     */
    void actionInvoked(int id, const QString &action);

    void xdgActivationTokenReceived(int id, const QString &token);

    void replied(int id, const QString &text);

private:
    std::unique_ptr<KNotificationPluginPrivate> const d;
};

#endif
