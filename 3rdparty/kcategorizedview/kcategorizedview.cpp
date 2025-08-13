/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2009 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
 * IMPLEMENTATION NOTES:
 *
 * QListView::setRowHidden() and QListView::isRowHidden() are not taken into
 * account. This methods should actually not exist. This effect should be handled
 * by an hypothetical QSortFilterProxyModel which filters out the desired rows.
 *
 * In case this needs to be implemented, contact me, but I consider this a faulty
 * design.
 */

#include "kcategorizedview.h"
#include "kcategorizedview_p.h"

#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>

//#include <kitemviews_debug.h>

#include "kcategorizedsortfilterproxymodel.h"
#include "kcategorydrawer.h"

// BEGIN: Private part

struct KCategorizedViewPrivate::Item {
    Item()
        : topLeft(QPoint())
        , size(QSize())
    {
    }

    QPoint topLeft;
    QSize size;
};

struct KCategorizedViewPrivate::Block {
    Block()
        : topLeft(QPoint())
        , firstIndex(QModelIndex())
        , quarantineStart(QModelIndex())
        , items(QList<Item>())
    {
    }

    bool operator!=(const Block &rhs) const
    {
        return firstIndex != rhs.firstIndex;
    }

    static bool lessThan(const Block &left, const Block &right)
    {
        Q_ASSERT(left.firstIndex.isValid());
        Q_ASSERT(right.firstIndex.isValid());
        return left.firstIndex.row() < right.firstIndex.row();
    }

    QPoint topLeft;
    int height = -1;
    QPersistentModelIndex firstIndex;
    // if we have n elements on this block, and we inserted an element at position i. The quarantine
    // will start at index (i, column, parent). This means that for all elements j where i <= j <= n, the
    // visual rect position of item j will have to be recomputed (cannot use the cached point). The quarantine
    // will only affect the current block, since the rest of blocks can be affected only in the way
    // that the whole block will have different offset, but items will keep the same relative position
    // in terms of their parent blocks.
    QPersistentModelIndex quarantineStart;
    QList<Item> items;

    // this affects the whole block, not items separately. items contain the topLeft point relative
    // to the block. Because of insertions or removals a whole block can be moved, so the whole block
    // will enter in quarantine, what is faster than moving all items in absolute terms.
    bool outOfQuarantine = false;

    // should we alternate its color ? is just a hint, could not be used
    bool alternate = false;
    bool collapsed = false;
};

KCategorizedViewPrivate::KCategorizedViewPrivate(KCategorizedView *qq)
    : q(qq)
    , hoveredBlock(new Block())
    , hoveredIndex(QModelIndex())
    , pressedPosition(QPoint())
    , rubberBandRect(QRect())
{
}

KCategorizedViewPrivate::~KCategorizedViewPrivate()
{
    delete hoveredBlock;
}

bool KCategorizedViewPrivate::isCategorized() const
{
    return proxyModel && categoryDrawer && proxyModel->isCategorizedModel();
}

QStyleOptionViewItem KCategorizedViewPrivate::viewOpts()
{
    QStyleOptionViewItem option;
    q->initViewItemOption(&option);
    return option;
}

QStyleOptionViewItem KCategorizedViewPrivate::blockRect(const QModelIndex &representative)
{
    QStyleOptionViewItem option = viewOpts();

    const int height = categoryDrawer->categoryHeight(representative, option);
    const QString categoryDisplay = representative.data(KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
    QPoint pos = blockPosition(categoryDisplay);
    pos.ry() -= height;
    option.rect.setTopLeft(pos);
    option.rect.setWidth(viewportWidth() + categoryDrawer->leftMargin() + categoryDrawer->rightMargin());
    option.rect.setHeight(height + blockHeight(categoryDisplay));
    option.rect = mapToViewport(option.rect);

    return option;
}

std::pair<QModelIndex, QModelIndex> KCategorizedViewPrivate::intersectingIndexesWithRect(const QRect &_rect) const
{
    const int rowCount = proxyModel->rowCount();

    const QRect rect = _rect.normalized();

    // binary search to find out the top border
    int bottom = 0;
    int top = rowCount - 1;
    while (bottom <= top) {
        const int middle = (bottom + top) / 2;
        const QModelIndex index = proxyModel->index(middle, q->modelColumn(), q->rootIndex());
        const QRect itemRect = q->visualRect(index);
        if (itemRect.bottomRight().y() <= rect.topLeft().y()) {
            bottom = middle + 1;
        } else {
            top = middle - 1;
        }
    }

    const QModelIndex bottomIndex = proxyModel->index(bottom, q->modelColumn(), q->rootIndex());

    // binary search to find out the bottom border
    bottom = 0;
    top = rowCount - 1;
    while (bottom <= top) {
        const int middle = (bottom + top) / 2;
        const QModelIndex index = proxyModel->index(middle, q->modelColumn(), q->rootIndex());
        const QRect itemRect = q->visualRect(index);
        if (itemRect.topLeft().y() <= rect.bottomRight().y()) {
            bottom = middle + 1;
        } else {
            top = middle - 1;
        }
    }

    const QModelIndex topIndex = proxyModel->index(top, q->modelColumn(), q->rootIndex());

    return {bottomIndex, topIndex};
}

QPoint KCategorizedViewPrivate::blockPosition(const QString &category)
{
    Block &block = blocks[category];

    if (block.outOfQuarantine && !block.topLeft.isNull()) {
        return block.topLeft;
    }

    QPoint res(categorySpacing, 0);

    const QModelIndex index = block.firstIndex;

    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        Block &block = *it;
        const QModelIndex categoryIndex = block.firstIndex;
        if (index.row() < categoryIndex.row()) {
            continue;
        }

        res.ry() += categoryDrawer->categoryHeight(categoryIndex, viewOpts()) + categorySpacing;
        if (index.row() == categoryIndex.row()) {
            continue;
        }
        res.ry() += blockHeight(it.key());
    }

    block.outOfQuarantine = true;
    block.topLeft = res;

    return res;
}

int KCategorizedViewPrivate::blockHeight(const QString &category)
{
    Block &block = blocks[category];

    if (block.collapsed) {
        return 0;
    }

    if (block.height > -1) {
        return block.height;
    }

    const QModelIndex firstIndex = block.firstIndex;
    const QModelIndex lastIndex = proxyModel->index(firstIndex.row() + block.items.count() - 1, q->modelColumn(), q->rootIndex());
    const QRect topLeft = q->visualRect(firstIndex);
    QRect bottomRight = q->visualRect(lastIndex);

    if (hasGrid()) {
        bottomRight.setHeight(qMax(bottomRight.height(), q->gridSize().height()));
    } else {
        if (!q->uniformItemSizes()) {
            bottomRight.setHeight(highestElementInLastRow(block) + q->spacing() * 2);
        }
    }

    const int height = bottomRight.bottomRight().y() - topLeft.topLeft().y() + 1;
    block.height = height;

    return height;
}

