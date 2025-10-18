#include "ultimatelyricscommandprovider.h"

#include "network/networkaccessmanager.h"

#include <QProcess>
#include <QUrl>
#include <QUrlQuery>
#include <qxmlstream.h>

static bool debugEnabled = false;
#define DBUG \
if (debugEnabled) qWarning() << "CommandLyricProvider" << __FUNCTION__
void UltimateLyricsCommandProvider::enableDebug()
{
	debugEnabled = true;
}

static QString noSpace(const QString& text)
{
	QString ret(text);
	ret.remove(' ');
	return ret;
}

static QString firstChar(const QString& text)
{
	return text.isEmpty() ? text : text[0].toLower();
}

static QString titleCase(const QString& text)
{
	if (0 == text.length()) {
		return QString();
	}
	if (1 == text.length()) {
		return text[0].toUpper();
	}
	return text[0].toUpper() + text.right(text.length() - 1).toLower();
}

QString UltimateLyricsCommandProvider::displayName() const
{
	return name;
}

void UltimateLyricsCommandProvider::fetchInfo(int id, Song metadata, bool removeThe)
{
	auto artistFixed = metadata.basicArtist();
	auto titleFixed = metadata.basicTitle();

	if (removeThe && artistFixed.startsWith(constThe)) {
		artistFixed = artistFixed.mid(constThe.length());
	}

	QStringList replacedArguments = arguments;
	for (auto &argument : replacedArguments) {
		argument = argument.replace(constArtistArg, artistFixed);
		argument = argument.replace(constArtistLowerArg, artistFixed.toLower());
		argument = argument.replace(constArtistLowerNoSpaceArg, noSpace(artistFixed.toLower()));
		argument = argument.replace(constArtistFirstCharArg, firstChar(artistFixed));
		argument = argument.replace(constAlbumArg, metadata.album);
		argument = argument.replace(constAlbumLowerArg, metadata.album.toLower());
		argument = argument.replace(constAlbumLowerNoSpaceArg, noSpace(metadata.album.toLower()));
		argument = argument.replace(constTitleArg, titleFixed);
		argument = argument.replace(constTitleLowerArg, titleFixed.toLower());
		argument = argument.replace(constTitleCaseArg, titleCase(titleFixed));
		argument = argument.replace(constYearArg, QString::number(metadata.year));
		argument = argument.replace(constTrackNoArg, QString::number(metadata.track));
		argument = argument.replace(constDuration, QString::number(metadata.time));
	}

	QProcess *process = new QProcess(this);
	processes.append(process);

	connect(process, &QProcess::finished, this, [this, process, id](int exitCode, QProcess::ExitStatus exitStatus) {
		DBUG << "Process " << executable << "exited with exitCode:" << exitCode << ", and exitStatus:" << exitStatus;

		processes.removeAll(process);
		process->deleteLater();

		auto output = process->readAllStandardOutput();
		emit lyricsReady(id, output);
	});

	connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
		qCritical() << "UltimateLyricsCommandProvider: Starting command failed:" << error << "err:" << process->errorString();

		processes.removeAll(process);
		process->deleteLater();
	});

	DBUG << "Starting " << executable << replacedArguments;
	process->start(executable, replacedArguments);
}

void UltimateLyricsCommandProvider::abort()
{
	for (const auto process : processes) {
		process->deleteLater();
	}
}

UltimateLyricsProvider* UltimateLyricsCommandProvider::parseProvider(QXmlStreamReader* reader)
{
	QXmlStreamAttributes attributes = reader->attributes();

	auto* scraper = new UltimateLyricsCommandProvider;
	scraper->setName(attributes.value("name").toString());
	scraper->setExecutable(attributes.value("executable").toString());

	while (!reader->atEnd()) {
		reader->readNext();

		if (QXmlStreamReader::EndElement == reader->tokenType()) {
			break;
		}

		if (QXmlStreamReader::StartElement == reader->tokenType()) {
			if (QLatin1String("argument") == reader->name()) {
				scraper->addArgument(reader->attributes().value("value").toString());
				reader->skipCurrentElement();
			}
		}
	}
	return scraper;
}

#include "moc_ultimatelyricscommandprovider.cpp"
