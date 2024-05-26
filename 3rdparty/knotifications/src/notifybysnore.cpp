/*
    SPDX-FileCopyrightText: 2019 Piyush Aggarwal <piyushaggarwal002@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notifybysnore.h"
#include "knotification.h"
#include "knotifyconfig.h"
#include "knotificationreplyaction.h"

#include <QGuiApplication>
#include <QIcon>
#include <QLocalSocket>

#include <snoretoastactions.h>

/*
 * On Windows a shortcut to your app is needed to be installed in the Start Menu
 * (and subsequently, registered with the OS) in order to show notifications.
 * Since KNotifications is a library, an app using it can't (feasibly) be properly
 * registered with the OS. It is possible we could come up with some complicated solution
 * which would require every KNotification-using app to do some special and probably
 * difficult to understand change to support Windows. Or we can have SnoreToast.exe
 * take care of all that nonsense for us.
 * Note that, up to this point, there have been no special
 * KNotifications changes to the generic application codebase to make this work,
 * just some tweaks to the Craft blueprint and packaging script
 * to pull in SnoreToast and trigger shortcut building respectively.
 * Be sure to have a shortcut installed in Windows Start Menu by SnoreToast.
 *
 * So the location doesn't matter, but it's only possible to register the internal COM server in an executable.
 * We could make it a static lib and link it in all KDE applications,
 * but to make the action center integration work, we would need to also compile a class
 * into the executable using a compile time uuid.
 *
 * The used header is meant to help with parsing the response.
 * The cmake target for LibSnoreToast is a INTERFACE lib, it only provides the include path.
 *
 *
 * Trigger the shortcut installation during the installation of your app; syntax for shortcut installation is -
 * ./SnoreToast.exe -install <absolute\address\of\shortcut> <absolute\address\to\app.exe> <appID>
 *
 * appID: use as-is from your app's QCoreApplication::applicationName() when installing the shortcut.
 * NOTE: Install the shortcut in Windows Start Menu folder.
 * For example, check out Craft Blueprint for Quassel-IRC or KDE Connect.
 */

namespace
{
const QString SnoreToastExecName()
{
    return QStringLiteral("SnoreToast.exe");
}
}

NotifyBySnore::NotifyBySnore(QObject *parent)
    : KNotificationPlugin(parent)
{
    m_server.listen(QString::number(qHash(qApp->applicationDirPath())));
    connect(&m_server, &QLocalServer::newConnection, this, [this]() {
        QLocalSocket *responseSocket = m_server.nextPendingConnection();
        connect(responseSocket, &QLocalSocket::readyRead, [this, responseSocket]() {
            const QByteArray rawNotificationResponse = responseSocket->readAll();
            responseSocket->deleteLater();

            const QString notificationResponse = QString::fromWCharArray(reinterpret_cast<const wchar_t *>(rawNotificationResponse.constData()),
                                                                         rawNotificationResponse.size() / sizeof(wchar_t));
            //qCDebug(LOG_KNOTIFICATIONS) << notificationResponse;

            QMap<QString, QStringView> notificationResponseMap;
            for (const auto str : QStringView(notificationResponse).split(QLatin1Char(';'))) {
                const int equalIndex = str.indexOf(QLatin1Char('='));
                notificationResponseMap.insert(str.mid(0, equalIndex).toString(), str.mid(equalIndex + 1));
            }

            const QString responseAction = notificationResponseMap[QStringLiteral("action")].toString();
            const int responseNotificationId = notificationResponseMap[QStringLiteral("notificationId")].toInt();

            //qCDebug(LOG_KNOTIFICATIONS) << "The notification ID is : " << responseNotificationId;

            KNotification *notification;
            const auto iter = m_notifications.constFind(responseNotificationId);
            if (iter != m_notifications.constEnd()) {
                notification = iter.value();
            } else {
                //qCWarning(LOG_KNOTIFICATIONS) << "Received a response for an unknown notification.";
                return;
            }

            std::wstring w_action(responseAction.size(), 0);
            responseAction.toWCharArray(const_cast<wchar_t *>(w_action.data()));

            switch (SnoreToastActions::getAction(w_action)) {
            case SnoreToastActions::Actions::Clicked:
                //qCDebug(LOG_KNOTIFICATIONS) << "User clicked on the toast.";
                break;

            case SnoreToastActions::Actions::Hidden:
                //qCDebug(LOG_KNOTIFICATIONS) << "The toast got hidden.";
                break;

            case SnoreToastActions::Actions::Dismissed:
                //qCDebug(LOG_KNOTIFICATIONS) << "User dismissed the toast.";
                break;

            case SnoreToastActions::Actions::Timedout:
                //qCDebug(LOG_KNOTIFICATIONS) << "The toast timed out.";
                break;

            case SnoreToastActions::Actions::ButtonClicked: {
                //qCDebug(LOG_KNOTIFICATIONS) << "User clicked an action button in the toast.";
                const QString responseButton = notificationResponseMap[QStringLiteral("button")].toString();
                Q_EMIT actionInvoked(responseNotificationId, responseButton);
                break;
            }

            case SnoreToastActions::Actions::TextEntered: {
                //qCWarning(LOG_KNOTIFICATIONS) << "User entered some text in the toast.";
                const QString replyText = notificationResponseMap[QStringLiteral("text")].toString();
                //qCWarning(LOG_KNOTIFICATIONS) << "Text entered was :: " << replyText;
                Q_EMIT replied(responseNotificationId, replyText);
                break;
            }

            default:
                //qCWarning(LOG_KNOTIFICATIONS) << "Unexpected behaviour with the toast. Please file a bug report / feature request.";
                break;
            }

            // Action Center callbacks are not yet supported so just close the notification once done
            if (notification != nullptr) {
                NotifyBySnore::close(notification);
            }
        });
    });
}

