/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
 *
 */
/*
 * Copyright (c) 2008 Sander Knopper (sander AT knopper DOT tk) and
 *                    Roeland Douma (roeland AT rullzer DOT com)
 *
 * This file is part of QtMPC.
 *
 * QtMPC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * QtMPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QtMPC.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OUTPUT_H
#define OUTPUT_H

#include "support/utils.h"
#include <QString>

struct Output {
	Output(quint32 i, bool en, QString n) : id(i), enabled(en), name(n) {}
	Output() : id(0xFFFFFFFF), enabled(false) {}

	Output(const Output& o) { *this = o; }

	Output& operator=(const Output& o)
	{
		id = o.id;
		enabled = o.enabled;
		inCurrentPartition = o.inCurrentPartition;
		name = o.name;
		return *this;
	}

	bool operator<(const Output& o) const
	{
		return Utils::compare(name, o.name) < 0;
	}

	virtual ~Output() {}

	quint32 id;
	bool enabled;
	bool inCurrentPartition{true};
	QString name;
};

#endif
