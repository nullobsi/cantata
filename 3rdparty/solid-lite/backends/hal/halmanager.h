/*
    Copyright 2005,2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BACKENDS_HAL_HALMANAGER_H
#define SOLID_BACKENDS_HAL_HALMANAGER_H

#include <solid-lite/deviceinterface.h>
#include <solid-lite/ifaces/devicemanager.h>

#include <QStringList>
#include <QVariant>

namespace Solid {
namespace Backends {
namespace Hal {
class HalManagerPrivate;

class HalManager : public Solid::Ifaces::DeviceManager {
	Q_OBJECT

public:
	HalManager(QObject* parent);
	~HalManager() override;

	QString udiPrefix() const override;
	QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;

	bool deviceExists(const QString& udi);
	QStringList allDevices() override;

	QStringList devicesFromQuery(const QString& parentUdi,
	                             Solid::DeviceInterface::Type type) override;

	QObject* createDevice(const QString& udi) override;

private Q_SLOTS:
	void slotDeviceAdded(const QString& udi);
	void slotDeviceRemoved(const QString& udi);

private:
	HalManagerPrivate* d;
};
}// namespace Hal
}// namespace Backends
}// namespace Solid

#endif// SOLID_BACKENDS_HAL_HALMANAGER_H