int KCategorizedViewPrivate::viewportWidth() const
{
    return q->viewport()->width() - categorySpacing * 2 - categoryDrawer->leftMargin() - categoryDrawer->rightMargin();
}

void KCategorizedViewPrivate::regenerateAllElements()
{
    for (QHash<QString, Block>::Iterator it = blocks.begin(); it != blocks.end(); ++it) {
        Block &block = *it;
        block.outOfQuarantine = false;
        block.quarantineStart = block.firstIndex;
        block.height = -1;
    }
}

void KCategorizedViewPrivate::rowsInserted(const QModelIndex &parent, int start, int end)
{
    if (!isCategorized()) {
        return;
    }

    for (int i = start; i <= end; ++i) {
        const QModelIndex index = proxyModel->index(i, q->modelColumn(), parent);

        Q_ASSERT(index.isValid());

        const QString category = categoryForIndex(index);

        Block &block = blocks[category];

        // BEGIN: update firstIndex
        // save as firstIndex in block if
        //     - it forced the category creation (first element on this category)
        //     - it is before the first row on that category
        const QModelIndex firstIndex = block.firstIndex;
        if (!firstIndex.isValid() || index.row() < firstIndex.row()) {
            block.firstIndex = index;
        }
        // END: update firstIndex

        Q_ASSERT(block.firstIndex.isValid());

        const int firstIndexRow = block.firstIndex.row();

        block.items.insert(index.row() - firstIndexRow, KCategorizedViewPrivate::Item());
        block.height = -1;

        q->visualRect(index);
        q->viewport()->update();
    }

    // BEGIN: update the items that are in quarantine in affected categories
    {
        const QModelIndex lastIndex = proxyModel->index(end, q->modelColumn(), parent);
        const QString category = categoryForIndex(lastIndex);
        KCategorizedViewPrivate::Block &block = blocks[category];
        block.quarantineStart = block.firstIndex;
    }
    // END: update the items that are in quarantine in affected categories

    // BEGIN: mark as in quarantine those categories that are under the affected ones
    {
        const QModelIndex firstIndex = proxyModel->index(start, q->modelColumn(), parent);
        const QString category = categoryForIndex(firstIndex);
        const QModelIndex firstAffectedCategory = blocks[category].firstIndex;
        // BEGIN: order for marking as alternate those blocks that are alternate
        QList<Block> blockList = blocks.values();
        std::sort(blockList.begin(), blockList.end(), Block::lessThan);
        QList<int> firstIndexesRows;
        for (const Block &block : std::as_const(blockList)) {
            firstIndexesRows << block.firstIndex.row();
        }
        // END: order for marking as alternate those blocks that are alternate
        for (auto it = blocks.begin(); it != blocks.end(); ++it) {
            KCategorizedViewPrivate::Block &block = *it;
            if (block.firstIndex.row() > firstAffectedCategory.row()) {
                block.outOfQuarantine = false;
                block.alternate = firstIndexesRows.indexOf(block.firstIndex.row()) % 2;
            } else if (block.firstIndex.row() == firstAffectedCategory.row()) {
                block.alternate = firstIndexesRows.indexOf(block.firstIndex.row()) % 2;
            }
        }
    }
    // END: mark as in quarantine those categories that are under the affected ones
}

QRect KCategorizedViewPrivate::mapToViewport(const QRect &rect) const
{
    const int dx = -q->horizontalOffset();
    const int dy = -q->verticalOffset();
    return rect.adjusted(dx, dy, dx, dy);
}

QRect KCategorizedViewPrivate::mapFromViewport(const QRect &rect) const
{
    const int dx = q->horizontalOffset();
    const int dy = q->verticalOffset();
    return rect.adjusted(dx, dy, dx, dy);
}

int KCategorizedViewPrivate::highestElementInLastRow(const Block &block) const
{
    // Find the highest element in the last row
    const QModelIndex lastIndex = proxyModel->index(block.firstIndex.row() + block.items.count() - 1, q->modelColumn(), q->rootIndex());
    const QRect prevRect = q->visualRect(lastIndex);
    int res = prevRect.height();
    QModelIndex prevIndex = proxyModel->index(lastIndex.row() - 1, q->modelColumn(), q->rootIndex());
    if (!prevIndex.isValid()) {
        return res;
    }
    Q_FOREVER {
        const QRect tempRect = q->visualRect(prevIndex);
        if (tempRect.topLeft().y() < prevRect.topLeft().y()) {
            break;
        }
        res = qMax(res, tempRect.height());
        if (prevIndex == block.firstIndex) {
            break;
        }
        prevIndex = proxyModel->index(prevIndex.row() - 1, q->modelColumn(), q->rootIndex());
    }

    return res;
}

bool KCategorizedViewPrivate::hasGrid() const
{
    const QSize gridSize = q->gridSize();
    return gridSize.isValid() && !gridSize.isNull();
}

