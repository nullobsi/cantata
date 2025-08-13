/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2009 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCATEGORYDRAWER_H
#define KCATEGORYDRAWER_H

// #include <kitemviews_export.h>

#include <QMouseEvent>
#include <QObject>
#include <memory>

class KCategoryDrawerPrivate;

class QPainter;
class QModelIndex;
class QStyleOption;

class KCategorizedView;

/*!
 * \class KCategoryDrawer
 * \inmodule KItemViews
 *
 * \brief The category drawing is performed by this class.
 *
 * It also gives information about the category height and margins.
 */
class /*KITEMVIEWS_EXPORT*/ KCategoryDrawer : public QObject
{
    Q_OBJECT
    friend class KCategorizedView;

public:
    /*!
     * Construct a category drawer for a given view
     *
     * \since 5.0
     */
    KCategoryDrawer(KCategorizedView *view);
    ~KCategoryDrawer() override;

    /*!
     * Returns the view this category drawer is associated with.
     */
    KCategorizedView *view() const;

    /*!
     * This method purpose is to draw a category represented by the given
     * \a index with the given \a sortRole sorting role
     *
     * \note This method will be called one time per category, always with the
     *       first element in that category
     */
    virtual void drawCategory(const QModelIndex &index, int sortRole, const QStyleOption &option, QPainter *painter) const;

    /*!
     * Returns the category height for the category represented by index \a index with
     *         style options \a option.
     */
    virtual int categoryHeight(const QModelIndex &index, const QStyleOption &option) const;

    /*!
     * \note 0 by default
     *
     * \since 4.4
     */
    virtual int leftMargin() const;

    /*!
     * \note 0 by default
     *
     * \since 4.4
     */
    virtual int rightMargin() const;

Q_SIGNALS:
    /*!
     * This signal becomes emitted when collapse or expand has been clicked.
     */
    void collapseOrExpandClicked(const QModelIndex &index);

    /*!
     * Emit this signal on your subclass implementation to notify that something happened. Usually
     * this will be triggered when you have received an event, and its position matched some "hot spot".
     *
     * You give this action the integer you want, and having connected this signal to your code,
     * the connected slot can perform the needed changes (view, model, selection model, delegate...)
     */
    void actionRequested(int action, const QModelIndex &index);

protected:
    /*!
     * Method called when the mouse button has been pressed.
     *
     * \a index The representative index of the block of items.
     *
     * \a blockRect The rect occupied by the block of items.
     *
     * \a event The mouse event.
     *
     * \warning You explicitly have to determine whether the event has been accepted or not. You
     *          have to call event->accept() or event->ignore() at all possible case branches in
     *          your code.
     */
    virtual void mouseButtonPressed(const QModelIndex &index, const QRect &blockRect, QMouseEvent *event);

    /*!
     * Method called when the mouse button has been released.
     *
     * \a index The representative index of the block of items.
     *
     * \a blockRect The rect occupied by the block of items.
     *
     * \a event The mouse event.
     *
     * \warning You explicitly have to determine whether the event has been accepted or not. You
     *          have to call event->accept() or event->ignore() at all possible case branches in
     *          your code.
     */
    virtual void mouseButtonReleased(const QModelIndex &index, const QRect &blockRect, QMouseEvent *event);

    /*!
     * Method called when the mouse has been moved.
     *
     * \a index The representative index of the block of items.
     *
     * \a blockRect The rect occupied by the block of items.
     *
     * \a event The mouse event.
     */
    virtual void mouseMoved(const QModelIndex &index, const QRect &blockRect, QMouseEvent *event);

    /*!
     * Method called when the mouse button has been double clicked.
     *
     * \a index The representative index of the block of items.
     *
     * \a blockRect The rect occupied by the block of items.
     *
     * \a event The mouse event.
     *
     * \warning You explicitly have to determine whether the event has been accepted or not. You
     *          have to call event->accept() or event->ignore() at all possible case branches in
     *          your code.
     */
    virtual void mouseButtonDoubleClicked(const QModelIndex &index, const QRect &blockRect, QMouseEvent *event);

    /*!
     * Method called when the mouse button has left this block.
     *
     * \a index The representative index of the block of items.
     *
     * \a blockRect The rect occupied by the block of items.
     */
    virtual void mouseLeft(const QModelIndex &index, const QRect &blockRect);

private:
    std::unique_ptr<KCategoryDrawerPrivate> const d;
};

#endif // KCATEGORYDRAWER_H
