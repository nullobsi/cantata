/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

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

#ifndef UDISKS2STORAGEDRIVE_H
#define UDISKS2STORAGEDRIVE_H

#include <ifaces/storagedrive.h>

#include "../shared/udevqt.h"

#include "udisksblock.h"

namespace Solid {
namespace Backends {
namespace UDisks2 {

class StorageDrive : public Block, virtual public Solid::Ifaces::StorageDrive {
	Q_OBJECT
	Q_INTERFACES(Solid::Ifaces::StorageDrive)

public:
	StorageDrive(Device* dev);
	~StorageDrive() override;

	qulonglong size() const override;
	bool isHotpluggable() const override;
	bool isRemovable() const override;
	Solid::StorageDrive::DriveType driveType() const override;
	Solid::StorageDrive::Bus bus() const override;

private:
	UdevQt::Device m_udevDevice;
};

}// namespace UDisks2
}// namespace Backends
}// namespace Solid

#endif// UDISK2SSTORAGEDRIVE_H