QString KCategorizedViewPrivate::categoryForIndex(const QModelIndex &index) const
{
    const auto indexModel = index.model();
    if (!indexModel || !proxyModel) {
        //qCWarning(KITEMVIEWS_LOG) << "Index or view doesn't contain model";
        return QString();
    }

    const QModelIndex categoryIndex = indexModel->index(index.row(), proxyModel->sortColumn(), index.parent());
    return categoryIndex.data(KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
}

void KCategorizedViewPrivate::leftToRightVisualRect(const QModelIndex &index, Item &item, const Block &block, const QPoint &blockPos) const
{
    const int firstIndexRow = block.firstIndex.row();

    if (hasGrid()) {
        const int relativeRow = index.row() - firstIndexRow;
        const int maxItemsPerRow = qMax(viewportWidth() / q->gridSize().width(), 1);
        if (q->layoutDirection() == Qt::LeftToRight) {
            item.topLeft.rx() = (relativeRow % maxItemsPerRow) * q->gridSize().width() + blockPos.x() + categoryDrawer->leftMargin();
        } else {
            item.topLeft.rx() = viewportWidth() - ((relativeRow % maxItemsPerRow) + 1) * q->gridSize().width() + categoryDrawer->leftMargin() + categorySpacing;
        }
        item.topLeft.ry() = (relativeRow / maxItemsPerRow) * q->gridSize().height();
    } else {
        if (q->uniformItemSizes()) {
            const int relativeRow = index.row() - firstIndexRow;
            const QSize itemSize = q->sizeHintForIndex(index);
            const int maxItemsPerRow = qMax((viewportWidth() - q->spacing()) / (itemSize.width() + q->spacing()), 1);
            if (q->layoutDirection() == Qt::LeftToRight) {
                item.topLeft.rx() = (relativeRow % maxItemsPerRow) * itemSize.width() + blockPos.x() + categoryDrawer->leftMargin();
            } else {
                item.topLeft.rx() = viewportWidth() - (relativeRow % maxItemsPerRow) * itemSize.width() + categoryDrawer->leftMargin() + categorySpacing;
            }
            item.topLeft.ry() = (relativeRow / maxItemsPerRow) * itemSize.height();
        } else {
            const QSize currSize = q->sizeHintForIndex(index);
            if (index != block.firstIndex) {
                const int viewportW = viewportWidth() - q->spacing();
                QModelIndex prevIndex = proxyModel->index(index.row() - 1, q->modelColumn(), q->rootIndex());
                QRect prevRect = q->visualRect(prevIndex);
                prevRect = mapFromViewport(prevRect);
                if ((prevRect.bottomRight().x() + 1) + currSize.width() - blockPos.x() + q->spacing() > viewportW) {
                    // we have to check the whole previous row, and see which one was the
                    // highest.
                    Q_FOREVER {
                        prevIndex = proxyModel->index(prevIndex.row() - 1, q->modelColumn(), q->rootIndex());
                        const QRect tempRect = q->visualRect(prevIndex);
                        if (tempRect.topLeft().y() < prevRect.topLeft().y()) {
                            break;
                        }
                        if (tempRect.bottomRight().y() > prevRect.bottomRight().y()) {
                            prevRect = tempRect;
                        }
                        if (prevIndex == block.firstIndex) {
                            break;
                        }
                    }
                    if (q->layoutDirection() == Qt::LeftToRight) {
                        item.topLeft.rx() = categoryDrawer->leftMargin() + blockPos.x() + q->spacing();
                    } else {
                        item.topLeft.rx() = viewportWidth() - currSize.width() + categoryDrawer->leftMargin() + categorySpacing;
                    }
                    item.topLeft.ry() = (prevRect.bottomRight().y() + 1) + q->spacing() - blockPos.y();
                } else {
                    if (q->layoutDirection() == Qt::LeftToRight) {
                        item.topLeft.rx() = (prevRect.bottomRight().x() + 1) + q->spacing();
                    } else {
                        item.topLeft.rx() = (prevRect.bottomLeft().x() - 1) - q->spacing() - item.size.width() + categoryDrawer->leftMargin() + categorySpacing;
                    }
                    item.topLeft.ry() = prevRect.topLeft().y() - blockPos.y();
                }
            } else {
                if (q->layoutDirection() == Qt::LeftToRight) {
                    item.topLeft.rx() = blockPos.x() + categoryDrawer->leftMargin() + q->spacing();
                } else {
                    item.topLeft.rx() = viewportWidth() - currSize.width() + categoryDrawer->leftMargin() + categorySpacing;
                }
                item.topLeft.ry() = q->spacing();
            }
        }
    }
    item.size = q->sizeHintForIndex(index);
}

void KCategorizedViewPrivate::topToBottomVisualRect(const QModelIndex &index, Item &item, const Block &block, const QPoint &blockPos) const
{
    const int firstIndexRow = block.firstIndex.row();

    if (hasGrid()) {
        const int relativeRow = index.row() - firstIndexRow;
        item.topLeft.rx() = blockPos.x() + categoryDrawer->leftMargin();
        item.topLeft.ry() = relativeRow * q->gridSize().height();
    } else {
        if (q->uniformItemSizes()) {
            const int relativeRow = index.row() - firstIndexRow;
            const QSize itemSize = q->sizeHintForIndex(index);
            item.topLeft.rx() = blockPos.x() + categoryDrawer->leftMargin();
            item.topLeft.ry() = relativeRow * itemSize.height();
        } else {
            if (index != block.firstIndex) {
                QModelIndex prevIndex = proxyModel->index(index.row() - 1, q->modelColumn(), q->rootIndex());
                QRect prevRect = q->visualRect(prevIndex);
                prevRect = mapFromViewport(prevRect);
                item.topLeft.rx() = blockPos.x() + categoryDrawer->leftMargin() + q->spacing();
                item.topLeft.ry() = (prevRect.bottomRight().y() + 1) + q->spacing() - blockPos.y();
            } else {
                item.topLeft.rx() = blockPos.x() + categoryDrawer->leftMargin() + q->spacing();
                item.topLeft.ry() = q->spacing();
            }
        }
    }
    item.size = q->sizeHintForIndex(index);
    item.size.setWidth(viewportWidth());
}

void KCategorizedViewPrivate::_k_slotCollapseOrExpandClicked(QModelIndex)
{
}

// END: Private part

// BEGIN: Public part

KCategorizedView::KCategorizedView(QWidget *parent)
    : QListView(parent)
    , d(new KCategorizedViewPrivate(this))
{
}

KCategorizedView::~KCategorizedView() = default;

void KCategorizedView::setModel(QAbstractItemModel *model)
{
    if (d->proxyModel == model) {
        return;
    }

    d->blocks.clear();

    if (d->proxyModel) {
        disconnect(d->proxyModel, SIGNAL(layoutChanged()), this, SLOT(slotLayoutChanged()));
    }

    d->proxyModel = dynamic_cast<KCategorizedSortFilterProxyModel *>(model);

    if (d->proxyModel) {
        connect(d->proxyModel, SIGNAL(layoutChanged()), this, SLOT(slotLayoutChanged()));
    }

    QListView::setModel(model);

    // if the model already had information inserted, update our data structures to it
    if (model && model->rowCount()) {
        slotLayoutChanged();
    }
}

void KCategorizedView::setGridSize(const QSize &size)
{
    setGridSizeOwn(size);
}

void KCategorizedView::setGridSizeOwn(const QSize &size)
{
    d->regenerateAllElements();
    QListView::setGridSize(size);
}

QRect KCategorizedView::visualRect(const QModelIndex &index) const
{
    if (!d->isCategorized()) {
        return QListView::visualRect(index);
    }

    if (!index.isValid()) {
        return QRect();
    }

    const QString category = d->categoryForIndex(index);

    if (!d->blocks.contains(category)) {
        return QRect();
    }

    KCategorizedViewPrivate::Block &block = d->blocks[category];
    const int firstIndexRow = block.firstIndex.row();

    Q_ASSERT(block.firstIndex.isValid());

    if (index.row() - firstIndexRow < 0 || index.row() - firstIndexRow >= block.items.count()) {
        return QRect();
    }

    const QPoint blockPos = d->blockPosition(category);

    KCategorizedViewPrivate::Item &ritem = block.items[index.row() - firstIndexRow];

    if (ritem.topLeft.isNull() //
        || (block.quarantineStart.isValid() && index.row() >= block.quarantineStart.row())) {
        if (flow() == LeftToRight) {
            d->leftToRightVisualRect(index, ritem, block, blockPos);
        } else {
            d->topToBottomVisualRect(index, ritem, block, blockPos);
        }

        // BEGIN: update the quarantine start
        const bool wasLastIndex = (index.row() == (block.firstIndex.row() + block.items.count() - 1));
        if (index.row() == block.quarantineStart.row()) {
            if (wasLastIndex) {
                block.quarantineStart = QModelIndex();
            } else {
                const QModelIndex nextIndex = d->proxyModel->index(index.row() + 1, modelColumn(), rootIndex());
                block.quarantineStart = nextIndex;
            }
        }
        // END: update the quarantine start
    }

    // we get now the absolute position through the relative position of the parent block. do not
    // save this on ritem, since this would override the item relative position in block terms.
    KCategorizedViewPrivate::Item item(ritem);
    item.topLeft.ry() += blockPos.y();

    const QSize sizeHint = item.size;

    if (d->hasGrid()) {
        const QSize sizeGrid = gridSize();
        const QSize resultingSize = sizeHint.boundedTo(sizeGrid);
        QRect res(item.topLeft.x() + ((sizeGrid.width() - resultingSize.width()) / 2), item.topLeft.y(), resultingSize.width(), resultingSize.height());
        if (block.collapsed) {
            // we can still do binary search, while we "hide" items. We move those items in collapsed
            // blocks to the left and set a 0 height.
            res.setLeft(-resultingSize.width());
            res.setHeight(0);
        }
        return d->mapToViewport(res);
    }

    QRect res(item.topLeft.x(), item.topLeft.y(), sizeHint.width(), sizeHint.height());
    if (block.collapsed) {
        // we can still do binary search, while we "hide" items. We move those items in collapsed
        // blocks to the left and set a 0 height.
        res.setLeft(-sizeHint.width());
        res.setHeight(0);
    }
    return d->mapToViewport(res);
}

KCategoryDrawer *KCategorizedView::categoryDrawer() const
{
    return d->categoryDrawer;
}

void KCategorizedView::setCategoryDrawer(KCategoryDrawer *categoryDrawer)
{
    if (d->categoryDrawer) {
        disconnect(d->categoryDrawer, SIGNAL(collapseOrExpandClicked(QModelIndex)), this, SLOT(_k_slotCollapseOrExpandClicked(QModelIndex)));
    }

    d->categoryDrawer = categoryDrawer;

    connect(d->categoryDrawer, SIGNAL(collapseOrExpandClicked(QModelIndex)), this, SLOT(_k_slotCollapseOrExpandClicked(QModelIndex)));
}

int KCategorizedView::categorySpacing() const
{
    return d->categorySpacing;
}

void KCategorizedView::setCategorySpacing(int categorySpacing)
{
    if (d->categorySpacing == categorySpacing) {
        return;
    }

    d->categorySpacing = categorySpacing;

    for (auto it = d->blocks.begin(); it != d->blocks.end(); ++it) {
        KCategorizedViewPrivate::Block &block = *it;
        block.outOfQuarantine = false;
    }
    Q_EMIT categorySpacingChanged(d->categorySpacing);
}

bool KCategorizedView::alternatingBlockColors() const
{
    return d->alternatingBlockColors;
}

void KCategorizedView::setAlternatingBlockColors(bool enable)
{
    if (d->alternatingBlockColors == enable) {
        return;
    }

    d->alternatingBlockColors = enable;
    Q_EMIT alternatingBlockColorsChanged(d->alternatingBlockColors);
}

bool KCategorizedView::collapsibleBlocks() const
{
    return d->collapsibleBlocks;
}

void KCategorizedView::setCollapsibleBlocks(bool enable)
{
    if (d->collapsibleBlocks == enable) {
        return;
    }

    d->collapsibleBlocks = enable;
    Q_EMIT collapsibleBlocksChanged(d->collapsibleBlocks);
}

QModelIndexList KCategorizedView::block(const QString &category)
{
    QModelIndexList res;
    const KCategorizedViewPrivate::Block &block = d->blocks[category];
    if (block.height == -1) {
        return res;
    }
    QModelIndex current = block.firstIndex;
    const int first = current.row();
    for (int i = 1; i <= block.items.count(); ++i) {
        if (current.isValid()) {
            res << current;
        }
        current = d->proxyModel->index(first + i, modelColumn(), rootIndex());
    }
    return res;
}

QModelIndexList KCategorizedView::block(const QModelIndex &representative)
{
    return block(representative.data(KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString());
}

QModelIndex KCategorizedView::indexAt(const QPoint &point) const
{
    if (!d->isCategorized()) {
        return QListView::indexAt(point);
    }

    const int rowCount = d->proxyModel->rowCount();
    if (!rowCount) {
        return QModelIndex();
    }

    // Binary search that will try to spot if there is an index under point
    int bottom = 0;
    int top = rowCount - 1;
    while (bottom <= top) {
        const int middle = (bottom + top) / 2;
        const QModelIndex index = d->proxyModel->index(middle, modelColumn(), rootIndex());
        const QRect rect = visualRect(index);
        if (rect.contains(point)) {
            if (index.model()->flags(index) & Qt::ItemIsEnabled) {
                return index;
            }
            return QModelIndex();
        }
        bool directionCondition;
        if (layoutDirection() == Qt::LeftToRight) {
            directionCondition = point.x() >= rect.bottomLeft().x();
        } else {
            directionCondition = point.x() <= rect.bottomRight().x();
        }
        if (point.y() < rect.topLeft().y()) {
            top = middle - 1;
        } else if (directionCondition) {
            bottom = middle + 1;
        } else if (point.y() <= rect.bottomRight().y()) {
            top = middle - 1;
        } else {
            bool after = true;
            for (int i = middle - 1; i >= bottom; i--) {
                const QModelIndex newIndex = d->proxyModel->index(i, modelColumn(), rootIndex());
                const QRect newRect = visualRect(newIndex);
                if (newRect.topLeft().y() < rect.topLeft().y()) {
                    break;
                } else if (newRect.contains(point)) {
                    if (newIndex.model()->flags(newIndex) & Qt::ItemIsEnabled) {
                        return newIndex;
                    }
                    return QModelIndex();
                    // clang-format off
                } else if ((layoutDirection() == Qt::LeftToRight) ?
                           (newRect.topLeft().x() <= point.x()) :
                           (newRect.topRight().x() >= point.x())) {
                    // clang-format on
                    break;
                } else if (newRect.bottomRight().y() >= point.y()) {
                    after = false;
                }
            }
            if (!after) {
                return QModelIndex();
            }
            bottom = middle + 1;
        }
    }
    return QModelIndex();
}

void KCategorizedView::reset()
{
    d->blocks.clear();
    QListView::reset();
}

void KCategorizedView::paintEvent(QPaintEvent *event)
{
    if (!d->isCategorized()) {
        QListView::paintEvent(event);
        return;
    }

    const std::pair<QModelIndex, QModelIndex> intersecting = d->intersectingIndexesWithRect(viewport()->rect().intersected(event->rect()));

    QPainter p(viewport());
    p.save();

    Q_ASSERT(selectionModel()->model() == d->proxyModel);

    // BEGIN: draw categories
    auto it = d->blocks.constBegin();
    while (it != d->blocks.constEnd()) {
        const KCategorizedViewPrivate::Block &block = *it;
        const QModelIndex categoryIndex = d->proxyModel->index(block.firstIndex.row(), d->proxyModel->sortColumn(), rootIndex());

        QStyleOptionViewItem option = d->viewOpts();
        option.features |= d->alternatingBlockColors && block.alternate //
            ? QStyleOptionViewItem::Alternate
            : QStyleOptionViewItem::None;
        option.state |= !d->collapsibleBlocks || !block.collapsed //
            ? QStyle::State_Open
            : QStyle::State_None;
        const int height = d->categoryDrawer->categoryHeight(categoryIndex, option);
        QPoint pos = d->blockPosition(it.key());
        pos.ry() -= height;
        option.rect.setTopLeft(pos);
        option.rect.setWidth(d->viewportWidth() + d->categoryDrawer->leftMargin() + d->categoryDrawer->rightMargin());
        option.rect.setHeight(height + d->blockHeight(it.key()));
        option.rect = d->mapToViewport(option.rect);
        if (!option.rect.intersects(viewport()->rect())) {
            ++it;
            continue;
        }
        d->categoryDrawer->drawCategory(categoryIndex, d->proxyModel->sortRole(), option, &p);
        ++it;
    }
    // END: draw categories

    if (intersecting.first.isValid() && intersecting.second.isValid()) {
        // BEGIN: draw items
        int i = intersecting.first.row();
        int indexToCheckIfBlockCollapsed = i;
        QModelIndex categoryIndex;
        QString category;
        KCategorizedViewPrivate::Block *block = nullptr;
        while (i <= intersecting.second.row()) {
            // BEGIN: first check if the block is collapsed. if so, we have to skip the item painting
            if (i == indexToCheckIfBlockCollapsed) {
                categoryIndex = d->proxyModel->index(i, d->proxyModel->sortColumn(), rootIndex());
                category = categoryIndex.data(KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
                block = &d->blocks[category];
                indexToCheckIfBlockCollapsed = block->firstIndex.row() + block->items.count();
                if (block->collapsed) {
                    i = indexToCheckIfBlockCollapsed;
                    continue;
                }
            }
            // END: first check if the block is collapsed. if so, we have to skip the item painting

            Q_ASSERT(block);

            const bool alternateItem = (i - block->firstIndex.row()) % 2;

            const QModelIndex index = d->proxyModel->index(i, modelColumn(), rootIndex());
            const Qt::ItemFlags flags = d->proxyModel->flags(index);
            QStyleOptionViewItem option(d->viewOpts());
            option.rect = visualRect(index);
            option.widget = this;
            option.features |= wordWrap() ? QStyleOptionViewItem::WrapText : QStyleOptionViewItem::None;
            option.features |= alternatingRowColors() && alternateItem ? QStyleOptionViewItem::Alternate : QStyleOptionViewItem::None;
            if (flags & Qt::ItemIsSelectable) {
                option.state |= selectionModel()->isSelected(index) ? QStyle::State_Selected : QStyle::State_None;
            } else {
                option.state &= ~QStyle::State_Selected;
            }
            option.state |= (index == currentIndex()) ? QStyle::State_HasFocus : QStyle::State_None;
            if (!(flags & Qt::ItemIsEnabled)) {
                option.state &= ~QStyle::State_Enabled;
            } else {
                option.state |= (index == d->hoveredIndex) ? QStyle::State_MouseOver : QStyle::State_None;
            }

            itemDelegateForIndex(index)->paint(&p, option, index);
            ++i;
        }
        // END: draw items
    }

    // BEGIN: draw selection rect
    if (isSelectionRectVisible() && d->rubberBandRect.isValid()) {
        QStyleOptionRubberBand opt;
        opt.initFrom(this);
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = d->mapToViewport(d->rubberBandRect).intersected(viewport()->rect().adjusted(-16, -16, 16, 16));
        p.save();
        style()->drawControl(QStyle::CE_RubberBand, &opt, &p);
        p.restore();
    }
    // END: draw selection rect

    p.restore();
}

void KCategorizedView::resizeEvent(QResizeEvent *event)
{
    d->regenerateAllElements();
    QListView::resizeEvent(event);
}

void KCategorizedView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    if (!d->isCategorized()) {
        QListView::setSelection(rect, flags);
        return;
    }

    if (rect.topLeft() == rect.bottomRight()) {
        const QModelIndex index = indexAt(rect.topLeft());
        selectionModel()->select(index, flags);
        return;
    }

    const std::pair<QModelIndex, QModelIndex> intersecting = d->intersectingIndexesWithRect(rect);

    QItemSelection selection;

    // TODO: think of a faster implementation
    QModelIndex firstIndex;
    QModelIndex lastIndex;
    for (int i = intersecting.first.row(); i <= intersecting.second.row(); ++i) {
        const QModelIndex index = d->proxyModel->index(i, modelColumn(), rootIndex());
        const bool visualRectIntersects = visualRect(index).intersects(rect);
        if (firstIndex.isValid()) {
            if (visualRectIntersects) {
                lastIndex = index;
            } else {
                selection << QItemSelectionRange(firstIndex, lastIndex);
                firstIndex = QModelIndex();
            }
        } else if (visualRectIntersects) {
            firstIndex = index;
            lastIndex = index;
        }
    }

    if (firstIndex.isValid()) {
        selection << QItemSelectionRange(firstIndex, lastIndex);
    }

    selectionModel()->select(selection, flags);
}

void KCategorizedView::mouseMoveEvent(QMouseEvent *event)
{
    QListView::mouseMoveEvent(event);
    d->hoveredIndex = indexAt(event->pos());
    const SelectionMode itemViewSelectionMode = selectionMode();
    if (state() == DragSelectingState //
        && isSelectionRectVisible() //
        && itemViewSelectionMode != SingleSelection //
        && itemViewSelectionMode != NoSelection) {
        QRect rect(d->pressedPosition, event->pos() + QPoint(horizontalOffset(), verticalOffset()));
        rect = rect.normalized();
        update(rect.united(d->rubberBandRect));
        d->rubberBandRect = rect;
    }
    if (!d->categoryDrawer) {
        return;
    }
    auto it = d->blocks.constBegin();
    while (it != d->blocks.constEnd()) {
        const KCategorizedViewPrivate::Block &block = *it;
        const QModelIndex categoryIndex = d->proxyModel->index(block.firstIndex.row(), d->proxyModel->sortColumn(), rootIndex());
        QStyleOptionViewItem option(d->viewOpts());
        const int height = d->categoryDrawer->categoryHeight(categoryIndex, option);
        QPoint pos = d->blockPosition(it.key());
        pos.ry() -= height;
        option.rect.setTopLeft(pos);
        option.rect.setWidth(d->viewportWidth() + d->categoryDrawer->leftMargin() + d->categoryDrawer->rightMargin());
        option.rect.setHeight(height + d->blockHeight(it.key()));
        option.rect = d->mapToViewport(option.rect);
        const QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
        if (option.rect.contains(mousePos)) {
            if (d->hoveredBlock->height != -1 && *d->hoveredBlock != block) {
                const QModelIndex categoryIndex = d->proxyModel->index(d->hoveredBlock->firstIndex.row(), d->proxyModel->sortColumn(), rootIndex());
                const QStyleOptionViewItem option = d->blockRect(categoryIndex);
                d->categoryDrawer->mouseLeft(categoryIndex, option.rect);
                *d->hoveredBlock = block;
                d->hoveredCategory = it.key();
                viewport()->update(option.rect);
            } else if (d->hoveredBlock->height == -1) {
                *d->hoveredBlock = block;
                d->hoveredCategory = it.key();
            } else {
                d->categoryDrawer->mouseMoved(categoryIndex, option.rect, event);
            }
            viewport()->update(option.rect);
            return;
        }
        ++it;
    }
    if (d->hoveredBlock->height != -1) {
        const QModelIndex categoryIndex = d->proxyModel->index(d->hoveredBlock->firstIndex.row(), d->proxyModel->sortColumn(), rootIndex());
        const QStyleOptionViewItem option = d->blockRect(categoryIndex);
        d->categoryDrawer->mouseLeft(categoryIndex, option.rect);
        *d->hoveredBlock = KCategorizedViewPrivate::Block();
        d->hoveredCategory = QString();
        viewport()->update(option.rect);
    }
}

void KCategorizedView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        d->pressedPosition = event->pos();
        d->pressedPosition.rx() += horizontalOffset();
        d->pressedPosition.ry() += verticalOffset();
    }
    if (!d->categoryDrawer) {
        QListView::mousePressEvent(event);
        return;
    }
    auto it = d->blocks.constBegin();
    while (it != d->blocks.constEnd()) {
        const KCategorizedViewPrivate::Block &block = *it;
        const QModelIndex categoryIndex = d->proxyModel->index(block.firstIndex.row(), d->proxyModel->sortColumn(), rootIndex());
        const QStyleOptionViewItem option = d->blockRect(categoryIndex);
        const QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
        if (option.rect.contains(mousePos)) {
            d->categoryDrawer->mouseButtonPressed(categoryIndex, option.rect, event);
            viewport()->update(option.rect);
            if (!event->isAccepted()) {
                QListView::mousePressEvent(event);
            }
            return;
        }
        ++it;
    }
    QListView::mousePressEvent(event);
}

