/*
    Copyright 2005-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "devicemanager_p.h"//krazy:exclude=includes (devicenotifier.h is the header file for this class)
#include "devicenotifier.h"

#include "device.h"
#include "device_p.h"
#include "predicate.h"

#include "ifaces/device.h"
#include "ifaces/devicemanager.h"

#include "soliddefs_p.h"

SOLID_GLOBAL_STATIC(Solid::DeviceManagerStorage, globalDeviceStorage)

#if QT_VERSION < 0x050000
#define QtPointer QWeakPointer
#else
#define QtPointer QPointer
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
template<typename T>
QSet<T> listToSet(const QList<T>& list) { return QSet<T>(list.cbegin(), list.cend()); }
#else
template<typename T>
static inline QSet<T> listToSet(const QList<T>& list) { return list.toSet(); }
#endif

Solid::DeviceManagerPrivate::DeviceManagerPrivate()
	: m_nullDevice(new DevicePrivate(QString()))
{
	loadBackends();

	QList<QObject*> backends = managerBackends();
	for (QObject* backend : backends) {
		connect(backend, SIGNAL(deviceAdded(QString)),
		        this, SLOT(_k_deviceAdded(QString)));
		connect(backend, SIGNAL(deviceRemoved(QString)),
		        this, SLOT(_k_deviceRemoved(QString)));
	}
}

Solid::DeviceManagerPrivate::~DeviceManagerPrivate()
{
	QList<QObject*> backends = managerBackends();
	for (QObject* backend : backends) {
		disconnect(backend, nullptr, this, nullptr);
	}

	for (const QtPointer<DevicePrivate>& dev : m_devicesMap) {
		if (dev.data() && !dev.data()->ref.deref()) {
			dev.data()->deleteLater();
		}
	}

	m_devicesMap.clear();
}

QList<Solid::Device> Solid::Device::allDevices()
{
	QList<Device> list;
	QList<QObject*> backends = globalDeviceStorage->managerBackends();

	for (QObject* backendObj : backends) {
		Ifaces::DeviceManager* backend = qobject_cast<Ifaces::DeviceManager*>(backendObj);

		if (backend == nullptr) continue;

		QStringList udis = backend->allDevices();

		for (const QString& udi : udis) {
			list.append(Device(udi));
		}
	}

	return list;
}

QList<Solid::Device> Solid::Device::listFromQuery(const QString& predicate,
                                                  const QString& parentUdi)
{
	Predicate p = Predicate::fromString(predicate);

	if (p.isValid()) {
		return listFromQuery(p, parentUdi);
	}
	else {
		return QList<Device>();
	}
}

QList<Solid::Device> Solid::Device::listFromType(const DeviceInterface::Type& type,
                                                 const QString& parentUdi)
{
	QList<Device> list;
	QList<QObject*> backends = globalDeviceStorage->managerBackends();

	for (QObject* backendObj : backends) {
		Ifaces::DeviceManager* backend = qobject_cast<Ifaces::DeviceManager*>(backendObj);

		if (backend == nullptr) continue;
		if (!backend->supportedInterfaces().contains(type)) continue;

		QStringList udis = backend->devicesFromQuery(parentUdi, type);

		for (const QString& udi : udis) {
			list.append(Device(udi));
		}
	}

	return list;
}

QList<Solid::Device> Solid::Device::listFromQuery(const Predicate& predicate,
                                                  const QString& parentUdi)
{
	QList<Device> list;
	QList<QObject*> backends = globalDeviceStorage->managerBackends();
	QSet<DeviceInterface::Type> usedTypes = predicate.usedTypes();

	for (QObject* backendObj : backends) {
		Ifaces::DeviceManager* backend = qobject_cast<Ifaces::DeviceManager*>(backendObj);

		if (backend == nullptr) continue;

		QSet<QString> udis;
		if (predicate.isValid()) {
			QSet<DeviceInterface::Type> supportedTypes = backend->supportedInterfaces();
			if (supportedTypes.intersect(usedTypes).isEmpty()) {
				continue;
			}

			for (DeviceInterface::Type type : supportedTypes) {
				udis += listToSet(backend->devicesFromQuery(parentUdi, type));
			}
		}
		else {
			udis += listToSet(backend->allDevices());
		}

		for (const QString& udi : udis) {
			Device dev(udi);

			bool matches = false;

			if (!predicate.isValid()) {
				matches = true;
			}
			else {
				matches = predicate.matches(dev);
			}

			if (matches) {
				list.append(dev);
			}
		}
	}

	return list;
}

Solid::DeviceNotifier* Solid::DeviceNotifier::instance()
{
	return globalDeviceStorage->notifier();
}

void Solid::DeviceManagerPrivate::_k_deviceAdded(const QString& udi)
{
	if (m_devicesMap.contains(udi)) {
		DevicePrivate* dev = m_devicesMap[udi].data();

		// Ok, this one was requested somewhere was invalid
		// and now becomes magically valid!

		if (dev && dev->backendObject() == nullptr) {
			dev->setBackendObject(createBackendObject(udi));
			Q_ASSERT(dev->backendObject() != nullptr);
		}
	}

	emit deviceAdded(udi);
}

void Solid::DeviceManagerPrivate::_k_deviceRemoved(const QString& udi)
{
	if (m_devicesMap.contains(udi)) {
		DevicePrivate* dev = m_devicesMap[udi].data();

		// Ok, this one was requested somewhere was valid
		// and now becomes magically invalid!

		if (dev) {
			Q_ASSERT(dev->backendObject() != nullptr);
			dev->setBackendObject(nullptr);
			Q_ASSERT(dev->backendObject() == nullptr);
		}
	}

	emit deviceRemoved(udi);
}

void Solid::DeviceManagerPrivate::_k_destroyed(QObject* object)
{
	QString udi = m_reverseMap.take(object);

	if (!udi.isEmpty()) {
		m_devicesMap.remove(udi);
	}
}

Solid::DevicePrivate* Solid::DeviceManagerPrivate::findRegisteredDevice(const QString& udi)
{
	if (udi.isEmpty()) {
		return m_nullDevice.data();
	}
	else if (m_devicesMap.contains(udi)) {
		return m_devicesMap[udi].data();
	}
	else {
		Ifaces::Device* iface = createBackendObject(udi);

		DevicePrivate* devData = new DevicePrivate(udi);
		devData->setBackendObject(iface);

		QtPointer<DevicePrivate> ptr(devData);
		m_devicesMap[udi] = ptr;
		m_reverseMap[devData] = udi;

		connect(devData, SIGNAL(destroyed(QObject*)),
		        this, SLOT(_k_destroyed(QObject*)));

		return devData;
	}
}

Solid::Ifaces::Device* Solid::DeviceManagerPrivate::createBackendObject(const QString& udi)
{
	QList<QObject*> backends = globalDeviceStorage->managerBackends();

	for (QObject* backendObj : backends) {
		Ifaces::DeviceManager* backend = qobject_cast<Ifaces::DeviceManager*>(backendObj);

		if (backend == nullptr) continue;
		if (!udi.startsWith(backend->udiPrefix())) continue;

		Ifaces::Device* iface = nullptr;

		QObject* object = backend->createDevice(udi);
		iface = qobject_cast<Ifaces::Device*>(object);

		if (iface == nullptr) {
			delete object;
		}

		return iface;
	}

	return nullptr;
}

Solid::DeviceManagerStorage::DeviceManagerStorage()
{
}

QList<QObject*> Solid::DeviceManagerStorage::managerBackends()
{
	ensureManagerCreated();
	return m_storage.localData()->managerBackends();
}

Solid::DeviceNotifier* Solid::DeviceManagerStorage::notifier()
{
	ensureManagerCreated();
	return m_storage.localData();
}

void Solid::DeviceManagerStorage::ensureManagerCreated()
{
	if (!m_storage.hasLocalData()) {
		m_storage.setLocalData(new DeviceManagerPrivate());
	}
}

#include "moc_devicemanager_p.cpp"
#include "moc_devicenotifier.cpp"
