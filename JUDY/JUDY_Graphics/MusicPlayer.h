#pragma once

#include <stdio.h>
#include "fmod.hpp"
#include "fmod_errors.h"
#include "OpenFileDialog.h"

#pragma comment(lib, "fmod_vc.lib")

using namespace FMOD;

class MusicPlayer {

private:
	FMOD::System	*mFmodSystem;
	FMOD::Sound		*mFmodSound;
	FMOD::Channel	*mFmodChannel;
	FMOD_RESULT		mFmodResult;
	unsigned int	mFmodVersion;
	FMOD_TAG		mFmodTag;

	float			mFVolume;
	char			*mStrFilePath;
	char			mStrMusicState[128];
	char			mStrTitle[128];
	char			mStrArtist[128];
	char			mStrAlbum[128];
	char			mStrGenre[64];

	bool errorCheck(FMOD_RESULT result);
	void playSound(void);
	void shutdownSystem(void);
	void setVolume(void);
	void readMusicTag(void);

public:
	MusicPlayer(void);
	~MusicPlayer(void);

	void init(void);
	void updateSystem(void);
	void stopMusic(void);
	void pausedMusic(void);
	void increaseVolume(bool);
	char* getStrMusicState(void);
	bool openMusic(void);

	char* getTitle(void);
	char* getArtist(void);
	char* getAlbum(void);
	char* getGenre(void);
};