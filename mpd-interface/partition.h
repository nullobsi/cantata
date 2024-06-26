/*
 * Cantata
 *
 * Copyright (c) 2021 David Hoyes <dphoyes@gmail.com>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PARTITION_H
#define PARTITION_H

#include "support/utils.h"
#include <QString>

struct Partition {
	Partition() = default;
	Partition(QString n) : name(n) {}
	~Partition() = default;

	bool operator<(const Partition& o) const
	{
		return Utils::compare(name, o.name) < 0;
	}

	QString name;
};

#endif
