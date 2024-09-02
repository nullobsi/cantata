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

#include "icon.h"
#include "utils.h"
#include <QApplication>
#include <QMessageBox>
#include <QToolButton>
#include <QtAwesome.h>
#include <qapplicationstatic.h>

Q_APPLICATION_STATIC(fa::QtAwesome, awesome, qApp);

int Icon::stdSize(int v)
{
	if (v < 19) {
		return 16;
	}
	else if (v <= 28) {
		return 22;
	}
	else if (v <= 40) {
		return 32;
	}
	else if (v <= 56) {
		return 48;
	}
	else if (v <= 90) {
		return 64;
	}

	if (Utils::isHighDpi()) {
		if (v <= 160) {
			return 128;
		}
		else {
			return 256;
		}
	}
	return 128;
}

int Icon::dlgIconSize()
{
	static int size = -1;

	if (-1 == size) {
		QMessageBox* box = new QMessageBox(nullptr);
		box->setVisible(false);
		box->setIcon(QMessageBox::Information);
		box->ensurePolished();
		QPixmap pix = box->iconPixmap();
		if (pix.isNull() || pix.width() < 16) {
			size = stdSize(QFontMetricsF(QApplication::font()).height() * 3.5);
		}
		else {
			size = pix.width();
		}
		box->deleteLater();
	}
	return size;
}

void Icon::init(QToolButton* btn, bool setFlat)
{
	static int size = -1;

	if (-1 == size) {
		size = QFontMetricsF(QApplication::font()).height();
		if (size > 22) {
			size = stdSize(size * 1.1);
		}
		else {
			size = 16;
		}
	}
	btn->setIconSize(QSize(size, size));
	if (setFlat) {
		btn->setAutoRaise(true);
	}
}

QPixmap Icon::getScaledPixmap(const QIcon& icon, int w, int h, int base)
{
	QList<QSize> sizes = icon.availableSizes();
	for (const QSize& s : sizes) {
		if (s.width() == w && s.height() == h) {
			return icon.pixmap(s);
		}
	}

	return icon.pixmap(base, base).scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

fa::QtAwesome* Icon::getFaRaw() {
	return awesome;
}

fa::QtAwesome* Icon::fa()
{
	awesome->initFontAwesome();
	return awesome;
}

QIcon Icon::fa(int s, int c, const QVariantMap& m)
{
	awesome->initFontAwesome();
	return awesome->icon(s, c, m);
}
QIcon Icon::fa(const QString& name, const QVariantMap& m)
{
	awesome->initFontAwesome();
	return awesome->icon(name, m);
}

QColor Icon::constRed(196, 32, 32);
