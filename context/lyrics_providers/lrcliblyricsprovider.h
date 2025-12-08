#ifndef CANTATA_LRCLIBLYRICSPROVIDER_H
#define CANTATA_LRCLIBLYRICSPROVIDER_H
#include "context/ultimatelyricsprovider.h"

class LRCLibLyricsProvider : public UltimateLyricsProvider {
public:
	static void enableDebug();

	QString displayName() const override { return "LRCLib.net"; }
	void fetchInfo(int id, Song metadata, bool removeThe) override;
	void abort() override;
	~LRCLibLyricsProvider() override;

private Q_SLOTS:
	void lyricsFetched();

private:
	QHash<NetworkJob*, int> requests;
};

#endif//CANTATA_LRCLIBLYRICSPROVIDER_H
