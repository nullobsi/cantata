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

#include "trayitem.h"
#include "config.h"
#include "currentcover.h"
#include "mainwindow.h"
#include "mpd-interface/song.h"
#include "settings.h"
#include "stdactions.h"
#include "support/action.h"
#include "support/utils.h"
#include "widgets/icons.h"
#include <QMenu>
#include <QWheelEvent>
#include <knotification.h>

#ifndef Q_OS_MAC
class VolumeSliderEventHandler : public QObject {
public:
	VolumeSliderEventHandler(QObject* p) : QObject(p) {}

protected:
	bool eventFilter(QObject* obj, QEvent* event) override
	{
		if (QEvent::Wheel == event->type()) {
			int numDegrees = static_cast<QWheelEvent*>(event)->angleDelta().y() / 8;
			int numSteps = numDegrees / 15;
			if (numSteps > 0) {
				for (int i = 0; i < numSteps; ++i) {
					StdActions::self()->increaseVolumeAction->trigger();
				}
			}
			else {
				for (int i = 0; i > numSteps; --i) {
					StdActions::self()->decreaseVolumeAction->trigger();
				}
			}
			return true;
		}

		return QObject::eventFilter(obj, event);
	}
};
#endif

TrayItem::TrayItem(MainWindow* p)
	: QObject(p)
#ifndef Q_OS_MAC
	  ,
	  mw(p), trayItem(nullptr), trayItemMenu(nullptr), connectionsAction(nullptr), partitionsAction(nullptr), outputsAction(nullptr)
#endif
	  ,
	  songNotif(nullptr)
{
}

Q_DECL_UNUSED static Action* copyAction(Action* orig)
{
	Action* newAction = new Action(orig->parent());
	newAction->setText(Utils::strippedText(orig->text()));
	newAction->setIcon(orig->icon());
	QObject::connect(newAction, SIGNAL(triggered()), orig, SIGNAL(triggered()));
	QObject::connect(newAction, SIGNAL(triggered(bool)), orig, SIGNAL(triggered(bool)));
	return newAction;
}

void TrayItem::setup()
{
#ifndef Q_OS_MAC
	if (!Settings::self()->useSystemTray() || !Utils::useSystemTray()) {
		if (trayItem) {
			trayItem->setVisible(false);
			trayItem->deleteLater();
			trayItem = nullptr;
			trayItemMenu->deleteLater();
			trayItemMenu = nullptr;
		}
		return;
	}

	if (trayItem) {
		return;
	}

#ifndef Q_OS_MAC
	connectionsAction = new Action(Utils::strippedText(mw->connectionsAction->text()), this);
	connectionsAction->setVisible(false);
	partitionsAction = new Action(Utils::strippedText(mw->partitionsAction->text()), this);
	partitionsAction->setVisible(false);
	outputsAction = new Action(Utils::strippedText(mw->outputsAction->text()), this);
	outputsAction->setVisible(false);
	updateConnections();
	updatePartitions();
	updateOutputs();
#endif

	// What systems DONT have a system tray? Also, isSytemTrayAvailable is checked in config dialog, so
	// useSystemTray should not be set if there is none.
	// Checking here seems to cause the icon not to appear if Cantata is autostarted in Plasma5 - #759
	//if (!QSystemTrayIcon::isSystemTrayAvailable()) {
	//    return;
	//}
	// Create tray backend
#if defined(USE_KSNI)
	trayItem = new KStatusNotifierItem(this);
	trayItem->setCategory(KStatusNotifierItem::ApplicationStatus);
	trayItem->setStatus(KStatusNotifierItem::Active);
	trayItemMenu = new QMenu(nullptr);
	trayItem->setContextMenu(trayItemMenu);
#else
	trayItem = new QSystemTrayIcon(this);
	trayItem->installEventFilter(new VolumeSliderEventHandler(this));
	trayItemMenu = new QMenu(nullptr);
	trayItem->setContextMenu(trayItemMenu);
#endif
	// Populate common menu
	trayItemMenu->addAction(StdActions::self()->prevTrackAction);
	trayItemMenu->addAction(StdActions::self()->playPauseTrackAction);
	trayItemMenu->addAction(StdActions::self()->stopPlaybackAction);
	trayItemMenu->addAction(StdActions::self()->stopAfterCurrentTrackAction);
	trayItemMenu->addAction(StdActions::self()->nextTrackAction);
#ifndef Q_OS_MAC
	trayItemMenu->addSeparator();
	trayItemMenu->addAction(connectionsAction);
	trayItemMenu->addAction(partitionsAction);
	trayItemMenu->addAction(outputsAction);
#endif
	trayItemMenu->addSeparator();
	trayItemMenu->addAction(mw->restoreAction);
	trayItemMenu->addSeparator();
	trayItemMenu->addAction(copyAction(mw->quitAction));
	trayItem->setContextMenu(trayItemMenu);
#if defined Q_OS_MAC || defined Q_OS_WIN
	QIcon icon;
	icon.addFile(CANTATA_SYS_ICONS_DIR + PROJECT_REV_ID ".png");
#else
	QIcon icon = QIcon::fromTheme(Utils::Gnome == Utils::currentDe() ? PROJECT_REV_ID "-symbolic" : PROJECT_REV_ID);
	// Bug: 660 If installed to non-standard folder, QIcon::fromTheme does not seem to find icon. Therefore
	// add icon files here...
	if (icon.isNull()) {
		QStringList sizes = QStringList() << "16"
										  << "22"
										  << "24"
										  << "32"
										  << "48"
										  << "64";
		for (const QString& s : sizes) {
			icon.addFile(QLatin1String(ICON_INSTALL_PREFIX "/") + s + QLatin1Char('x') + s + QLatin1String("/apps/" PROJECT_REV_ID ".png"));
		}

		icon.addFile(QLatin1String(ICON_INSTALL_PREFIX "/scalable/apps/" PROJECT_REV_ID ".svg"));
	}
#endif

	trayItem->setIcon(icon);
#if defined(USE_KSNI)
	trayItem->setToolTip(QIcon::fromTheme(QStringLiteral(PROJECT_REV_ID)), tr("Cantata"), QString());
	// KStatusNotifierItem does not need show() or QSystemTrayIcon connections
#else
	trayItem->setToolTip(tr("Cantata"));
	trayItem->show();
	connect(trayItem, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	        this, SLOT(trayItemClicked(QSystemTrayIcon::ActivationReason)));
#endif

#endif
}

