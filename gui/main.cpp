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

#include "application.h"
#include "config.h"
#include "db/librarydb.h"
#include "initialsettingswizard.h"
#include "mainwindow.h"
#include "mpd-interface/song.h"
#include "settings.h"
#include "support/thread.h"
#include "support/utils.h"
#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QSettings>
#include <QTranslator>

// To enable debug...
#include "context/lastfmengine.h"
#include "context/metaengine.h"
#include "context/wikipediaengine.h"
#include "covers.h"
#include "mpd-interface/cuefile.h"
#include "mpd-interface/mpdconnection.h"
#include "mpd-interface/mpdparseutils.h"
#include "playlists/dynamicplaylists.h"
#ifdef ENABLE_DEVICES_SUPPORT
#include "models/devicesmodel.h"
#endif
#include "context/contextwidget.h"
#include "context/ultimatelyricsprovider.h"
#include "http/httpserver.h"
#include "network/networkaccessmanager.h"
#include "streams/streamfetcher.h"
#include "tags/taghelperiface.h"
#include "widgets/songdialog.h"
#ifdef ENABLE_SCROBBLING
#include "scrobbling/scrobbler.h"
#endif
#include "gui/mediakeys.h"
#ifdef ENABLE_HTTP_STREAM_PLAYBACK
#include "mpd-interface/httpstream.h"
#endif
#ifdef Avahi_FOUND
#include "avahidiscovery.h"
#endif
#include "customactions.h"

#include <QByteArray>
#include <QCommandLineParser>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <iostream>

static QMutex msgMutex;
static bool firstMsg = true;
static bool debugToFile = false;
static void cantataQtMsgHandler(QtMsgType, const QMessageLogContext&, const QString& msg)
{
	if (debugToFile) {
		QMutexLocker locker(&msgMutex);
		QFile f(Utils::cacheDir(QString(), true) + "cantata.log");
		if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
			QTextStream stream(&f);
			if (firstMsg) {
				stream << "------------START------------" << CANTATA_ENDL;
				firstMsg = false;
			}
			stream << QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ") << " - " << msg << CANTATA_ENDL;
		}
	}
	else {
		std::cout << QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ").toLatin1().constData()
				  << " - " << msg.toLocal8Bit().constData() << std::endl;
	}
}

static void loadTranslation(const QString& prefix, const QString& path, const QString& overrideLanguage = QString())
{
	QString language = overrideLanguage.isEmpty() ? QLocale::system().name() : overrideLanguage;
	QTranslator* t = new QTranslator;
	if (t->load(prefix + "_" + language, path)) {
		QCoreApplication::installTranslator(t);
	}
	else {
		delete t;
	}
}

static void removeOldFiles(const QString& d, const QStringList& types)
{
	if (!d.isEmpty()) {
		QDir dir(d);
		if (dir.exists()) {
			QFileInfoList files = dir.entryInfoList(types, QDir::Files | QDir::NoDotAndDotDot);
			for (const QFileInfo& file : files) {
				QFile::remove(file.absoluteFilePath());
			}
			QString dirName = dir.dirName();
			if (!dirName.isEmpty()) {
				dir.cdUp();
				dir.rmdir(dirName);
			}
		}
	}
}

static void removeOldFiles()
{
	// Remove Cantata 1.x XML cache files
	removeOldFiles(Utils::cacheDir("library"), QStringList() << "*.xml");
}

