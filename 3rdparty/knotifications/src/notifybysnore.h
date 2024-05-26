/*
    SPDX-FileCopyrightText: 2019 Piyush Aggarwal <piyushaggarwal002@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOTIFYBYSNORE_H
#define NOTIFYBYSNORE_H

#include "knotificationplugin.h"

#include <QLocalServer>
#include <QPointer>
#include <QProcess>
#include <QTemporaryDir>

class NotifyBySnore : public KNotificationPlugin {
	Q_OBJECT

public:
	explicit NotifyBySnore(QObject* parent = nullptr);
	~NotifyBySnore() override;

	QString optionName() override
	{
		return QStringLiteral("Popup");
	}
	void notify(KNotification* notification, const KNotifyConfig& notifyConfig) override;
	void notifyDeferred(KNotification* notification);
	void close(KNotification* notification) override;
	void update(KNotification* notification, const KNotifyConfig& notifyConfig) override;

private:
	QHash<int, QPointer<KNotification>> m_notifications;
	QLocalServer m_server;
	QTemporaryDir m_iconDir;
};

#endif// NOTIFYBYSNORE_H
