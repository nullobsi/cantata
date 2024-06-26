/***************************************************************************
 *   Copyright (C) 2010 by the Quassel Project                             *
 *   devel@quassel-irc.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3.                                           *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef SHORTCUTSSETTINGSWIDGET_H
#define SHORTCUTSSETTINGSWIDGET_H

#include "support/ui_shortcutssettingswidget.h"
#include <QSortFilterProxyModel>

class ActionCollection;
class ShortcutsModel;

class ShortcutsFilter : public QSortFilterProxyModel {
	Q_OBJECT
public:
	ShortcutsFilter(QObject* parent = nullptr);

public Q_SLOTS:
	void setFilterString(const QString& filterString);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
	QString _filterString;
};

class ShortcutsSettingsWidget : public QWidget, private Ui::ShortcutsSettingsWidget {
	Q_OBJECT
public:
	ShortcutsSettingsWidget(const QHash<QString, ActionCollection*>& actionCollections, QWidget* parent = nullptr);

	inline bool hasDefaults() const { return true; }
	QTreeView* view();

public Q_SLOTS:
	void save();

private Q_SLOTS:
	void on_searchEdit_textChanged(const QString& text);
	void keySequenceChanged(const QKeySequence& seq, const QModelIndex& conflicting);
	void setWidgetStates();
	void toggledCustomOrDefault();

private:
	ShortcutsModel* _shortcutsModel;
	ShortcutsFilter* _shortcutsFilter;
};

#endif// SHORTCUTSSETTINGSPAGE_H
