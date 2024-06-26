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

#include "actiondialog.h"
#include "devicepropertiesdialog.h"
#include "devicepropertieswidget.h"
#include "encoders.h"
#include "gui/covers.h"
#include "models/mpdlibrarymodel.h"
#include "models/musiclibraryitemalbum.h"
#include "models/musiclibraryitemartist.h"
#include "models/musiclibraryitemroot.h"
#include "models/musiclibraryitemsong.h"
#include "mpd-interface/mpdconnection.h"
#include "mpd-interface/mpdparseutils.h"
#include "support/thread.h"
#include "support/utils.h"
#include "tags/tags.h"
#include "transcodingjob.h"
#include "umsdevice.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTimer>

const QLatin1String FsDevice::constCantataCacheFile("/.cache");
const QLatin1String FsDevice::constCantataSettingsFile("/.cantata");
const QLatin1String FsDevice::constMusicFilenameSchemeKey("music_filenamescheme");
const QLatin1String FsDevice::constVfatSafeKey("vfat_safe");
const QLatin1String FsDevice::constAsciiOnlyKey("ascii_only");
const QLatin1String FsDevice::constIgnoreTheKey("ignore_the");
const QLatin1String FsDevice::constReplaceSpacesKey("replace_spaces");
const QLatin1String FsDevice::constCoverFileNameKey("cover_filename");         // Cantata extension!
const QLatin1String FsDevice::constCoverMaxSizeKey("cover_maxsize");           // Cantata extension!
const QLatin1String FsDevice::constVariousArtistsFixKey("fix_various_artists");// Cantata extension!
const QLatin1String FsDevice::constTranscoderKey("transcoder");                // Cantata extension!
const QLatin1String FsDevice::constUseCacheKey("use_cache");                   // Cantata extension!
const QLatin1String FsDevice::constDefCoverFileName("cover.jpg");
const QLatin1String FsDevice::constAutoScanKey("auto_scan");// Cantata extension!

MusicScanner::MusicScanner(const QString& id)
	: QObject(nullptr), stopRequested(false), count(0)
{
	thread = new Thread(metaObject()->className() + QLatin1String("::") + id);
	moveToThread(thread);
	thread->start();
}

MusicScanner::~MusicScanner()
{
	stop();
}

void MusicScanner::scan(const QString& folder, const QString& cacheFile, bool readCache, const QSet<FileOnlySong>& existingSongs)
{
	if (!cacheFile.isEmpty() && readCache) {
		MusicLibraryItemRoot* lib = new MusicLibraryItemRoot;
		readProgress(0.0);
		if (lib->fromXML(cacheFile, folder)) {
			if (!stopRequested) {
				emit libraryUpdated(lib);
			}
			else {
				delete lib;
			}
			return;
		}
		else {
			delete lib;
		}
	}

	if (stopRequested) {
		return;
	}
	count = 0;
	MusicLibraryItemRoot* library = new MusicLibraryItemRoot;
	QString topLevel = Utils::fixPath(QDir(folder).absolutePath());
	QSet<FileOnlySong> existing = existingSongs;
	timer.start();
	scanFolder(library, topLevel, topLevel, existing, 0);

	if (!stopRequested) {
		if (!cacheFile.isEmpty()) {
			writeProgress(0.0);
			library->toXML(cacheFile, this);
		}
		emit libraryUpdated(library);
	}
	else {
		delete library;
	}
}

void MusicScanner::saveCache(const QString& cache, MusicLibraryItemRoot* lib)
{
	writeProgress(0.0);
	lib->toXML(cache, this);
	emit cacheSaved();
}

void MusicScanner::stop()
{
	stopRequested = true;
	thread->stop();
	thread = nullptr;
}

