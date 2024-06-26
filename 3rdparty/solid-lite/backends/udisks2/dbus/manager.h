/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -p manager manager.xml
 *
 * qdbusxml2cpp is Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef MANAGER_H_1329493525
#define MANAGER_H_1329493525

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

#include "../udisks2.h"

/*
 * Proxy class for interface org.freedesktop.DBus.ObjectManager
 */
class OrgFreedesktopDBusObjectManagerInterface : public QDBusAbstractInterface {
	Q_OBJECT
public:
	static inline const char* staticInterfaceName()
	{
		return "org.freedesktop.DBus.ObjectManager";
	}

public:
	OrgFreedesktopDBusObjectManagerInterface(const QString& service, const QString& path, const QDBusConnection& connection, QObject* parent = nullptr);

	~OrgFreedesktopDBusObjectManagerInterface() override;

public Q_SLOTS:// METHODS
	inline QDBusPendingReply<DBUSManagerStruct> GetManagedObjects()
	{
		QList<QVariant> argumentList;
		return asyncCallWithArgumentList(QLatin1String("GetManagedObjects"), argumentList);
	}

Q_SIGNALS:// SIGNALS
	void InterfacesAdded(const QDBusObjectPath& object_path, const QVariantMapMap& interfaces_and_properties);
	void InterfacesRemoved(const QDBusObjectPath& object_path, const QStringList& interfaces);
};

namespace org {
namespace freedesktop {
namespace DBus {
typedef ::OrgFreedesktopDBusObjectManagerInterface ObjectManager;
}
}// namespace freedesktop
}// namespace org
#endif