void KCategorizedView::mouseReleaseEvent(QMouseEvent *event)
{
    d->pressedPosition = QPoint();
    d->rubberBandRect = QRect();
    if (!d->categoryDrawer) {
        QListView::mouseReleaseEvent(event);
        return;
    }
    auto it = d->blocks.constBegin();
    while (it != d->blocks.constEnd()) {
        const KCategorizedViewPrivate::Block &block = *it;
        const QModelIndex categoryIndex = d->proxyModel->index(block.firstIndex.row(), d->proxyModel->sortColumn(), rootIndex());
        const QStyleOptionViewItem option = d->blockRect(categoryIndex);
        const QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
        if (option.rect.contains(mousePos)) {
            d->categoryDrawer->mouseButtonReleased(categoryIndex, option.rect, event);
            viewport()->update(option.rect);
            if (!event->isAccepted()) {
                QListView::mouseReleaseEvent(event);
            }
            return;
        }
        ++it;
    }
    QListView::mouseReleaseEvent(event);
}

void KCategorizedView::leaveEvent(QEvent *event)
{
    QListView::leaveEvent(event);
    if (d->hoveredIndex.isValid()) {
        viewport()->update(visualRect(d->hoveredIndex));
        d->hoveredIndex = QModelIndex();
    }
    if (d->categoryDrawer && d->hoveredBlock->height != -1) {
        const QModelIndex categoryIndex = d->proxyModel->index(d->hoveredBlock->firstIndex.row(), d->proxyModel->sortColumn(), rootIndex());
        const QStyleOptionViewItem option = d->blockRect(categoryIndex);
        d->categoryDrawer->mouseLeft(categoryIndex, option.rect);
        *d->hoveredBlock = KCategorizedViewPrivate::Block();
        d->hoveredCategory = QString();
        viewport()->update(option.rect);
    }
}