void MusicScanner::scanFolder(MusicLibraryItemRoot* library, const QString& topLevel, const QString& f,
                              QSet<FileOnlySong>& existing, int level)
{
	if (stopRequested) {
		return;
	}
	if (level < 4) {
		QDir d(f);
		QFileInfoList entries = d.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot);
		MusicLibraryItemArtist* artistItem = nullptr;
		MusicLibraryItemAlbum* albumItem = nullptr;
		for (const QFileInfo& info : entries) {
			if (stopRequested) {
				return;
			}
			if (info.isDir()) {
				scanFolder(library, topLevel, info.absoluteFilePath(), existing, level + 1);
			}
			else if (info.isReadable()) {
				Song song;
				QString fname = info.absoluteFilePath().mid(topLevel.length());

				if (fname.endsWith(".jpg", Qt::CaseInsensitive) || fname.endsWith(".png", Qt::CaseInsensitive) || fname.endsWith(".lyrics", Qt::CaseInsensitive) || fname.endsWith(".pamp", Qt::CaseInsensitive)) {
					continue;
				}
				song.file = fname;
				QSet<FileOnlySong>::iterator it = existing.find(song);
				if (existing.end() == it) {
					song = Tags::read(info.absoluteFilePath());
					song.file = fname;
				}
				else {
					song = *it;
					existing.erase(it);
				}
				if (song.isEmpty()) {
					continue;
				}
				count++;
				if (timer.elapsed() >= 1500 || 0 == (count % 5)) {
					timer.restart();
					emit songCount(count);
				}

				song.fillEmptyFields();
				song.populateSorts();
				song.size = info.size();
				if (!artistItem || song.albumArtistOrComposer() != artistItem->data()) {
					artistItem = library->artist(song);
				}
				if (!albumItem || albumItem->parentItem() != artistItem || song.albumName() != albumItem->data()) {
					albumItem = artistItem->album(song);
				}
				albumItem->append(new MusicLibraryItemSong(song, albumItem));
			}
		}
	}
}

void MusicScanner::readProgress(double pc)
{
	emit readingCache(pc);
}

void MusicScanner::writeProgress(double pc)
{
	emit savingCache(pc);
}

bool FsDevice::readOpts(const QString& fileName, DeviceOptions& opts, bool readAll)
{
	QFile file(fileName);

	opts = DeviceOptions(constDefCoverFileName);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&file);
		while (!in.atEnd()) {
			QString line = in.readLine();
			if (line.startsWith(constCoverFileNameKey + "=")) {
				opts.coverName = line.section('=', 1, 1);
			}
			if (line.startsWith(constCoverMaxSizeKey + "=")) {
				opts.coverMaxSize = line.section('=', 1, 1).toUInt();
				opts.checkCoverSize();
			}
			else if (line.startsWith(constVariousArtistsFixKey + "=")) {
				opts.fixVariousArtists = QLatin1String("true") == line.section('=', 1, 1);
			}
			else if (line.startsWith(constTranscoderKey + "=")) {
				QStringList parts = line.section('=', 1, 1).split(',');
				if (parts.size() >= 3) {
					opts.transcoderCodec = parts.at(0);
					opts.transcoderValue = parts.at(1).toInt();
					if (parts.size() >= 4) {
						if (QLatin1String("true") == parts.at(3)) {
							opts.transcoderWhen = DeviceOptions::TW_IfLossess;
						}
						else if (QLatin1String("true") == parts.at(2)) {
							opts.transcoderWhen = DeviceOptions::TW_IfDifferent;
						}
						else {
							opts.transcoderWhen = DeviceOptions::TW_Always;
						}
					}
					else {
						const QString& val = parts.at(2);
						if (QLatin1String("true") == val) {
							opts.transcoderWhen = DeviceOptions::TW_IfDifferent;
						}
						else if (QLatin1String("false") == val) {
							opts.transcoderWhen = DeviceOptions::TW_Always;
						}
						else {
							opts.transcoderWhen = (DeviceOptions::TranscodeWhen)val.toInt();
						}
					}
				}
			}
			else if (line.startsWith(constUseCacheKey + "=")) {
				opts.useCache = QLatin1String("true") == line.section('=', 1, 1);
			}
			else if (line.startsWith(constAutoScanKey + "=")) {
				opts.autoScan = QLatin1String("true") == line.section('=', 1, 1);
			}
			else if (readAll) {
				// For UMS these are stored in .is_audio_player - for Amarok compatability!
				if (line.startsWith(constMusicFilenameSchemeKey + "=")) {
					QString scheme = line.section('=', 1, 1);
					//protect against empty setting.
					if (!scheme.isEmpty()) {
						opts.scheme = scheme;
					}
				}
				else if (line.startsWith(constVfatSafeKey + "=")) {
					opts.vfatSafe = QLatin1String("true") == line.section('=', 1, 1);
				}
				else if (line.startsWith(constAsciiOnlyKey + "=")) {
					opts.asciiOnly = QLatin1String("true") == line.section('=', 1, 1);
				}
				else if (line.startsWith(constIgnoreTheKey + "=")) {
					opts.ignoreThe = QLatin1String("true") == line.section('=', 1, 1);
				}
				else if (line.startsWith(constReplaceSpacesKey + "=")) {
					opts.replaceSpaces = QLatin1String("true") == line.section('=', 1, 1);
				}
			}
		}

		return true;
	}
	return false;
}

