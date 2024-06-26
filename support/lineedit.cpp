/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
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
#include "lineedit.h"
#include <QApplication>

void LineEdit::setReadOnly(bool e)
{
	QLineEdit::setReadOnly(e);
	if (e) {
		QPalette p(palette());
		p.setColor(QPalette::Active, QPalette::Base, p.color(QPalette::Active, QPalette::Window));
		p.setColor(QPalette::Disabled, QPalette::Base, p.color(QPalette::Disabled, QPalette::Window));
		p.setColor(QPalette::Inactive, QPalette::Base, p.color(QPalette::Inactive, QPalette::Window));
		setPalette(p);
	}
	else {
		setPalette(qApp->palette());
	}
}
