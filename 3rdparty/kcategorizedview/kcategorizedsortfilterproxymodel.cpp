/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcategorizedsortfilterproxymodel.h"
#include "kcategorizedsortfilterproxymodel_p.h"

#include <QCollator>

KCategorizedSortFilterProxyModel::KCategorizedSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new KCategorizedSortFilterProxyModelPrivate())

{
}

KCategorizedSortFilterProxyModel::~KCategorizedSortFilterProxyModel() = default;

void KCategorizedSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    d->sortColumn = column;
    d->sortOrder = order;

    QSortFilterProxyModel::sort(column, order);
}

bool KCategorizedSortFilterProxyModel::isCategorizedModel() const
{
    return d->categorizedModel;
}

void KCategorizedSortFilterProxyModel::setCategorizedModel(bool categorizedModel)
{
    if (categorizedModel == d->categorizedModel) {
        return;
    }

    d->categorizedModel = categorizedModel;

    invalidate();
}

int KCategorizedSortFilterProxyModel::sortColumn() const
{
    return d->sortColumn;
}

Qt::SortOrder KCategorizedSortFilterProxyModel::sortOrder() const
{
    return d->sortOrder;
}

void KCategorizedSortFilterProxyModel::setSortCategoriesByNaturalComparison(bool sortCategoriesByNaturalComparison)
{
    if (sortCategoriesByNaturalComparison == d->sortCategoriesByNaturalComparison) {
        return;
    }

    d->sortCategoriesByNaturalComparison = sortCategoriesByNaturalComparison;

    invalidate();
}

bool KCategorizedSortFilterProxyModel::sortCategoriesByNaturalComparison() const
{
    return d->sortCategoriesByNaturalComparison;
}

bool KCategorizedSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (d->categorizedModel) {
        int compare = compareCategories(left, right);

        if (compare > 0) { // left is greater than right
            return false;
        } else if (compare < 0) { // left is less than right
            return true;
        }
    }

    return subSortLessThan(left, right);
}

bool KCategorizedSortFilterProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return QSortFilterProxyModel::lessThan(left, right);
}

int KCategorizedSortFilterProxyModel::compareCategories(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant l = (left.model() ? left.model()->data(left, CategorySortRole) : QVariant());
    QVariant r = (right.model() ? right.model()->data(right, CategorySortRole) : QVariant());

    Q_ASSERT(l.isValid());
    Q_ASSERT(r.isValid());
    Q_ASSERT(l.userType() == r.userType());

    if (l.userType() == QMetaType::QString) {
        QString lstr = l.toString();
        QString rstr = r.toString();

        if (d->sortCategoriesByNaturalComparison) {
            return d->m_collator.compare(lstr, rstr);
        } else {
            if (lstr < rstr) {
                return -1;
            }

            if (lstr > rstr) {
                return 1;
            }

            return 0;
        }
    }

    qlonglong lint = l.toLongLong();
    qlonglong rint = r.toLongLong();

    if (lint < rint) {
        return -1;
    }

    if (lint > rint) {
        return 1;
    }

    return 0;
}

#include "moc_kcategorizedsortfilterproxymodel.cpp"