static inline QString toString(bool b)
{
	return b ? QLatin1String("true") : QLatin1String("false");
}

void FsDevice::writeOpts(const QString& fileName, const DeviceOptions& opts, bool writeAll)
{
	DeviceOptions def(constDefCoverFileName);
	// If we are just using the defaults, then mayas wel lremove the file!
	if ((writeAll && opts == def) || (!writeAll && opts.coverName == constDefCoverFileName && 0 == opts.coverMaxSize && opts.fixVariousArtists != def.fixVariousArtists && opts.transcoderCodec.isEmpty() && opts.useCache == def.useCache && opts.autoScan != def.autoScan)) {
		if (QFile::exists(fileName)) {
			QFile::remove(fileName);
		}
		return;
	}

	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {

		QTextStream out(&file);
		if (writeAll) {
			if (opts.scheme != def.scheme) {
				out << constMusicFilenameSchemeKey << '=' << opts.scheme << '\n';
			}
			if (opts.vfatSafe != def.vfatSafe) {
				out << constVfatSafeKey << '=' << toString(opts.vfatSafe) << '\n';
			}
			if (opts.asciiOnly != def.asciiOnly) {
				out << constAsciiOnlyKey << '=' << toString(opts.asciiOnly) << '\n';
			}
			if (opts.ignoreThe != def.ignoreThe) {
				out << constIgnoreTheKey << '=' << toString(opts.ignoreThe) << '\n';
			}
			if (opts.replaceSpaces != def.replaceSpaces) {
				out << constReplaceSpacesKey << '=' << toString(opts.replaceSpaces) << '\n';
			}
		}

		// NOTE: If any options are added/changed - take care of the "if ( (writeAll..." block above!!!
		if (opts.coverName != constDefCoverFileName) {
			out << constCoverFileNameKey << '=' << opts.coverName << '\n';
		}
		if (0 != opts.coverMaxSize) {
			out << constCoverMaxSizeKey << '=' << opts.coverMaxSize << '\n';
		}
		if (opts.fixVariousArtists != def.fixVariousArtists) {
			out << constVariousArtistsFixKey << '=' << toString(opts.fixVariousArtists) << '\n';
		}
		if (!opts.transcoderCodec.isEmpty()) {
			out << constTranscoderKey << '=' << opts.transcoderCodec << ',' << opts.transcoderValue
				<< ',' << opts.transcoderWhen << '\n';
		}
		if (opts.useCache != def.useCache) {
			out << constUseCacheKey << '=' << toString(opts.useCache) << '\n';
		}
		if (opts.autoScan != def.autoScan) {
			out << constAutoScanKey << '=' << toString(opts.autoScan) << '\n';
		}
	}
}

FsDevice::FsDevice(MusicLibraryModel* m, Solid::Device& dev)
	: Device(m, dev), state(Idle), scanned(false), cacheProgress(-1), scanner(nullptr)
{
}

FsDevice::FsDevice(MusicLibraryModel* m, const QString& name, const QString& id)
	: Device(m, name, id), state(Idle), scanned(false), cacheProgress(-1), scanner(nullptr)
{
}

FsDevice::~FsDevice()
{
	stopScanner();
}

void FsDevice::rescan(bool full)
{
	spaceInfo.setDirty();
	// If this is the first scan (scanned=false) and we are set to use cache, attempt to load that before scanning
	if (isIdle()) {
		if (full) {
			removeCache();
			clear();
		}
		startScanner(full);
		scanned = true;
	}
}

void FsDevice::stop()
{
	if (nullptr != scanner) {
		stopScanner();
	}
}

