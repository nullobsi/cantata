/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BACKENDS_HAL_OPTICALDISC_H
#define SOLID_BACKENDS_HAL_OPTICALDISC_H

#include "halvolume.h"
#include <solid-lite/ifaces/opticaldisc.h>

namespace Solid {
namespace Backends {
namespace Hal {
class OpticalDisc : public Volume, virtual public Solid::Ifaces::OpticalDisc {
	Q_OBJECT
	Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
	OpticalDisc(HalDevice* device);
	~OpticalDisc() override;

	Solid::OpticalDisc::ContentTypes availableContent() const override;
	Solid::OpticalDisc::DiscType discType() const override;
	bool isAppendable() const override;
	bool isBlank() const override;
	bool isRewritable() const override;
	qulonglong capacity() const override;
};
}// namespace Hal
}// namespace Backends
}// namespace Solid

#endif// SOLID_BACKENDS_HAL_OPTICALDISC_H