void KCategorizedView::startDrag(Qt::DropActions supportedActions)
{
    QListView::startDrag(supportedActions);
}

void KCategorizedView::dragMoveEvent(QDragMoveEvent *event)
{
    QListView::dragMoveEvent(event);
    d->hoveredIndex = indexAt(event->position().toPoint());
}

void KCategorizedView::dragEnterEvent(QDragEnterEvent *event)
{
    QListView::dragEnterEvent(event);
}

void KCategorizedView::dragLeaveEvent(QDragLeaveEvent *event)
{
    QListView::dragLeaveEvent(event);
}

void KCategorizedView::dropEvent(QDropEvent *event)
{
    QListView::dropEvent(event);
}

// TODO: improve se we take into account collapsed blocks
// TODO: take into account when there is no grid and no uniformItemSizes
QModelIndex KCategorizedView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    if (!d->isCategorized() || viewMode() == QListView::ListMode) {
        return QListView::moveCursor(cursorAction, modifiers);
    }

    const QModelIndex current = currentIndex();
    const QRect currentRect = visualRect(current);
    if (!current.isValid()) {
        const int rowCount = d->proxyModel->rowCount(rootIndex());
        if (!rowCount) {
            return QModelIndex();
        }
        return d->proxyModel->index(0, modelColumn(), rootIndex());
    }

    switch (cursorAction) {
    case MoveLeft: {
        if (!current.row()) {
            return QModelIndex();
        }
        const QModelIndex previous = d->proxyModel->index(current.row() - 1, modelColumn(), rootIndex());
        const QRect previousRect = visualRect(previous);
        if (previousRect.top() == currentRect.top()) {
            return previous;
        }

        return QModelIndex();
    }
    case MoveRight: {
        if (current.row() == d->proxyModel->rowCount() - 1) {
            return QModelIndex();
        }
        const QModelIndex next = d->proxyModel->index(current.row() + 1, modelColumn(), rootIndex());
        const QRect nextRect = visualRect(next);
        if (nextRect.top() == currentRect.top()) {
            return next;
        }

        return QModelIndex();
    }
    case MoveDown: {
        if (d->hasGrid() || uniformItemSizes()) {
            const QModelIndex current = currentIndex();
            const QSize itemSize = d->hasGrid() ? gridSize() : sizeHintForIndex(current);
            const KCategorizedViewPrivate::Block &block = d->blocks[d->categoryForIndex(current)];
            const int maxItemsPerRow = qMax(d->viewportWidth() / itemSize.width(), 1);
            const bool canMove = current.row() + maxItemsPerRow < block.firstIndex.row() + block.items.count();

            if (canMove) {
                return d->proxyModel->index(current.row() + maxItemsPerRow, modelColumn(), rootIndex());
            }

            const int currentRelativePos = (current.row() - block.firstIndex.row()) % maxItemsPerRow;
            const QModelIndex nextIndex = d->proxyModel->index(block.firstIndex.row() + block.items.count(), modelColumn(), rootIndex());

            if (!nextIndex.isValid()) {
                return QModelIndex();
            }

            const KCategorizedViewPrivate::Block &nextBlock = d->blocks[d->categoryForIndex(nextIndex)];

            if (nextBlock.items.count() <= currentRelativePos) {
                return QModelIndex();
            }

            if (currentRelativePos < (block.items.count() % maxItemsPerRow)) {
                return d->proxyModel->index(nextBlock.firstIndex.row() + currentRelativePos, modelColumn(), rootIndex());
            }
        }
        return QModelIndex();
    }
    case MoveUp: {
        if (d->hasGrid() || uniformItemSizes()) {
            const QModelIndex current = currentIndex();
            const QSize itemSize = d->hasGrid() ? gridSize() : sizeHintForIndex(current);
            const KCategorizedViewPrivate::Block &block = d->blocks[d->categoryForIndex(current)];
            const int maxItemsPerRow = qMax(d->viewportWidth() / itemSize.width(), 1);
            const bool canMove = current.row() - maxItemsPerRow >= block.firstIndex.row();

            if (canMove) {
                return d->proxyModel->index(current.row() - maxItemsPerRow, modelColumn(), rootIndex());
            }

            const int currentRelativePos = (current.row() - block.firstIndex.row()) % maxItemsPerRow;
            const QModelIndex prevIndex = d->proxyModel->index(block.firstIndex.row() - 1, modelColumn(), rootIndex());

            if (!prevIndex.isValid()) {
                return QModelIndex();
            }

            const KCategorizedViewPrivate::Block &prevBlock = d->blocks[d->categoryForIndex(prevIndex)];

            if (prevBlock.items.count() <= currentRelativePos) {
                return QModelIndex();
            }

            const int remainder = prevBlock.items.count() % maxItemsPerRow;
            if (currentRelativePos < remainder) {
                return d->proxyModel->index(prevBlock.firstIndex.row() + prevBlock.items.count() - remainder + currentRelativePos, modelColumn(), rootIndex());
            }

            return QModelIndex();
        }
        break;
    }
    default:
        break;
    }

    return QModelIndex();
}