void FsDevice::addSong(const Song& s, bool overwrite, bool copyCover)
{
	jobAbortRequested = false;
	if (!isConnected()) {
		emit actionStatus(NotConnected);
		return;
	}

	needToFixVa = opts.fixVariousArtists && s.isVariousArtists();

	if (!overwrite) {
		Song check = s;

		if (needToFixVa) {
			Device::fixVariousArtists(QString(), check, true);
		}
		if (songExists(check)) {
			emit actionStatus(SongExists);
			return;
		}
	}

	if (!QFile::exists(s.file)) {
		emit actionStatus(SourceFileDoesNotExist);
		return;
	}

	currentDestFile = audioFolder + opts.createFilename(s);
	Encoders::Encoder encoder;

	transcoding = false;
	if (!opts.transcoderCodec.isEmpty()) {
		encoder = Encoders::getEncoder(opts.transcoderCodec);
		if (encoder.codec.isEmpty()) {
			emit actionStatus(CodecNotAvailable);
			return;
		}

		transcoding = !opts.transcoderCodec.isEmpty() && (DeviceOptions::TW_IfDifferent != opts.transcoderWhen || encoder.isDifferent(s.file)) && (DeviceOptions::TW_IfLossess != opts.transcoderWhen || Device::isLossless(s.file));

		if (transcoding) {
			currentDestFile = encoder.changeExtension(currentDestFile);
		}
	}

	if (!overwrite && QFile::exists(currentDestFile)) {
		emit actionStatus(FileExists);
		return;
	}

	QDir dir(Utils::getDir(currentDestFile));
	if (!dir.exists() && !Utils::createWorldReadableDir(dir.absolutePath(), QString())) {
		emit actionStatus(DirCreationFaild);
		return;
	}
	currentSong = s;

	if (transcoding) {
		TranscodingJob* job = new TranscodingJob(encoder, opts.transcoderValue, s.file, currentDestFile, copyCover ? opts : DeviceOptions(Device::constNoCover),
		                                         (needToFixVa ? CopyJob::OptsApplyVaFix : CopyJob::OptsNone) | (Device::RemoteFs == devType() ? CopyJob::OptsFixLocal : CopyJob::OptsNone),
		                                         currentSong);
		connect(job, SIGNAL(result(int)), SLOT(addSongResult(int)));
		connect(job, SIGNAL(percent(int)), SLOT(percent(int)));
		job->start();
	}
	else {
		CopyJob* job = new CopyJob(s.file, currentDestFile, copyCover ? opts : DeviceOptions(Device::constNoCover),
		                           (needToFixVa ? CopyJob::OptsApplyVaFix : CopyJob::OptsNone) | (Device::RemoteFs == devType() ? CopyJob::OptsFixLocal : CopyJob::OptsNone),
		                           currentSong);
		connect(job, SIGNAL(result(int)), SLOT(addSongResult(int)));
		connect(job, SIGNAL(percent(int)), SLOT(percent(int)));
		job->start();
	}
}

void FsDevice::copySongTo(const Song& s, const QString& musicPath, bool overwrite, bool copyCover)
{
	jobAbortRequested = false;
	if (!isConnected()) {
		emit actionStatus(NotConnected);
		return;
	}

	needToFixVa = opts.fixVariousArtists && s.isVariousArtists();

	if (!overwrite) {
		Song check = s;

		if (needToFixVa) {
			Device::fixVariousArtists(QString(), check, false);
		}
		if (MpdLibraryModel::self()->songExists(check)) {
			emit actionStatus(SongExists);
			return;
		}
	}

	QString source = audioFolder + s.file;

	if (!QFile::exists(source)) {
		emit actionStatus(SourceFileDoesNotExist);
		return;
	}

	QString baseDir = MPDConnection::self()->getDetails().dir;
	if (!overwrite && QFile::exists(baseDir + musicPath)) {
		emit actionStatus(FileExists);
		return;
	}

	currentDestFile = baseDir + musicPath;
	QDir dir(Utils::getDir(currentDestFile));
	if (!dir.exists() && !Utils::createWorldReadableDir(dir.absolutePath(), baseDir)) {
		emit actionStatus(DirCreationFaild);
		return;
	}

	currentSong = s;
	// Pass an empty filename as covername, so that Covers::copyCover knows this is TO MPD...
	CopyJob* job = new CopyJob(source, currentDestFile, copyCover ? DeviceOptions(QString()) : DeviceOptions(Device::constNoCover),
	                           needToFixVa ? CopyJob::OptsUnApplyVaFix : CopyJob::OptsNone, currentSong);
	connect(job, SIGNAL(result(int)), SLOT(copySongToResult(int)));
	connect(job, SIGNAL(percent(int)), SLOT(percent(int)));
	job->start();
}

