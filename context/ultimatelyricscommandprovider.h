#ifndef ULTIMATELYRICSCOMMANDPROVIDER_H
#define ULTIMATELYRICSCOMMANDPROVIDER_H

#include "ultimatelyricsprovider.h"
#include <QObject>
#include <QProcess>
#include <qxmlstream.h>

class QXmlStreamReader;

class UltimateLyricsCommandProvider : public UltimateLyricsProvider {
	Q_OBJECT

public:
	static void enableDebug();

	QString displayName() const override;
	void fetchInfo(int id, Song metadata, bool removeThe = false) override;
	void abort() override;

	void setExecutable(const QString& c) { executable = c; }

	void addArgument(QString value) { arguments.append(value); };
	static UltimateLyricsProvider* parseProvider(QXmlStreamReader* reader);

private:
	QString executable;
	QStringList arguments;
	QList<QProcess*> processes;
};

#endif//ULTIMATELYRICSCOMMANDPROVIDER_H
