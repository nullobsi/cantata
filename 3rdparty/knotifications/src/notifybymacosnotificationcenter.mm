/*
    SPDX-FileCopyrightText: 2019-2020 Weixuan XIAO <veyx.shaw at gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notifybymacosnotificationcenter.h"
#include "knotification.h"
#include "knotifyconfig.h"

#include <QIcon>
#include <QDebug>

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>


Q_FORWARD_DECLARE_OBJC_CLASS(MacOSNotificationCenterDelegate);

class MacOSNotificationCenterPrivate
{
public:
    static MacOSNotificationCenterPrivate *instance();
    ~MacOSNotificationCenterPrivate();

    QMap<int, KNotification*> m_notifications;

    int m_internalCounter;
private:
    MacOSNotificationCenterPrivate();
    MacOSNotificationCenterDelegate *m_delegate;
    static MacOSNotificationCenterPrivate *m_instance;
};

@interface MacOSNotificationCenterDelegate : NSObject<NSUserNotificationCenterDelegate> {}
@end

@implementation MacOSNotificationCenterDelegate
- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
    Q_UNUSED(center);
    Q_UNUSED(notification);
    return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didDeliverNotification:(NSUserNotification *)notification
{
    Q_UNUSED(center);
    Q_UNUSED(notification);
    //qCDebug(LOG_KNOTIFICATIONS) << "Send notification " << [notification.userInfo[@"id"] intValue];
}


@end

MacOSNotificationCenterPrivate *MacOSNotificationCenterPrivate::m_instance = nullptr;

MacOSNotificationCenterPrivate::MacOSNotificationCenterPrivate()
    : m_internalCounter(0)
{
    // Set delegate
    m_delegate = [MacOSNotificationCenterDelegate alloc];
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:m_delegate];
}

MacOSNotificationCenterPrivate::~MacOSNotificationCenterPrivate()
{
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate: nil];
    [m_delegate release];

    // Try to finish all NSNotification
    NSArray<NSUserNotification *> *deliveredNotifications = [NSUserNotificationCenter defaultUserNotificationCenter].deliveredNotifications;
    for (NSUserNotification *deliveredNotification in deliveredNotifications) {
        // Remove NSNotification in notification center
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeDeliveredNotification: deliveredNotification];
    }
}

MacOSNotificationCenterPrivate *MacOSNotificationCenterPrivate::instance() {
    if (!m_instance) {
        m_instance = new MacOSNotificationCenterPrivate();
    }
    return m_instance;
}


NotifyByMacOSNotificationCenter::NotifyByMacOSNotificationCenter(QObject* parent)
    : KNotificationPlugin(parent)
{
    // Clear notifications
    NSArray<NSUserNotification *> *deliveredNotifications = [NSUserNotificationCenter defaultUserNotificationCenter].deliveredNotifications;
    for (NSUserNotification *deliveredNotification in deliveredNotifications) {
        // Remove NSNotification in notification center
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeDeliveredNotification: deliveredNotification];
    }
}

NotifyByMacOSNotificationCenter::~NotifyByMacOSNotificationCenter() {}

void NotifyByMacOSNotificationCenter::notify(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    Q_UNUSED(notifyConfig);

    int internalId = MacOSNotificationCenterPrivate::instance()->m_internalCounter++;
    NSUserNotification *osxNotification = [[[NSUserNotification alloc] init] autorelease];
    NSString *notificationId = [NSString stringWithFormat: @"%d", notification->id()];
    NSString *internalNotificationId = [NSString stringWithFormat: @"%d", internalId];
    NSString *title = notification->title().toNSString();
    NSString *text = notification->text().toNSString();

    osxNotification.title = title;
    osxNotification.userInfo = [NSDictionary dictionaryWithObjectsAndKeys: notificationId, @"id",
        internalNotificationId, @"internalId", nil];
    osxNotification.informativeText = text;

    if (notification->pixmap().isNull()) {
        QIcon notificationIcon = QIcon::fromTheme(notification->iconName());
        if (!notificationIcon.isNull()) {
            osxNotification.contentImage = [[NSImage alloc]
                initWithCGImage: notificationIcon.pixmap(QSize(64, 64)).toImage().toCGImage() size: NSMakeSize(64, 64)];
        }
    } else {
        osxNotification.contentImage = [[NSImage alloc]
            initWithCGImage: notification->pixmap().toImage().toCGImage() size: NSMakeSize(64, 64)];
    }

    if (notification->actions().isEmpty()) {
        // Remove all buttons
        osxNotification.hasReplyButton = false;
        osxNotification.hasActionButton = false;
    } else {
        osxNotification.hasActionButton = true;
        // Workaround: this "API" will cause refuse from Apple
        // [osxNotification setValue:[NSNumber numberWithBool:YES] forKey: @"_alwaysShowAlternateActionMenu"];

        // Assign first action to action button
        if (!notification->actions().empty()) {
            osxNotification.actionButtonTitle = notification->actions().at(0)->label().toNSString();
        }

        // Construct a list for all actions left for additional buttons
        NSMutableArray<NSUserNotificationAction*> *actions = [[NSMutableArray alloc] init];
        for (int index = 1; index < notification->actions().length(); index++) {
            NSUserNotificationAction *action =
                [NSUserNotificationAction actionWithIdentifier: [NSString stringWithFormat:@"%d", index]
                                          title: notification->actions().at(index)->label().toNSString()];
            [actions addObject: action];
        }
        osxNotification.additionalActions = actions;
    }

    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification: osxNotification];

    MacOSNotificationCenterPrivate::instance()->m_notifications.insert(internalId, notification);
}

void NotifyByMacOSNotificationCenter::close(KNotification *notification)
{
    //qCDebug(LOG_KNOTIFICATIONS) << "Remove notification " << notification->id();

    NSArray<NSUserNotification *> *deliveredNotifications = [NSUserNotificationCenter defaultUserNotificationCenter].deliveredNotifications;
    for (NSUserNotification *deliveredNotification in deliveredNotifications) {
        if ([deliveredNotification.userInfo[@"id"] intValue] == notification->id()) {
            // Remove KNotification in mapping
            int internalId = [deliveredNotification.userInfo[@"id"] intValue];

            MacOSNotificationCenterPrivate::instance()->m_notifications.remove(internalId);

            // Remove NSNotification in notification center
            [[NSUserNotificationCenter defaultUserNotificationCenter] removeDeliveredNotification: deliveredNotification];
            finish(notification);
            return;
        }
    }
    //qCDebug(LOG_KNOTIFICATIONS) <<  "Notification " << notification->id() << " not found";
    finish(notification);
}

void NotifyByMacOSNotificationCenter::update(KNotification *notification, const KNotifyConfig &notifyConfig)
{
    close(notification);
    notify(notification, notifyConfig);
}

#include "moc_notifybymacosnotificationcenter.cpp"
