/*
    Copyright 2012 Lukáš Tinkl <ltinkl@redhat.com>

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

#ifndef UDISKS2BLOCK_H
#define UDISKS2BLOCK_H

#include "udisksdeviceinterface.h"
#include <solid-lite/ifaces/block.h>

#include <sys/types.h>// dev_t

namespace Solid {
namespace Backends {
namespace UDisks2 {

class Block : public DeviceInterface, virtual public Solid::Ifaces::Block {

	Q_OBJECT
	Q_INTERFACES(Solid::Ifaces::Block)

public:
	Block(Device* dev);
	~Block() override;

	QString device() const override;
	int deviceMinor() const override;
	int deviceMajor() const override;

private:
	dev_t m_devNum;
	QString m_devFile;
};

}// namespace UDisks2
}// namespace Backends
}// namespace Solid

#endif// UDISKS2BLOCK_H
