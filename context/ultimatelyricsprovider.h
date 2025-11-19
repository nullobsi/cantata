#ifndef LYRICSPROVIDER_H
#define LYRICSPROVIDER_H

#include "mpd-interface/song.h"
#include <QObject>

class NetworkJob;

static const QString constArtistArg = QLatin1String("{Artist}");
static const QString constArtistLowerArg = QLatin1String("{artist}");
static const QString constArtistLowerNoSpaceArg = QLatin1String("{artist2}");
static const QString constArtistFirstCharArg = QLatin1String("{a}");
static const QString constAlbumArg = QLatin1String("{Album}");
static const QString constAlbumLowerArg = QLatin1String("{album}");
static const QString constAlbumLowerNoSpaceArg = QLatin1String("{album2}");
static const QString constTitleLowerArg = QLatin1String("{title}");
static const QString constTitleArg = QLatin1String("{Title}");
static const QString constTitleCaseArg = QLatin1String("{Title2}");
static const QString constYearArg = QLatin1String("{year}");
static const QString constTrackNoArg = QLatin1String("{track}");
static const QString constDuration = QLatin1String("{duration}");
static const QString constThe = QLatin1String("The ");

class UltimateLyricsProvider : public QObject{
	Q_OBJECT

public:
	virtual QString displayName() const { return ""; };
	virtual void fetchInfo(int id, Song metadata, bool removeThe = false) { (void)id; (void)metadata; (void)removeThe; };
	virtual void abort() {};

	void setName(const QString& n) { name = n; }
	QString getName() const { return name; }
	void setRelevance(int r) { relevance = r; }
	int getRelevance() const { return relevance; }
	bool isEnabled() const { return enabled; }
	void setEnabled(bool e) { enabled = e; }

	Q_SIGNALS:
		void lyricsReady(int id, const QString& data);

protected:
	bool enabled;
	QString name;
	int relevance;
};

#endif//LYRICSPROVIDER_H