void KCategorizedView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if (!d->isCategorized()) {
        QListView::rowsAboutToBeRemoved(parent, start, end);
        return;
    }

    *d->hoveredBlock = KCategorizedViewPrivate::Block();
    d->hoveredCategory = QString();

    if (end - start + 1 == d->proxyModel->rowCount()) {
        d->blocks.clear();
        QListView::rowsAboutToBeRemoved(parent, start, end);
        return;
    }

    // Removal feels a bit more complicated than insertion. Basically we can consider there are
    // 3 different cases when going to remove items. (*) represents an item, Items between ([) and
    // (]) are the ones which are marked for removal.
    //
    // - 1st case:
    //              ... * * * * * * [ * * * ...
    //
    //   The items marked for removal are the last part of this category. No need to mark any item
    //   of this category as in quarantine, because no special offset will be pushed to items at
    //   the right because of any changes (since the removed items are those on the right most part
    //   of the category).
    //
    // - 2nd case:
    //              ... * * * * * * ] * * * ...
    //
    //   The items marked for removal are the first part of this category. We have to mark as in
    //   quarantine all items in this category. Absolutely all. All items will have to be moved to
    //   the left (or moving up, because rows got a different offset).
    //
    // - 3rd case:
    //              ... * * [ * * * * ] * * ...
    //
    //   The items marked for removal are in between of this category. We have to mark as in
    //   quarantine only those items that are at the right of the end of the removal interval,
    //   (starting on "]").
    //
    // It hasn't been explicitly said, but when we remove, we have to mark all blocks that are
    // located under the top most affected category as in quarantine (the block itself, as a whole),
    // because such a change can force it to have a different offset (note that items themselves
    // contain relative positions to the block, so marking the block as in quarantine is enough).
    //
    // Also note that removal implicitly means that we have to update correctly firstIndex of each
    // block, and in general keep updated the internal information of elements.

    QStringList listOfCategoriesMarkedForRemoval;

    QString lastCategory;
    int alreadyRemoved = 0;
    for (int i = start; i <= end; ++i) {
        const QModelIndex index = d->proxyModel->index(i, modelColumn(), parent);

        Q_ASSERT(index.isValid());

        const QString category = d->categoryForIndex(index);

        if (lastCategory != category) {
            lastCategory = category;
            alreadyRemoved = 0;
        }

        KCategorizedViewPrivate::Block &block = d->blocks[category];
        block.items.removeAt(i - block.firstIndex.row() - alreadyRemoved);
        ++alreadyRemoved;

        if (block.items.isEmpty()) {
            listOfCategoriesMarkedForRemoval << category;
        }

        block.height = -1;

        viewport()->update();
    }

    // BEGIN: update the items that are in quarantine in affected categories
    {
        const QModelIndex lastIndex = d->proxyModel->index(end, modelColumn(), parent);
        const QString category = d->categoryForIndex(lastIndex);
        KCategorizedViewPrivate::Block &block = d->blocks[category];
        if (!block.items.isEmpty() && start <= block.firstIndex.row() && end >= block.firstIndex.row()) {
            block.firstIndex = d->proxyModel->index(end + 1, modelColumn(), parent);
        }
        block.quarantineStart = block.firstIndex;
    }
    // END: update the items that are in quarantine in affected categories

    for (const QString &category : std::as_const(listOfCategoriesMarkedForRemoval)) {
        d->blocks.remove(category);
    }

    // BEGIN: mark as in quarantine those categories that are under the affected ones
    {
        // BEGIN: order for marking as alternate those blocks that are alternate
        QList<KCategorizedViewPrivate::Block> blockList = d->blocks.values();
        std::sort(blockList.begin(), blockList.end(), KCategorizedViewPrivate::Block::lessThan);
        QList<int> firstIndexesRows;
        for (const KCategorizedViewPrivate::Block &block : std::as_const(blockList)) {
            firstIndexesRows << block.firstIndex.row();
        }
        // END: order for marking as alternate those blocks that are alternate
        for (auto it = d->blocks.begin(); it != d->blocks.end(); ++it) {
            KCategorizedViewPrivate::Block &block = *it;
            if (block.firstIndex.row() > start) {
                block.outOfQuarantine = false;
                block.alternate = firstIndexesRows.indexOf(block.firstIndex.row()) % 2;
            } else if (block.firstIndex.row() == start) {
                block.alternate = firstIndexesRows.indexOf(block.firstIndex.row()) % 2;
            }
        }
    }
    // END: mark as in quarantine those categories that are under the affected ones

    QListView::rowsAboutToBeRemoved(parent, start, end);
}

