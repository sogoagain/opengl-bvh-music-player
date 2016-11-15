#pragma once

#include <stdio.h>
#include "fmod.hpp"
#include "fmod_errors.h"
#include "OpenFileDialog.h"

#pragma comment(lib, "fmod_vc.lib")

class MusicPlayer {

public:
	FMOD::System	*fmodSystem;
	FMOD::Sound		*fmodSound;
	FMOD::Channel	*fmodChannel;
	FMOD_RESULT		fmodResult;
	unsigned int	fmod_version;
	char			*mStrFilePath;
	char			musicState[128];

	MusicPlayer(void);
	~MusicPlayer(void);

	void FMOD_init(void);
	void FMOD_ERRCHECK(FMOD_RESULT result);
	void FMOD_update(void);
	void FMOD_playSound(void);

	void FMOD_stopMusic(void);
	void FMOD_shutdownSystem(void);
	void FMOD_pausedMusic(void);
	void FMOD_getNowState(void);

	void openMusic(void);
};