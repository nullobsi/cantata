/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCATEGORIZEDSORTFILTERPROXYMODEL_P_H
#define KCATEGORIZEDSORTFILTERPROXYMODEL_P_H

#include <QCollator>

#include "kcategorizedsortfilterproxymodel.h"

class KCategorizedSortFilterProxyModelPrivate
{
public:
    KCategorizedSortFilterProxyModelPrivate()
        : sortColumn(0)
        , sortOrder(Qt::AscendingOrder)
        , categorizedModel(false)
        , sortCategoriesByNaturalComparison(true)
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
