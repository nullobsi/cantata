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

#ifndef PODCAST_SEARCH_DIALOG_H
#define PODCAST_SEARCH_DIALOG_H

#include "support/dialog.h"
#include "support/icon.h"
#include <QList>
#include <QUrl>

class LineEdit;
class QTreeWidget;
class NetworkJob;
class QIODevice;
class Spinner;
class QTreeWidgetItem;
class TextBrowser;
class MessageWidget;
class PageWidget;
class PodcastService;

namespace OpmlParser {
struct Category;
struct Podcast;
}// namespace OpmlParser

class PodcastPage : public QWidget {
	Q_OBJECT
public:
	PodcastPage(QWidget* p, const QString& n);
	~PodcastPage() override
	{
		cancel();
		cancelImage();
	}

	const QIcon& icon() const { return icn; }
	const QString& name() const { return pageName; }
	QUrl currentRss() const;

Q_SIGNALS:
	void rssSelected(const QUrl& url);
	void error(const QString& msg);

protected:
	void fetch(const QUrl& url);
	void fetchImage(const QUrl& url);
	void cancel();
	void cancelImage();
	void addPodcast(const QString& title, const QUrl& url, const QUrl& image, const QString& description, const QString& webPage, QTreeWidgetItem* p);
	void addCategory(const OpmlParser::Category& cat, QTreeWidgetItem* p);
	void addPodcast(const OpmlParser::Podcast& pod, QTreeWidgetItem* p);

private Q_SLOTS:
	void selectionChanged();
	void jobFinished();
	void imageJobFinished();
	void openLink(const QUrl& url);

private:
	void updateText();
	virtual void parseResonse(QIODevice* dev) = 0;

protected:
	QString pageName;
	Spinner* spinner;
	Spinner* imageSpinner;
	QTreeWidget* tree;
	TextBrowser* text;
	NetworkJob* job;
	NetworkJob* imageJob;
	QIcon icn;
};

class PodcastSearchPage : public PodcastPage {
	Q_OBJECT
public:
	PodcastSearchPage(QWidget* p, const QString& n, int s, int i, const QUrl& qu, const QString& qk, const QStringList& other = QStringList());
	~PodcastSearchPage() override {}

	void showEvent(QShowEvent* e) override;

private:
	void parseResonse(QIODevice* dev) override;

private Q_SLOTS:
	virtual void doSearch();
	virtual void parse(const QVariant& data) = 0;

protected:
	LineEdit* search;
	QPushButton* searchButton;
	QString currentSearch;
	QUrl queryUrl;
	QString queryKey;
	QStringList otherArgs;
};

class OpmlBrowsePage : public PodcastPage {
	Q_OBJECT
public:
	OpmlBrowsePage(QWidget* p, const QString& n, const QIcon& i, const QUrl& u);
	~OpmlBrowsePage() override {}

	void showEvent(QShowEvent* e) override;

private Q_SLOTS:
	void reload();

private:
	void parseResonse(QIODevice* dev) override;

private:
	bool loaded;
	QUrl url;
};

class PodcastUrlPage : public PodcastPage {
	Q_OBJECT
public:
	PodcastUrlPage(QWidget* p);
	~PodcastUrlPage() override {}

	void showEvent(QShowEvent* e) override;

private:
	void parseResonse(QIODevice* dev) override;
	void parse(QIODevice* dev);

private Q_SLOTS:
	void loadUrl();
	void openPath();

protected:
	LineEdit* urlEntry;
	QUrl currentUrl;
};

class PodcastSearchDialog : public Dialog {
	Q_OBJECT
public:
	static int instanceCount();
	static QString constCacheDir;
	static QString constExt;

	PodcastSearchDialog(PodcastService* s, QWidget* parent);
	~PodcastSearchDialog() override;

private Q_SLOTS:
	void rssSelected(const QUrl& url);
	void showError(const QString& msg);
	void showInfo(const QString& msg);
	void msgWidgetVisible(bool v);
	void pageChanged();

private:
	QList<PodcastPage*> loadDirectories(const QString& dir, bool isSystem, QSet<QString>& loaded);
	void slotButtonClicked(int button) override;

private:
	QUrl currentUrl;
	PageWidget* pageWidget;
	MessageWidget* messageWidget;
	QWidget* spacer;
	PodcastService* service;
};

#endif