static QString debugAreas()
{
	return QObject::tr("mpd - MPD communication") + QLatin1Char('\n')
			+ QObject::tr("mpdparse - Parsing of MPD response") + QLatin1Char('\n')
			+ QObject::tr("cue - Cue file parsing") + QLatin1Char('\n')
			+ QObject::tr("covers - Cover fetching, and loading") + QLatin1Char('\n')
			+ QObject::tr("covers-verbose - Cover fetching, and loading (verbose) ") + QLatin1Char('\n')
			+ QObject::tr("context-wikipedia - Wikipedia info in context view") + QLatin1Char('\n')
			+ QObject::tr("context-lastfm - Last.fm info in context view") + QLatin1Char('\n')
			+ QObject::tr("context-widget - General debug in context view") + QLatin1Char('\n')
			+ QObject::tr("context-lyrics - Lyrics in context view") + QLatin1Char('\n')
			+ QObject::tr("dynamic - Dynamic playlists") + QLatin1Char('\n')
			+ QObject::tr("stream-fetcher - Fetching of stream URLs") + QLatin1Char('\n')
			+ QObject::tr("http-server - Built-in HTTP server") + QLatin1Char('\n')
			+ QObject::tr("song-dialog - Song dialogs (tags, replaygain, organiser)") + QLatin1Char('\n')
			+ QObject::tr("network-access - Network access") + QLatin1Char('\n')
			+ QObject::tr("threads - Threads") + QLatin1Char('\n')
#ifdef ENABLE_SCROBBLING
			+ QObject::tr("scrobbler - Scrobbling") + QLatin1Char('\n')
#endif
			+ QObject::tr("sql - SQL access") + QLatin1Char('\n')
			+ QObject::tr("media-keys - Media-keys") + QLatin1Char('\n')
			+ QObject::tr("custom-actions - Custom actions") + QLatin1Char('\n')
#ifdef TagLib_FOUND
			+ QObject::tr("tags - Tag reading/writing") + QLatin1Char('\n')
#endif
#ifdef ENABLE_DEVICES_SUPPORT
			+ QObject::tr("devices - Device support") + QLatin1Char('\n')
#endif
#ifdef ENABLE_HTTP_STREAM_PLAYBACK
			+ QObject::tr("http-stream - Playback of MPD output streams") + QLatin1Char('\n')
#endif
#ifdef Avahi_FOUND
			+ QObject::tr("avahi - Auto-discovery of MPD servers") + QLatin1Char('\n')
#endif
			+ QObject::tr("all - Enable all debug") + QLatin1Char('\n');
}

static void installDebugMessageHandler(const QString& cmdLine)
{
	QStringList items = cmdLine.split(",", CANTATA_SKIP_EMPTY);

	for (const auto& area : items) {
		bool all = QLatin1String("all") == area;
		if (all || QLatin1String("mpd") == area) {
			MPDConnection::enableDebug();
		}
		if (all || QLatin1String("mpdparse") == area) {
			MPDParseUtils::enableDebug();
		}
		if (all || QLatin1String("cue") == area) {
			CueFile::enableDebug();
		}
		if (all || QLatin1String("covers") == area) {
			Covers::enableDebug(false);
		}
		if (all || QLatin1String("covers-verbose") == area) {
			Covers::enableDebug(true);
		}
		if (all || QLatin1String("context-wikipedia") == area) {
			WikipediaEngine::enableDebug();
		}
		if (all || QLatin1String("context-lastfm") == area) {
			LastFmEngine::enableDebug();
		}
		if (all || QLatin1String("context-info") == area) {
			MetaEngine::enableDebug();
		}
		if (all || QLatin1String("context-widget") == area) {
			ContextWidget::enableDebug();
		}
		if (all || QLatin1String("dynamic") == area) {
			DynamicPlaylists::enableDebug();
		}
		if (all || QLatin1String("stream-fetcher") == area) {
			StreamFetcher::enableDebug();
		}
		if (all || QLatin1String("http-server") == area) {
			HttpServer::enableDebug();
		}
		if (all || QLatin1String("song-dialog") == area) {
			SongDialog::enableDebug();
		}
		if (all || QLatin1String("network-access") == area) {
			NetworkAccessManager::enableDebug();
		}
		if (all || QLatin1String("context-lyrics") == area) {
			UltimateLyricsProvider::enableDebug();
		}
		if (all || QLatin1String("threads") == area) {
			ThreadCleaner::enableDebug();
		}
#ifdef ENABLE_SCROBBLING
		if (all || QLatin1String("scrobbler") == area) {
			Scrobbler::enableDebug();
		}
#endif
		if (all || QLatin1String("sql") == area) {
			LibraryDb::enableDebug();
		}
		if (all || QLatin1String("media-keys") == area) {
			MediaKeys::enableDebug();
		}
		if (all || QLatin1String("custom-actions") == area) {
			CustomActions::enableDebug();
		}
#ifdef TagLib_FOUND
		if (all || QLatin1String("tags") == area) {
			TagHelperIface::enableDebug();
		}
#endif
#ifdef ENABLE_DEVICES_SUPPORT
		if (all || QLatin1String("devices") == area) {
			DevicesModel::enableDebug();
		}
#endif
#ifdef ENABLE_HTTP_STREAM_PLAYBACK
		if (all || QLatin1String("http-stream") == area) {
			HttpStream::enableDebug();
		}
#endif
#ifdef Avahi_FOUND
		if (all || QLatin1String("avahi") == area) {
			AvahiDiscovery::enableDebug();
		}
#endif
	}
	qInstallMessageHandler(cantataQtMsgHandler);
}

