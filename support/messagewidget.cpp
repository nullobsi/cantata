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

#include "messagewidget.h"

MessageWidget::MessageWidget(QWidget* parent)
	: KMsgWidget(parent), active(false), msgType(Positive)
{
}

MessageWidget::~MessageWidget()
{
}

void MessageWidget::setMessage(const QString& msg, MessageType type, bool showCloseButton)
{
	if (isActive() && !msg.isEmpty() && type != msgType) {
		setVisible(false);
	}
	msgType = type;
	if (msg.isEmpty() && isVisible()) {
		setVisible(false);
		return;
	}
	//    QString text=msg;
	//    if (text.length()>154) {
	//        text=text.left(150)+QLatin1String("...");
	//    }
	//    if (msg.length()>500) {
	//        setToolTip(msg.left(500)+QLatin1String("..."));
	//    } else {
	//        setToolTip(msg);
	//    }
	//    setText(text);
	setText(msg);
	setToolTip(msg);
	setMessageType(type);
	setCloseButtonVisible(showCloseButton);
#if defined NO_ANIMATED_SHOW
	setVisible(true);
#else
	if (!parentWidget()->isVisible()) {
		show();
		setVisible(true);
	}
	else {
		animatedShow();
	}
#endif
}

void MessageWidget::setVisible(bool v)
{
	active = v;
	KMsgWidget::setVisible(v);
	emit visible(v);
}

void MessageWidget::removeAllActions()
{
	QList<QAction*> acts = actions();
	for (QAction* a : acts) {
		removeAction(a);
	}
}

void MessageWidget::setActions(const QList<QAction*> acts)
{
	if (acts == actions()) {
		return;
	}

	removeAllActions();
	for (QAction* a : acts) {
		addAction(a);
	}
}

#include "moc_messagewidget.cpp"
