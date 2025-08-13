/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2007, 2009 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcategorydrawer.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOption>

#include <kcategorizedsortfilterproxymodel.h>
#include <kcategorizedview.h>

#include <cmath>

class KCategoryDrawerPrivate
{
public:
    KCategoryDrawerPrivate(KCategorizedView *view)
        : view(view)
    {
    }

    ~KCategoryDrawerPrivate()
    {
    }

    KCategorizedView *const view;
};

KCategoryDrawer::KCategoryDrawer(KCategorizedView *view)
    : QObject(view)
    , d(new KCategoryDrawerPrivate(view))
{
}

KCategoryDrawer::~KCategoryDrawer() = default;

void KCategoryDrawer::drawCategory(const QModelIndex &index, int /*sortRole*/, const QStyleOption &option, QPainter *painter) const
{
    // Keep this in sync with Kirigami.ListSectionHeader
    painter->setRenderHint(QPainter::Antialiasing);

    const QString category = index.model()->data(index, KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
    QFont font(QApplication::font());
    font.setBold(true);
    const QFontMetrics fontMetrics = QFontMetrics(font);

    const int topPadding = 8 + 4; // Kirigami.Units.largeSpacing + smallSpacing
    const int sidePadding = 8; // Kirigami.Units.largeSpacing

    // BEGIN: text
    {
        QRect textRect(option.rect);
        textRect.setTop(textRect.top() + topPadding);
        textRect.setLeft(textRect.left() + sidePadding);
        textRect.setRight(textRect.right() - sidePadding);
        textRect.setHeight(fontMetrics.height());

        painter->save();
        painter->setFont(font);
        QColor penColor(option.palette.text().color());
        penColor.setAlphaF(0.7);
        painter->setPen(penColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, category);
        painter->restore();
    }
    // END: text

    // BEGIN: horizontal line
    {
        QColor backgroundColor = option.palette.text().color();
        backgroundColor.setAlphaF(0.7 * 0.15); // replicate Kirigami.Separator color
        QRect backgroundRect(option.rect);
        backgroundRect.setLeft(fontMetrics.horizontalAdvance(category) + sidePadding * 2);
        backgroundRect.setRight(backgroundRect.right() - sidePadding);
        backgroundRect.setTop(backgroundRect.top() + topPadding + ceil(fontMetrics.height() / 2));
        backgroundRect.setHeight(1);
        painter->save();
        painter->setBrush(backgroundColor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(backgroundRect);
        painter->restore();
    }
    // END: horizontal line
}

int KCategoryDrawer::categoryHeight(const QModelIndex &index, const QStyleOption &option) const
{
    Q_UNUSED(index);
    Q_UNUSED(option)

    QFont font(QApplication::font());
    QFontMetrics fontMetrics(font);

    const int height = fontMetrics.height() + 8 + 8; // Kirigami.Units.largeSpacing + smallSpacing * 2
    return height;
}

int KCategoryDrawer::leftMargin() const
{
    return 0;
}

int KCategoryDrawer::rightMargin() const
{
    return 0;
}

KCategorizedView *KCategoryDrawer::view() const
{
    return d->view;
}

void KCategoryDrawer::mouseButtonPressed(const QModelIndex &, const QRect &, QMouseEvent *event)
{
    event->ignore();
}

void KCategoryDrawer::mouseButtonReleased(const QModelIndex &, const QRect &, QMouseEvent *event)
{
    event->ignore();
}

void KCategoryDrawer::mouseMoved(const QModelIndex &, const QRect &, QMouseEvent *event)
{
    event->ignore();
}

void KCategoryDrawer::mouseButtonDoubleClicked(const QModelIndex &, const QRect &, QMouseEvent *event)
{
    event->ignore();
}

void KCategoryDrawer::mouseLeft(const QModelIndex &, const QRect &)
{
}

#include "moc_kcategorydrawer.cpp"