#if defined Q_OS_LINUX && defined __GNUC__ && defined __GLIBC__
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

static void sigHandler(int /*i*/)
{
	// Adapted from: https://panthema.net/2008/0901-stacktrace-demangled/

	// stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net/
	// published under the WTFPL v2.0
	static const int constMaxFuncNameLen = 256;
	static const int constNumLevels = 15;
	void* addrlist[constNumLevels + 1];

	fprintf(stderr, "Unfortunately Cantata has crashed. Please report a bug at \n"
	                "https://github.com/nullobsi/cantata/issues/ and include the following stack trace:\n\n");
	// retrieve current stack addresses
	int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
	if (!addrlen) {
		fprintf(stderr, "Failed to produce stack trace!\n");
		_exit(0);
	}

	char** symbolList = backtrace_symbols(addrlist, addrlen);
	char funcName[constMaxFuncNameLen];

	// iterate over the returned symbol lines. skip the first, it is the
	// address of this function.
	for (int i = 1; i < addrlen; i++) {
		char* beginName = nullptr;
		char* beginOffset = nullptr;
		char* endOffset = nullptr;

		// find parentheses and +address offset surrounding the mangled name:
		// ./module(function+0x15c) [0x8048a6d]
		for (char* p = symbolList[i]; *p; ++p) {
			if (*p == '(') {
				beginName = p;
			}
			else if (*p == '+') {
				beginOffset = p;
			}
			else if (*p == ')' && beginOffset) {
				endOffset = p;
				break;
			}
		}

		if (beginName && beginOffset && endOffset && beginName < beginOffset) {
			*beginName++ = '\0';
			*beginOffset++ = '\0';
			*endOffset = '\0';

			// mangled name is now in [begin_name, begin_offset) and caller
			// offset in [begin_offset, end_offset). now apply
			// __cxa_demangle():

			int status = 0;
			size_t nameLen = constMaxFuncNameLen;
			char* ret = abi::__cxa_demangle(beginName, funcName, &nameLen, &status);
			if (!status) {
				fprintf(stderr, "  %s : %s+%s\n", symbolList[i], ret, beginOffset);
			}
			else {
				// demangling failed. Output function name as a C function with
				// no arguments.
				fprintf(stderr, "  %s : %s()+%s\n", symbolList[i], beginName, beginOffset);
			}
		}
		else {
			// couldn't parse the line? print the whole line.
			fprintf(stderr, "  %s\n", symbolList[i]);
		}
	}

	free(symbolList);
	_exit(1);
}
#endif