void KCategorizedView::updateGeometries()
{
    const int oldVerticalOffset = verticalOffset();
    const Qt::ScrollBarPolicy verticalP = verticalScrollBarPolicy();
    const Qt::ScrollBarPolicy horizontalP = horizontalScrollBarPolicy();

    // BEGIN bugs 213068, 287847 ------------------------------------------------------------
    /*
     * QListView::updateGeometries() has it's own opinion on whether the scrollbars should be visible (valid range) or not
     * and triggers a (sometimes additionally timered) resize through ::layoutChildren()
     * http://qt.gitorious.org/qt/qt/blobs/4.7/src/gui/itemviews/qlistview.cpp#line1499
     * (the comment above the main block isn't all accurate, layoutChldren is called regardless of the policy)
     *
     * As a result QListView and KCategorizedView occasionally started a race on the scrollbar visibility, effectively blocking the UI
     * So we prevent QListView from having an own opinion on the scrollbar visibility by
     * fixing it before calling the baseclass QListView::updateGeometries()
     *
     * Since the implicit show/hide by the following range setting will cause further resizes if the policy is Qt::ScrollBarAsNeeded
     * we keep it static until we're done, then restore the original value and ultimately change the scrollbar visibility ourself.
     */
    if (d->isCategorized()) { // important! - otherwise we'd pollute the setting if the view is initially not categorized
        setVerticalScrollBarPolicy((verticalP == Qt::ScrollBarAlwaysOn || verticalScrollBar()->isVisibleTo(this)) ? Qt::ScrollBarAlwaysOn
                                                                                                                  : Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy((horizontalP == Qt::ScrollBarAlwaysOn || horizontalScrollBar()->isVisibleTo(this)) ? Qt::ScrollBarAlwaysOn
                                                                                                                        : Qt::ScrollBarAlwaysOff);
    }
    // END bugs 213068, 287847 --------------------------------------------------------------

    QListView::updateGeometries();

    if (!d->isCategorized()) {
        return;
    }

    const int rowCount = d->proxyModel->rowCount();
    if (!rowCount) {
        verticalScrollBar()->setRange(0, 0);
        // unconditional, see function end todo
        // BEGIN bugs 213068, 287847 ------------------------------------------------------------
        // restoring values from above ...
        horizontalScrollBar()->setRange(0, 0);
        setVerticalScrollBarPolicy(verticalP);
        setHorizontalScrollBarPolicy(horizontalP);
        // END bugs 213068, 287847 --------------------------------------------------------------
        return;
    }

    const QModelIndex lastIndex = d->proxyModel->index(rowCount - 1, modelColumn(), rootIndex());
    Q_ASSERT(lastIndex.isValid());
    QRect lastItemRect = visualRect(lastIndex);

    if (d->hasGrid()) {
        lastItemRect.setSize(lastItemRect.size().expandedTo(gridSize()));
    } else {
        if (uniformItemSizes()) {
            QSize itemSize = sizeHintForIndex(lastIndex);
            itemSize.setHeight(itemSize.height() + spacing());
            lastItemRect.setSize(itemSize);
        } else {
            QSize itemSize = sizeHintForIndex(lastIndex);
            const QString category = d->categoryForIndex(lastIndex);
            itemSize.setHeight(d->highestElementInLastRow(d->blocks[category]) + spacing());
            lastItemRect.setSize(itemSize);
        }
    }

    const int bottomRange = lastItemRect.bottomRight().y() + verticalOffset() - viewport()->height();

    if (verticalScrollMode() == ScrollPerItem) {
        verticalScrollBar()->setSingleStep(lastItemRect.height());
        const int rowsPerPage = qMax(viewport()->height() / lastItemRect.height(), 1);
        verticalScrollBar()->setPageStep(rowsPerPage * lastItemRect.height());
    }

    verticalScrollBar()->setRange(0, bottomRange);
    verticalScrollBar()->setValue(oldVerticalOffset);

    // TODO: also consider working with the horizontal scroll bar. since at this level I am not still
    //      supporting "top to bottom" flow, there is no real problem. If I support that someday
    //      (think how to draw categories), we would have to take care of the horizontal scroll bar too.
    //      In theory, as KCategorizedView has been designed, there is no need of horizontal scroll bar.
    horizontalScrollBar()->setRange(0, 0);

    // BEGIN bugs 213068, 287847 ------------------------------------------------------------
    // restoring values from above ...
    setVerticalScrollBarPolicy(verticalP);
    setHorizontalScrollBarPolicy(horizontalP);
    // ... and correct the visibility
    bool validRange = verticalScrollBar()->maximum() != verticalScrollBar()->minimum();
    if (verticalP == Qt::ScrollBarAsNeeded && (verticalScrollBar()->isVisibleTo(this) != validRange)) {
        verticalScrollBar()->setVisible(validRange);
    }
    validRange = horizontalScrollBar()->maximum() > horizontalScrollBar()->minimum();
    if (horizontalP == Qt::ScrollBarAsNeeded && (horizontalScrollBar()->isVisibleTo(this) != validRange)) {
        horizontalScrollBar()->setVisible(validRange);
    }
    // END bugs 213068, 287847 --------------------------------------------------------------
}

void KCategorizedView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QListView::currentChanged(current, previous);
}