void FsDevice::removeSong(const Song& s)
{
	jobAbortRequested = false;
	if (!isConnected()) {
		emit actionStatus(NotConnected);
		return;
	}

	if (!QFile::exists(audioFolder + s.file)) {
		emit actionStatus(SourceFileDoesNotExist);
		return;
	}

	currentSong = s;
	DeleteJob* job = new DeleteJob(audioFolder + s.file);
	connect(job, SIGNAL(result(int)), SLOT(removeSongResult(int)));
	job->start();
}

void FsDevice::cleanDirs(const QSet<QString>& dirs)
{
	CleanJob* job = new CleanJob(dirs, audioFolder, opts.coverName);
	connect(job, SIGNAL(result(int)), SLOT(cleanDirsResult(int)));
	connect(job, SIGNAL(percent(int)), SLOT(percent(int)));
	job->start();
}

Covers::Image FsDevice::requestCover(const Song& s)
{
	Covers::Image i;
	QString songFile = audioFolder + s.file;
	QString dirName = Utils::getDir(songFile);

	if (QFile::exists(dirName + opts.coverName)) {
		QImage img(dirName + opts.coverName);
		if (!img.isNull()) {
			emit cover(s, img);
			return Covers::Image(img, dirName + opts.coverName);
		}
	}

	QStringList files = QDir(dirName).entryList(QStringList() << QLatin1String("*.jpg") << QLatin1String("*.png"), QDir::Files | QDir::Readable);
	for (const QString& fileName : files) {
		QImage img(dirName + fileName);

		if (!img.isNull()) {
			emit cover(s, img);
			return Covers::Image(img, dirName + fileName);
		}
	}
	return Covers::Image();
}

void FsDevice::percent(int pc)
{
	if (jobAbortRequested && 100 != pc) {
		FileJob* job = qobject_cast<FileJob*>(sender());
		if (job) {
			job->stop();
		}
		return;
	}
	emit progress(pc);
}

void FsDevice::addSongResult(int status)
{
	CopyJob* job = qobject_cast<CopyJob*>(sender());
	FileJob::finished(job);
	spaceInfo.setDirty();

	if (jobAbortRequested) {
		if (job && job->wasStarted() && QFile::exists(currentDestFile)) {
			QFile::remove(currentDestFile);
		}
		return;
	}
	if (Ok != status) {
		emit actionStatus(status);
	}
	else {
		currentSong.file = currentDestFile.mid(audioFolder.length());
		if (needToFixVa) {
			currentSong.fixVariousArtists();
		}
		addSongToList(currentSong);
		emit actionStatus(Ok, job && job->coverCopied());
	}
}

void FsDevice::copySongToResult(int status)
{
	CopyJob* job = qobject_cast<CopyJob*>(sender());
	FileJob::finished(job);
	spaceInfo.setDirty();
	if (jobAbortRequested) {
		if (job && job->wasStarted() && QFile::exists(currentDestFile)) {
			QFile::remove(currentDestFile);
		}
		return;
	}
	if (Ok != status) {
		emit actionStatus(status);
	}
	else {
		currentSong.file = currentDestFile.mid(MPDConnection::self()->getDetails().dir.length());
		QString origPath;
		if (MPDConnection::self()->isMopidy()) {
			origPath = currentSong.file;
			currentSong.file = Song::encodePath(currentSong.file);
		}
		if (needToFixVa) {
			currentSong.revertVariousArtists();
		}
		Utils::setFilePerms(currentDestFile);
		//        MusicLibraryModel::self()->addSongToList(currentSong);
		//        DirViewModel::self()->addFileToList(origPath.isEmpty() ? currentSong.file : origPath,
		//                                            origPath.isEmpty() ? QString() : currentSong.file);
		emit actionStatus(Ok, job && job->coverCopied());
	}
}

void FsDevice::removeSongResult(int status)
{
	FileJob::finished(sender());
	spaceInfo.setDirty();
	if (jobAbortRequested) {
		return;
	}
	if (Ok != status) {
		emit actionStatus(status);
	}
	else {
		removeSongFromList(currentSong);
		emit actionStatus(Ok);
	}
}

void FsDevice::cleanDirsResult(int status)
{
	FileJob::finished(sender());
	spaceInfo.setDirty();
	if (jobAbortRequested) {
		return;
	}
	emit actionStatus(status);
}