#if defined(USE_KSNI)
void TrayItem::setOverlayPaused(bool paused)
{
	if (!trayItem) return;
	if (paused) {
		trayItem->setOverlayIconByName(QStringLiteral("media-playback-pause"));
	}
	else {
		trayItem->setOverlayIcon(QIcon());// clear overlay
	}
}
#endif

void TrayItem::trayItemClicked(QSystemTrayIcon::ActivationReason reason)
{
#ifdef Q_OS_MAC
	Q_UNUSED(reason)
#else
	switch (reason) {
	case QSystemTrayIcon::Trigger:
		if (mw->isHidden()) {
			mw->restoreWindow();
		}
		else {
			mw->hideWindow();
		}
		break;
	case QSystemTrayIcon::MiddleClick:
		mw->playPauseTrack();
		break;
	default:
		break;
	}
#endif
}

void TrayItem::songChanged(const Song& song, bool isPlaying)
{
	bool useable = song.isStandardStream()
			? !song.title.isEmpty() && !song.name().isEmpty()
			: !song.title.isEmpty() && !song.artist.isEmpty() && !song.album.isEmpty();
	if (useable && isPlaying) {
		// Always emit a KDE notification: Plasma can suppress popups but still keep history.
		auto* n = new KNotification(QStringLiteral("newSong"));
		// Ensure it maps to cantata.notifyrc (safer for per‑app settings/history)
		n->setComponentName(QStringLiteral("cantata"));
		n->setTitle(song.mainText());
		n->setText(song.subText());
		n->setImage(CurrentCover::self()->image().scaledToHeight(512, Qt::SmoothTransformation));
		n->setUrgency(KNotification::LowUrgency);
		n->sendEvent();
	}

#if defined(USE_KSNI)
	const QString title = song.isStandardStream()
			? song.name()
			: QStringLiteral("%1 — %2").arg(song.artist, song.title);
	const QString sub = song.isStandardStream() ? song.title : song.album;
	setToolTip(QStringLiteral("audio-x-generic"), title, sub);
	setOverlayPaused(!isPlaying);
#else
	const QString oneLine = song.isStandardStream()
			? QStringLiteral("%1 — %2").arg(song.name(), song.title)
			: QStringLiteral("%1 — %2").arg(song.artist, song.title);
	setToolTip(QString(), oneLine, QString());
#endif
}

#ifndef Q_OS_MAC
static void copyMenu(Action* from, Action* to)
{
	if (!to) {
		return;
	}
	to->setVisible(from->isVisible());
	if (to->isVisible()) {
		if (!to->menu()) {
			to->setMenu(new QMenu(nullptr));
		}
		QMenu* m = to->menu();
		m->clear();

		for (QAction* act : from->menu()->actions()) {
			m->addAction(act);
		}
	}
}
#endif

void TrayItem::updateConnections()
{
#ifndef Q_OS_MAC
	copyMenu(mw->connectionsAction, connectionsAction);
#endif
}

void TrayItem::updatePartitions()
{
#ifndef Q_OS_MAC
	copyMenu(mw->partitionsAction, partitionsAction);
#endif
}

void TrayItem::updateOutputs()
{
#ifndef Q_OS_MAC
	copyMenu(mw->outputsAction, outputsAction);
#endif
}

#include "moc_trayitem.cpp"
