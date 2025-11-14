/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
 *
 */
/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ultimatelyrics.h"

#include "gui/settings.h"
#include "lyrics_providers/lrcliblyricsprovider.h"
#include "support/globalstatic.h"
#include "ultimatelyricscommandprovider.h"
#include "ultimatelyricshttpprovider.h"
#include "ultimatelyricsprovider.h"
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QSet>
#include <QXmlStreamReader>
#include <algorithm>
#include <unistd.h>

GLOBAL_STATIC(UltimateLyrics, instance)

static bool compareLyricProviders(const UltimateLyricsProvider* a, const UltimateLyricsProvider* b)
{
	return a->getRelevance() < b->getRelevance();
}

void UltimateLyrics::release()
{
	for (UltimateLyricsProvider* provider : providers) {
		delete provider;
	}
	providers.clear();
}

const QList<UltimateLyricsProvider*> UltimateLyrics::getProviders()
{
	load();
	return providers;
}

UltimateLyricsProvider* UltimateLyrics::providerByName(const QString& name) const
{
	for (UltimateLyricsProvider* provider : providers) {
		if (provider->getName() == name) {
			return provider;
		}
	}
	return nullptr;
}

UltimateLyricsProvider* UltimateLyrics::getNext(int& index)
{
	load();
	index++;
	if (index > -1 && index < providers.count()) {
		for (int i = index; i < providers.count(); ++i) {
			if (providers.at(i)->isEnabled()) {
				index = i;
				return providers.at(i);
			}
		}
	}
	return nullptr;
}

void UltimateLyrics::registerExtra(UltimateLyricsProvider* provider)
{
	providers << provider;
	connect(provider, &UltimateLyricsProvider::lyricsReady, this, &UltimateLyrics::lyricsReady);
}

void UltimateLyrics::load()
{
	if (!providers.isEmpty()) {
		return;
	}

	QStringList files;
	QString userDir = Utils::dataDir();

	if (!userDir.isEmpty()) {
		QFileInfoList files = QDir(userDir).entryInfoList(QStringList() << QLatin1String("lyrics_*.xml"), QDir::NoDotAndDotDot | QDir::Files);
		for (const QFileInfo& f : files) {
			files.append(QFileInfo(f.absoluteFilePath()));
		}
	}

	files.append(":lyrics_providers.xml");

	QSet<QString> providerNames;

	for (const auto& f : files) {
		QFile file(f);
		if (file.open(QIODevice::ReadOnly)) {
			QXmlStreamReader reader(&file);
			while (!reader.atEnd()) {
				reader.readNext();

				if (QLatin1String("provider") == reader.name()) {
					auto attributes = reader.attributes();
					QString name = attributes.value("name").toString();
					QString type = attributes.value("type").toString();

					if (!providerNames.contains(name)) {
						UltimateLyricsProvider* provider;

						if (type == QString("http")) {
							provider = UltimateLyricsHttpProvider::parseProvider(&reader);
						} else if (type == QString("command")) {
							provider = UltimateLyricsCommandProvider::parseProvider(&reader);
						} else {
							// TODO: Throw exception
							__builtin_unreachable();
						}

						if (provider) {
							providers << provider;
							connect(provider, SIGNAL(lyricsReady(int, QString)), this, SIGNAL(lyricsReady(int, QString)));
							providerNames.insert(name);
						}
					}
				}
			}
		}
	}

	// Register extra providers
	registerExtra(new LRCLibLyricsProvider());

	setEnabled(Settings::self()->lyricProviders());
}

void UltimateLyrics::setEnabled(const QStringList& enabled)
{
	for (UltimateLyricsProvider* provider : providers) {
		provider->setEnabled(false);
		provider->setRelevance(0xFFFF);
	}

	int relevance = 0;
	for (const QString& p : enabled) {
		UltimateLyricsProvider* provider = providerByName(p);
		if (provider) {
			provider->setEnabled(true);
			provider->setRelevance(relevance++);
		}
	}
	std::sort(providers.begin(), providers.end(), compareLyricProviders);
	Settings::self()->saveLyricProviders(enabled);
}

#include "moc_ultimatelyrics.cpp"