void KCategorizedView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    QListView::dataChanged(topLeft, bottomRight, roles);
    if (!d->isCategorized()) {
        return;
    }

    *d->hoveredBlock = KCategorizedViewPrivate::Block();
    d->hoveredCategory = QString();

    // BEGIN: since the model changed data, we need to reconsider item sizes
    int i = topLeft.row();
    int indexToCheck = i;
    QModelIndex categoryIndex;
    QString category;
    KCategorizedViewPrivate::Block *block;
    while (i <= bottomRight.row()) {
        const QModelIndex currIndex = d->proxyModel->index(i, modelColumn(), rootIndex());
        if (i == indexToCheck) {
            categoryIndex = d->proxyModel->index(i, d->proxyModel->sortColumn(), rootIndex());
            category = categoryIndex.data(KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();
            block = &d->blocks[category];
            block->quarantineStart = currIndex;
            indexToCheck = block->firstIndex.row() + block->items.count();
        }
        visualRect(currIndex);
        ++i;
    }
    // END: since the model changed data, we need to reconsider item sizes
}

void KCategorizedView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    if (!d->isCategorized()) {
        return;
    }

    *d->hoveredBlock = KCategorizedViewPrivate::Block();
    d->hoveredCategory = QString();
    d->rowsInserted(parent, start, end);
}

void KCategorizedView::slotLayoutChanged()
{
    if (!d->isCategorized()) {
        return;
    }

    d->blocks.clear();
    *d->hoveredBlock = KCategorizedViewPrivate::Block();
    d->hoveredCategory = QString();
    if (d->proxyModel->rowCount()) {
        d->rowsInserted(rootIndex(), 0, d->proxyModel->rowCount() - 1);
    }
}

// END: Public part

#include "moc_kcategorizedview.cpp"