int main(int argc, char* argv[])
{
#if defined Q_OS_LINUX && defined __GNUC__ && defined __GLIBC__
	signal(SIGSEGV, sigHandler);
#endif
	QThread::currentThread()->setObjectName("GUI");
	QCoreApplication::setApplicationName(PACKAGE_NAME);
	QCoreApplication::setOrganizationName(ORGANIZATION_NAME);

	Application app(argc, argv);
	app.setApplicationVersion(PACKAGE_VERSION_STRING);

	QCommandLineParser cmdLineParser;
	cmdLineParser.setApplicationDescription(QObject::tr("MPD Client"));
	cmdLineParser.addHelpOption();
	cmdLineParser.addVersionOption();
	QCommandLineOption debugOption(QStringList() << "d"
	                                             << "debug",
	                               QObject::tr("Comma-separated list of debug areas - possible values:\n") + debugAreas(), "debug", "");
	QCommandLineOption debugToFileOption(QStringList() << "f"
	                                                   << "debug-to-file",
	                                     QObject::tr("Log debug messages to %1").arg(Utils::cacheDir(QString(), true) + "cantata.log"), "", "false");
	QCommandLineOption noNetworkOption(QStringList() << "n"
	                                                 << "no-network",
	                                   QObject::tr("Disable network access"), "", "false");
	QCommandLineOption collectionOption(QStringList() << "c"
	                                                  << "collection",
	                                    QObject::tr("Collection name"), "collection", "");
	QCommandLineOption fullscreenOption(QStringList() << "F"
	                                                  << "fullscreen",
	                                    QObject::tr("Start full screen"), "", "false");
	cmdLineParser.addOption(debugOption);
	cmdLineParser.addOption(debugToFileOption);
	cmdLineParser.addOption(noNetworkOption);
	cmdLineParser.addOption(collectionOption);
	cmdLineParser.addOption(fullscreenOption);
	cmdLineParser.process(app);
	QStringList files = cmdLineParser.positionalArguments();

	if (!app.start(files)) {
		return 0;
	}

	if (cmdLineParser.isSet(noNetworkOption)) {
		NetworkAccessManager::disableNetworkAccess();
	}

// Set the permissions on the config file on Unix - it can contain passwords
// for internet services so it's important that other users can't read it.
// On Windows these are stored in the registry instead.
#ifdef Q_OS_UNIX
	QSettings s;

	// Create the file if it doesn't exist already
	if (!QFile::exists(s.fileName())) {
		QFile file(s.fileName());
		if (!file.open(QIODevice::WriteOnly)) {
			qWarning() << "Failed to open settings file: " << s.fileName();
		}
	}

	// Set -rw-------
	QFile::setPermissions(s.fileName(), QFile::ReadOwner | QFile::WriteOwner);
#endif

	removeOldFiles();
	if (cmdLineParser.isSet(debugOption)) {
		installDebugMessageHandler(cmdLineParser.value(debugOption));
		debugToFile = cmdLineParser.isSet(debugToFileOption);
	}

	// Translations
	QString lang = Settings::self()->lang();
#if defined Q_OS_WIN || defined Q_OS_MAC
	loadTranslation("qt", CANTATA_SYS_TRANS_DIR, lang);
#else
	loadTranslation("qt", QLibraryInfo::path(QLibraryInfo::TranslationsPath), lang);
#endif
	QString local = Utils::fixPath(QCoreApplication::applicationDirPath()) + "translations";
	loadTranslation("cantata", QDir(local).exists() ? local : CANTATA_SYS_TRANS_DIR, lang);

	Application::init();
	// Ensure QColator gets initialised...
	Utils::compare(QString(), QString());

	if (Settings::self()->firstRun()) {
		InitialSettingsWizard wz;
		if (QDialog::Rejected == wz.exec()) {
			return 0;
		}
		Settings::self()->save();
	}
	else if (cmdLineParser.isSet(collectionOption)) {
		QString col = cmdLineParser.value(collectionOption);
		if (!col.isEmpty() && col != Settings::self()->currentConnection()) {
			auto collections = Settings::self()->allConnections();
			for (const auto& c : collections) {
				if (c.name == col) {
					Settings::self()->saveCurrentConnection(col);
					break;
				}
			}
		}
	}
	MainWindow mw;
#if defined Q_OS_WIN || defined Q_OS_MAC
	app.setActivationWindow(&mw);
#endif// !defined Q_OS_MAC
	app.loadFiles(files);

	if (cmdLineParser.isSet(fullscreenOption)) {
		mw.fullScreen();
	}

	return app.exec();
}
