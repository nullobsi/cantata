/*
    Copyright 2010 Rafael Fernández López <ereslibre@kde.org>

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

#ifndef SOLID_BACKENDS_UDEV_UDEVMANAGER_H
#define SOLID_BACKENDS_UDEV_UDEVMANAGER_H

#include <solid-lite/ifaces/devicemanager.h>

#include "../shared/udevqt.h"

namespace Solid {
namespace Backends {
namespace UDev {
class UDevManager : public Solid::Ifaces::DeviceManager {
	Q_OBJECT

public:
	UDevManager(QObject* parent);
	~UDevManager() override;

	QString udiPrefix() const override;
	QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;

	QStringList allDevices() override;

	QStringList devicesFromQuery(const QString& parentUdi,
	                             Solid::DeviceInterface::Type type) override;

	QObject* createDevice(const QString& udi) override;

private Q_SLOTS:
	void slotDeviceAdded(const UdevQt::Device& device);
	void slotDeviceRemoved(const UdevQt::Device& device);

private:
	class Private;
	Private* const d;
};
}// namespace UDev
}// namespace Backends
}// namespace Solid

#endif// SOLID_BACKENDS_UDEV_UDEVMANAGER_H