NotifyBySnore::~NotifyBySnore()
{
    m_server.close();
}

void NotifyBySnore::notify(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    Q_UNUSED(notifyConfig);
    // HACK work around that notification->id() is only populated after returning from here
    // note that config will be invalid at that point, so we can't pass that along
    QMetaObject::invokeMethod(
        this,
        [this, notification]() {
            NotifyBySnore::notifyDeferred(notification);
        },
        Qt::QueuedConnection);
}

void NotifyBySnore::notifyDeferred(KNotification *notification)
{
    m_notifications.insert(notification->id(), notification);

    const QString notificationTitle = ((!notification->title().isEmpty()) ? notification->title() : qApp->applicationDisplayName());
    QStringList snoretoastArgsList{QStringLiteral("-id"),
                                   QString::number(notification->id()),
                                   QStringLiteral("-t"),
                                   notificationTitle,
                                   QStringLiteral("-m"),
                                   stripRichText(notification->text()),
                                   QStringLiteral("-appID"),
                                   qApp->applicationName(),
                                   QStringLiteral("-pid"),
                                   QString::number(qApp->applicationPid()),
                                   QStringLiteral("-pipename"),
                                   m_server.fullServerName()};

    // handle the icon for toast notification
    const QString iconPath = m_iconDir.path() + QLatin1Char('/') + QString::number(notification->id());
    const bool hasIcon = (notification->pixmap().isNull()) ? qApp->windowIcon().pixmap(1024, 1024).save(iconPath, "PNG") //
                                                           : notification->pixmap().save(iconPath, "PNG");
    if (hasIcon) {
        snoretoastArgsList << QStringLiteral("-p") << iconPath;
    }

    // if'd below, because SnoreToast currently doesn't support both textbox and buttons in the same notification
    if (notification->replyAction()) {
        snoretoastArgsList << QStringLiteral("-tb");
    } else if (!notification->actions().isEmpty()) {
        // add actions if any

        const auto actions = notification->actions();
        QStringList actionLabels;

        for (KNotificationAction *action : actions) {
            action->setId(action->label());
            actionLabels << action->label();
        }

        snoretoastArgsList << QStringLiteral("-b") << actionLabels.join(QLatin1Char(';'));
    }

    QProcess *snoretoastProcess = new QProcess();
    connect(snoretoastProcess, &QProcess::readyReadStandardError, [snoretoastProcess, snoretoastArgsList]() {
        const auto data = snoretoastProcess->readAllStandardError();
        //qCDebug(LOG_KNOTIFICATIONS) << "SnoreToast process stderr:" << snoretoastArgsList << data;
    });
    connect(snoretoastProcess, &QProcess::readyReadStandardOutput, [snoretoastProcess, snoretoastArgsList]() {
        const auto data = snoretoastProcess->readAllStandardOutput();
        //qCDebug(LOG_KNOTIFICATIONS) << "SnoreToast process stdout:" << snoretoastArgsList << data;
    });
    connect(snoretoastProcess, &QProcess::errorOccurred, this, [this, snoretoastProcess, snoretoastArgsList, iconPath](QProcess::ProcessError error) {
        //qCWarning(LOG_KNOTIFICATIONS) << "SnoreToast process errored:" << snoretoastArgsList << error;
        snoretoastProcess->deleteLater();
        QFile::remove(iconPath);
    });
    connect(snoretoastProcess,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this, snoretoastProcess, snoretoastArgsList, iconPath](int exitCode, QProcess::ExitStatus exitStatus) {
                //qCDebug(LOG_KNOTIFICATIONS) << "SnoreToast process finished:" << snoretoastArgsList;
                //qCDebug(LOG_KNOTIFICATIONS) << "code:" << exitCode << "status:" << exitStatus;
                snoretoastProcess->deleteLater();
                QFile::remove(iconPath);
            });

    //qCDebug(LOG_KNOTIFICATIONS) << "SnoreToast process starting:" << snoretoastArgsList;
    snoretoastProcess->start(SnoreToastExecName(), snoretoastArgsList);
}

void NotifyBySnore::close(KNotification *notification)
{
    //qCDebug(LOG_KNOTIFICATIONS) << "Requested to close notification with ID:" << notification->id();
    if (m_notifications.constFind(notification->id()) == m_notifications.constEnd()) {
        //qCWarning(LOG_KNOTIFICATIONS) << "Couldn't find the notification in m_notifications. Nothing to close.";
        return;
    }

    m_notifications.remove(notification->id());

    const QStringList snoretoastArgsList{QStringLiteral("-close"), QString::number(notification->id()), QStringLiteral("-appID"), qApp->applicationName()};

    //qCDebug(LOG_KNOTIFICATIONS) << "Closing notification; SnoreToast process arguments:" << snoretoastArgsList;
    QProcess::startDetached(SnoreToastExecName(), snoretoastArgsList);

    finish(notification);
}

void NotifyBySnore::update(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    Q_UNUSED(notification);
    Q_UNUSED(notifyConfig);
    //qCWarning(LOG_KNOTIFICATIONS) << "updating a notification is not supported yet.";
}

#include "moc_notifybysnore.cpp"
