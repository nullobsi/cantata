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

#ifndef SOLID_BACKENDS_UDEV_PORTABLEMEDIAPLAYER_H
#define SOLID_BACKENDS_UDEV_PORTABLEMEDIAPLAYER_H

#include "udevdeviceinterface.h"
#include <solid-lite/ifaces/portablemediaplayer.h>

#include <QStringList>

namespace Solid {
namespace Backends {
namespace UDev {
class UDevDevice;

class PortableMediaPlayer : public DeviceInterface, virtual public Solid::Ifaces::PortableMediaPlayer {
	Q_OBJECT
	Q_INTERFACES(Solid::Ifaces::PortableMediaPlayer)

public:
	PortableMediaPlayer(UDevDevice* device);
	~PortableMediaPlayer() override;

	QStringList supportedProtocols() const override;
	QStringList supportedDrivers(QString protocol = QString()) const override;
	QVariant driverHandle(const QString& driver) const override;

private:
	/**
     * Return full absolute path to media-player-info .mpi file, based on ID_MEDIA_PLAYER
     * udev property. Does not check for existence. Returns empty string in case no reasonable
     * file path could be determined.
     */
	QString mediaPlayerInfoFilePath() const;
};
}// namespace UDev
}// namespace Backends
}// namespace Solid

#endif// SOLID_BACKENDS_UDEV_PORTABLEMEDIAPLAYER_H
