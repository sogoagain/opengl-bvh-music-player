#include "MusicPlayer.h"

MusicPlayer::MusicPlayer(void) {
	fmodSystem = NULL;
	fmodSound = NULL;
	fmodChannel = 0;
	fmodResult = FMOD_OK;
	fmod_version = 0;
	mStrFilePath = (char*)calloc(MAX_PATH, sizeof(char));
}

MusicPlayer::~MusicPlayer(void) {
	this->FMOD_shutdownSystem();
	free(mStrFilePath);
}

void MusicPlayer::FMOD_init(void) {
	/*
	Create a System object and initialize.
	*/
	fmodResult = FMOD::System_Create(&fmodSystem);
	FMOD_ERRCHECK(fmodResult);

	fmodResult = fmodSystem->getVersion(&fmod_version);
	FMOD_ERRCHECK(fmodResult);

	if (fmod_version < FMOD_VERSION) {
		printf("FMOD lib fmod_version %08x doesn't match header fmod_version %08x", fmod_version, FMOD_VERSION);
	}

	fmodResult = fmodSystem->init(32, FMOD_INIT_NORMAL, NULL);
	FMOD_ERRCHECK(fmodResult);
}

void MusicPlayer::FMOD_ERRCHECK(FMOD_RESULT result) {
	if (result != FMOD_OK) {
		printf("fmod error\n");
	}
}

void MusicPlayer::FMOD_update(void) {
	fmodResult = fmodSystem->update();
	FMOD_ERRCHECK(fmodResult);
}

void MusicPlayer::FMOD_playSound(void) {
	fmodResult = fmodSystem->createStream(mStrFilePath, FMOD_LOOP_NORMAL | FMOD_2D, 0, &fmodSound);
	FMOD_ERRCHECK(fmodResult);

	/*
	Play the sound.
	*/
	bool isPlaying = false;

	if (fmodChannel) {
		fmodResult = fmodChannel->isPlaying(&isPlaying);
		FMOD_ERRCHECK(fmodResult);
	}

	if (isPlaying) {
		fmodResult = fmodChannel->stop();
		FMOD_ERRCHECK(fmodResult);
	}

	fmodResult = fmodSystem->playSound(fmodSound, 0, false, &fmodChannel);
	FMOD_ERRCHECK(fmodResult);
}

void MusicPlayer::FMOD_stopMusic(void) {
	bool isPlaying = false;

	if (fmodChannel) {
		fmodResult = fmodChannel->isPlaying(&isPlaying);
		FMOD_ERRCHECK(fmodResult);
	}

	if (isPlaying) {
		fmodResult = fmodChannel->stop();
		FMOD_ERRCHECK(fmodResult);
	}
	else {
		fmodResult = fmodSystem->playSound(fmodSound, 0, false, &fmodChannel);
		FMOD_ERRCHECK(fmodResult);
	}
}

void MusicPlayer::FMOD_shutdownSystem(void) {
	/*
	Shut down
	*/
	fmodResult = fmodSound->release();  /* Release the parent, not the sound that was retrieved with getSubSound. */
	FMOD_ERRCHECK(fmodResult);
	fmodResult = fmodSystem->close();
	FMOD_ERRCHECK(fmodResult);
	fmodResult = fmodSystem->release();
	FMOD_ERRCHECK(fmodResult);
}

void MusicPlayer::FMOD_pausedMusic(void) {
	bool paused;
	fmodResult = fmodChannel->getPaused(&paused);
	FMOD_ERRCHECK(fmodResult);
	fmodResult = fmodChannel->setPaused(!paused);
	FMOD_ERRCHECK(fmodResult);
}

void MusicPlayer::FMOD_getNowState(void) {
	unsigned int	ms = 0;
	unsigned int	lenms = 0;
	bool			playing = false;
	bool			paused = false;

	if (fmodChannel) {
		fmodResult = fmodChannel->isPlaying(&playing);
		if ((fmodResult != FMOD_OK) && (fmodResult != FMOD_ERR_INVALID_HANDLE))
			FMOD_ERRCHECK(fmodResult);

		fmodResult = fmodChannel->getPaused(&paused);
		if ((fmodResult != FMOD_OK) && (fmodResult != FMOD_ERR_INVALID_HANDLE))
			FMOD_ERRCHECK(fmodResult);

		fmodResult = fmodChannel->getPosition(&ms, FMOD_TIMEUNIT_MS);
		if ((fmodResult != FMOD_OK) && (fmodResult != FMOD_ERR_INVALID_HANDLE))
			FMOD_ERRCHECK(fmodResult);

		fmodResult = fmodSound->getLength(&lenms, FMOD_TIMEUNIT_MS);
		if ((fmodResult != FMOD_OK) && (fmodResult != FMOD_ERR_INVALID_HANDLE))
			FMOD_ERRCHECK(fmodResult);
	}

	sprintf_s(musicState, "Time %02d:%02d:%02d/%02d:%02d:%02d : %s", ms / 1000 / 60, ms / 1000 % 60, ms / 10 % 100, lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100, paused ? "Paused " : playing ? "Playing" : "Stopped");
}

void MusicPlayer::openMusic(void) {
	OpenFileDialog* fileDialog = new OpenFileDialog();
	fileDialog->InitialDir = _T("C:\\Users\\");
	fileDialog->Title = _T("Open Music File");
	fileDialog->Filter = _T("mp3 음악 파일 (*.mp3)\0*.mp3\0");

	if (fileDialog->ShowDialog()) {
		WideCharToMultiByte(CP_ACP, 0, fileDialog->FileName, MAX_PATH, mStrFilePath, MAX_PATH, NULL, NULL);
#ifdef DEBUG	
		MessageBox(0, fileDialog->FileName, _T("선택된 음악 파일"),
			MB_OK | MB_ICONINFORMATION);
		printf("선택된 음악 파일: %s\n", filePath);
#endif	
		this->FMOD_playSound();
	}

	delete fileDialog;
}