/**
  * This file is part of the KDE project
  * Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
  * Copyright (C) 2007 John Tapsell <tapsell@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#ifndef KCATEGORIZEDSORTFILTERPROXYMODEL_P_H
#define KCATEGORIZEDSORTFILTERPROXYMODEL_P_H

#include <QtCore/QCollator>

#include "kcategorizedsortfilterproxymodel.h"

class KCategorizedSortFilterProxyModelPrivate {
public:
	KCategorizedSortFilterProxyModelPrivate()
		: sortColumn(0), sortOrder(Qt::AscendingOrder), categorizedModel(false), sortCategoriesByNaturalComparison(true)
	{
		m_collator.setNumericMode(true);
		m_collator.setCaseSensitivity(Qt::CaseSensitive);
	}

	~KCategorizedSortFilterProxyModelPrivate()
	{
	}

	int sortColumn;
	Qt::SortOrder sortOrder;
	bool categorizedModel;
	bool sortCategoriesByNaturalComparison;
	QCollator m_collator;
};

#endif