void FsDevice::initScaner()
{
	if (!scanner) {
		static bool registeredTypes = false;

		if (!registeredTypes) {
			qRegisterMetaType<QSet<FileOnlySong>>("QSet<FileOnlySong>");
			registeredTypes = true;
		}
		scanner = new MusicScanner(data());
		connect(scanner, SIGNAL(libraryUpdated(MusicLibraryItemRoot*)), this, SLOT(libraryUpdated(MusicLibraryItemRoot*)));
		connect(scanner, SIGNAL(songCount(int)), this, SLOT(songCount(int)));
		connect(scanner, SIGNAL(cacheSaved()), this, SLOT(savedCache()));
		connect(scanner, SIGNAL(savingCache(int)), this, SLOT(savingCache(int)));
		connect(scanner, SIGNAL(readingCache(int)), this, SLOT(readingCache(int)));
		connect(this, SIGNAL(scan(const QString&, const QString&, bool, const QSet<FileOnlySong>&)), scanner, SLOT(scan(const QString&, const QString&, bool, const QSet<FileOnlySong>&)));
		connect(this, SIGNAL(saveCache(const QString&, MusicLibraryItemRoot*)), scanner, SLOT(saveCache(const QString&, MusicLibraryItemRoot*)));
	}
}

void FsDevice::startScanner(bool fullScan)
{
	stopScanner();
	initScaner();
	QSet<FileOnlySong> existingSongs;
	if (!fullScan) {
		QSet<Song> songs = allSongs();

		for (const Song& s : songs) {
			existingSongs.insert(FileOnlySong(s));
		}
	}
	state = Updating;
	emit scan(audioFolder, opts.useCache ? cacheFileName() : QString(), !scanned, existingSongs);
	setStatusMessage(tr("Updating..."));
	emit updating(id(), true);
}

void FsDevice::stopScanner()
{
	state = Idle;
	if (!scanner) {
		return;
	}
	disconnect(scanner, SIGNAL(libraryUpdated(MusicLibraryItemRoot*)), this, SLOT(libraryUpdated(MusicLibraryItemRoot*)));
	disconnect(scanner, SIGNAL(songCount(int)), this, SLOT(songCount(int)));
	disconnect(scanner, SIGNAL(cacheSaved()), this, SLOT(savedCache()));
	disconnect(scanner, SIGNAL(savingCache(int)), this, SLOT(savingCache(int)));
	disconnect(scanner, SIGNAL(readingCache(int)), this, SLOT(readingCache(int)));
	scanner->deleteLater();
	scanner = nullptr;
}

void FsDevice::clear() const
{
	if (childCount()) {
		FsDevice* that = const_cast<FsDevice*>(this);
		that->update = new MusicLibraryItemRoot();
		that->applyUpdate();
		that->scanned = false;
	}
}

void FsDevice::libraryUpdated(MusicLibraryItemRoot* lib)
{
	cacheProgress = -1;
	if (update) {
		delete update;
	}
	update = lib;
	setStatusMessage(QString());
	state = Idle;
	emit updating(id(), false);
}

QString FsDevice::cacheFileName() const
{
	if (audioFolder.isEmpty()) {
		setAudioFolder();
	}
	return audioFolder + constCantataCacheFile + ".xml.gz";
}

void FsDevice::saveCache()
{
	if (opts.useCache) {
		state = SavingCache;
		initScaner();
		emit saveCache(cacheFileName(), this);
	}
}

void FsDevice::savedCache()
{
	state = Idle;
	cacheProgress = -1;
	setStatusMessage(QString());
	emit cacheSaved();
}

void FsDevice::removeCache()
{
	QString cacheFile(cacheFileName());
	if (QFile::exists(cacheFile)) {
		QFile::remove(cacheFile);
	}
}

void FsDevice::readingCache(int pc)
{
	cacheStatus(tr("Reading cache"), pc);
}

void FsDevice::savingCache(int pc)
{
	cacheStatus(tr("Saving cache"), pc);
}

void FsDevice::cacheStatus(const QString& msg, int prog)
{
	if (prog != cacheProgress) {
		cacheProgress = prog;
		setStatusMessage(tr("%1 %2%", "Message percent").arg(msg).arg(cacheProgress));
	}
}

#include "moc_fsdevice.cpp"
