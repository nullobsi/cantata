/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>

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

#ifndef UDISKSBLOCK_H
#define UDISKSBLOCK_H

#include "udisksdeviceinterface.h"
#include <solid-lite/ifaces/block.h>

namespace Solid {
namespace Backends {
namespace UDisks {

class Block : public DeviceInterface, virtual public Solid::Ifaces::Block {

	Q_OBJECT
	Q_INTERFACES(Solid::Ifaces::Block)

public:
	Block(UDisksDevice* device);
	virtual ~Block();

	virtual QString device() const;
	virtual int deviceMinor() const;
	virtual int deviceMajor() const;
};

}// namespace UDisks
}// namespace Backends
}// namespace Solid

#endif// UDISKSBLOCK_H
