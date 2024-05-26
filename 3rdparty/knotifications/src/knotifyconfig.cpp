/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "knotifyconfig.h"

#include <QCache>
#include <QStandardPaths>

#include<QMap>
#include <QPair>

// I don't want to use KConfig so this is our "global" config.
typedef QMap<QString, QString> PropMap;
Q_GLOBAL_STATIC(PropMap, globalProps);

typedef QMap<QPair<QString, QString>, QString> DoublePropMap;
Q_GLOBAL_STATIC(DoublePropMap, localProps);

void KNotifyConfig::setGlobalEntry(const QString &key, const QString &val) {
	globalProps->insert(key, val);
}

KNotifyConfig::KNotifyConfig(const QString& id):
	m_eventId(id) {}

QString KNotifyConfig::eventId() const
{
    return m_eventId;
}

QString KNotifyConfig::readGlobalEntry(const QString &key) const
{
	if (globalProps->contains(key)) {
		return globalProps->value(key);
	}
	return QString();
}

QString KNotifyConfig::readEntry(const QString &key) const
{
	auto pair = qMakePair(m_eventId, key);
	if (localProps->contains(qMakePair(m_eventId, key))) {
		return localProps->value(pair);
	}
	return QString();
}

void KNotifyConfig::setEntry(const QString &key, const QString &val) {
	auto pair = qMakePair(m_eventId, key);
	localProps->insert(pair, val);
}
