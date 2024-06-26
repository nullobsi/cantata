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

#include "musiclibraryproxymodel.h"
#include "musiclibraryitem.h"
#include "musiclibraryitemalbum.h"
#include "musiclibraryitemartist.h"
#include "musiclibraryitemsong.h"

MusicLibraryProxyModel::MusicLibraryProxyModel(QObject* parent)
	: ProxyModel(parent)
{
	setDynamicSortFilter(true);
	setFilterCaseSensitivity(Qt::CaseInsensitive);
	setSortCaseSensitivity(Qt::CaseInsensitive);
	setSortLocaleAware(true);
}

bool MusicLibraryProxyModel::filterAcceptsRoot(const MusicLibraryItem* item) const
{
	for (const MusicLibraryItem* i : static_cast<const MusicLibraryItemContainer*>(item)->childItems()) {
		if (MusicLibraryItem::Type_Artist == i->itemType() && filterAcceptsArtist(i)) {
			return true;
		}
		else if (MusicLibraryItem::Type_Song == i->itemType() && filterAcceptsSong(i)) {
			return true;
		}
	}

	return false;
}

bool MusicLibraryProxyModel::filterAcceptsArtist(const MusicLibraryItem* item) const
{
	for (const MusicLibraryItem* i : static_cast<const MusicLibraryItemContainer*>(item)->childItems()) {
		if (filterAcceptsAlbum(i)) {
			return true;
		}
	}

	return false;
}

bool MusicLibraryProxyModel::filterAcceptsAlbum(const MusicLibraryItem* item) const
{
	for (const MusicLibraryItem* i : static_cast<const MusicLibraryItemContainer*>(item)->childItems()) {
		if (filterAcceptsSong(i)) {
			return true;
		}
	}

	return false;
}

bool MusicLibraryProxyModel::filterAcceptsSong(const MusicLibraryItem* item) const
{
	return matchesFilter(static_cast<const MusicLibraryItemSong*>(item)->song());
}

bool MusicLibraryProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	if (!filterEnabled) {
		return true;
	}
	if (!isChildOfRoot(sourceParent)) {
		return true;
	}

	const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	const MusicLibraryItem* item = static_cast<const MusicLibraryItem*>(index.internalPointer());

	if (filter) {
		if (item == filter) {// Accept top-level item!
			return true;
		}
		const MusicLibraryItem* p = item->parentItem();
		while (p && p->parentItem()) {
			p = p->parentItem();
		}
		if (p != filter) {// Accept all items that are not children of top-level item!
			return true;
		}
	}

	if (filterStrings.isEmpty()) {
		return true;
	}

	switch (item->itemType()) {
	case MusicLibraryItem::Type_Root:
		return filterAcceptsRoot(item);
	case MusicLibraryItem::Type_Artist:
		return filterAcceptsArtist(item);
	case MusicLibraryItem::Type_Album:
		return filterAcceptsAlbum(item);
	case MusicLibraryItem::Type_Song:
		return filterAcceptsSong(item);
	default:
		break;
	}

	return false;
}

bool MusicLibraryProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	if (left.row() < 0 || right.row() < 0) {
		return left.row() < 0;
	}

	if (static_cast<MusicLibraryItem*>(left.internalPointer())->itemType() == MusicLibraryItem::Type_Song) {
		MusicLibraryItemSong* l = static_cast<MusicLibraryItemSong*>(left.internalPointer());
		MusicLibraryItemSong* r = static_cast<MusicLibraryItemSong*>(right.internalPointer());
		return l->song() < r->song();
	}
	else if (static_cast<MusicLibraryItem*>(left.internalPointer())->itemType() == MusicLibraryItem::Type_Album) {
		return MusicLibraryItemAlbum::lessThan(static_cast<MusicLibraryItem*>(left.internalPointer()), static_cast<MusicLibraryItem*>(right.internalPointer()));
	}
	else if (static_cast<MusicLibraryItem*>(left.internalPointer())->itemType() == MusicLibraryItem::Type_Artist) {
		return MusicLibraryItemArtist::lessThan(static_cast<MusicLibraryItem*>(left.internalPointer()), static_cast<MusicLibraryItem*>(right.internalPointer()));
	}

	return QSortFilterProxyModel::lessThan(left, right);
}

#include "moc_musiclibraryproxymodel.cpp"
