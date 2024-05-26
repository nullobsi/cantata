/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNOTIFYCONFIG_H
#define KNOTIFYCONFIG_H

#include <QMap>

class KNotifyConfigPrivate;

/**
 * @class KNotifyConfig knotifyconfig.h KNotifyConfig
 *
 * Represent the configuration for an event
 *
 * @author Olivier Goffart <ogoffart@kde.org>
 * @author Kai Uwe Broulik <kde@broulik.de>
 */
class KNotifyConfig
{
public:
	KNotifyConfig(const QString& id);

    /**
     * the name of the notification
     */
    QString eventId() const;

    /**
     * @return entry from the relevant Global notifyrc config group
     *
     * This will return the configuration from the user for the given key.
     * It first look into the user config file, and then in the global config file.
     *
     * return a null string if the entry doesn't exist
     */
    QString readGlobalEntry(const QString &key) const;

    /**
     * @return entry from the relevant Event/ notifyrc config group
     *
     * This will return the configuration from the user for the given key.
     * It first look into the user config file, and then in the global config file.
     *
     * return a null string if the entry doesn't exist
     */
    QString readEntry(const QString &key) const;

    /**
     * @return path entry from the relevant Event/ notifyrc config group
     *
     * This will return the configuration from the user for the given key
     * and interpret it as a path.
     */
    QString readPathEntry(const QString &key) const;

	void setEntry(const QString &key, const QString &val);

	static void setGlobalEntry(const QString &key, const QString& val);

private:
	QString m_eventId;
};
#endif
