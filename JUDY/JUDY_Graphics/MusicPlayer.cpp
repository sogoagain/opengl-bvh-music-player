#include "MusicPlayer.h"

MusicPlayer::MusicPlayer(void) {
	mFmodSystem = NULL;
	mFmodSound = NULL;
	mFmodChannel = 0;
	mFmodResult = FMOD_OK;
	mFmodVersion = 0;
	mStrFilePath = (char*)calloc(MAX_PATH, sizeof(char));
	mFVolume = 0.5;
}

MusicPlayer::~MusicPlayer(void) {
	this->shutdownSystem();
	free(mStrFilePath);
}

bool MusicPlayer::errorCheck(FMOD_RESULT result) {
	if (result != FMOD_OK) {
		printf("FMOD error %d - %s\n", result, FMOD_ErrorString(result));
		return false;
	}
	return true;
}

void MusicPlayer::playSound(void) {
	mFmodResult = mFmodSystem->createStream(mStrFilePath, FMOD_LOOP_NORMAL | FMOD_2D, 0, &mFmodSound);
	errorCheck(mFmodResult);

	/*
	Play the sound.
	*/
	bool isPlaying = false;

	if (mFmodChannel) {
		mFmodResult = mFmodChannel->isPlaying(&isPlaying);
		errorCheck(mFmodResult);
	}

	if (isPlaying) {
		mFmodResult = mFmodChannel->stop();
		errorCheck(mFmodResult);
	}

	mFmodResult = mFmodSystem->playSound(mFmodSound, 0, false, &mFmodChannel);
	errorCheck(mFmodResult);
}

void MusicPlayer::shutdownSystem(void) {
	/*
	Shut down
	*/
	mFmodResult = mFmodSound->release();  /* Release the parent, not the sound that was retrieved with getSubSound. */
	errorCheck(mFmodResult);
	mFmodResult = mFmodSystem->close();
	errorCheck(mFmodResult);
	mFmodResult = mFmodSystem->release();
	errorCheck(mFmodResult);
}

void MusicPlayer::setVolume(void) {
	if (mFVolume > 1)
		mFVolume = 1;
	else if (mFVolume < 0)
		mFVolume = 0;

	mFmodResult = mFmodChannel->setVolume(mFVolume);
	errorCheck(mFmodResult);
}

void MusicPlayer::readMusicTag(void) {
	// Tag 확인용
	/*
	for (int i = 0; i < 17; i++) {
		mFmodResult = mFmodSound->getTag(NULL, i, &mFmodTag);
		errorCheck(mFmodResult);
		printf("%d-> %s: %d\n", i,mFmodTag.name, mFmodTag.data);
	}
	*/

	mFmodResult = mFmodSound->getTag("TIT2", 0, &mFmodTag);
	if (errorCheck(mFmodResult)) {
		strcpy_s(mStrTitle, (char*)mFmodTag.data);
	}
	else {
		char tmp[MAX_PATH];
		strcpy(tmp, mStrFilePath);
		char *token = strtok(tmp, "\\");

		while (token != NULL)
		{
			strcpy_s(mStrTitle, token);
			token = strtok(NULL, "\\");
		}
	}


	mFmodResult = mFmodSound->getTag("TPE1", 0, &mFmodTag);
	if (errorCheck(mFmodResult)) {
		strcpy_s(mStrArtist, (char*)mFmodTag.data);
	}
	else {
		strcpy_s(mStrArtist, "");
	}

	mFmodResult = mFmodSound->getTag("TALB", 0, &mFmodTag);
	if (errorCheck(mFmodResult)) {
		strcpy_s(mStrAlbum, (char*)mFmodTag.data);
	}
	else {
		strcpy_s(mStrAlbum, "");
	}

	mFmodResult = mFmodSound->getTag("TCON", 0, &mFmodTag);
	if (errorCheck(mFmodResult)) {
		strcpy_s(mStrGenre, (char*)mFmodTag.data);
	}
	else {
		strcpy_s(mStrGenre, "");
	}

	printf("TITLE: %s\n", mStrTitle);
	printf("ARTIST: %s\n", mStrArtist);
	printf("ALBUM: %s\n", mStrAlbum);
	printf("GENRE: %s\n", mStrGenre);
}

void MusicPlayer::init(void) {
	/*
	Create a System object and initialize.
	*/
	mFmodResult = FMOD::System_Create(&mFmodSystem);
	errorCheck(mFmodResult);

	mFmodResult = mFmodSystem->getVersion(&mFmodVersion);
	errorCheck(mFmodResult);

	if (mFmodVersion < FMOD_VERSION) {
		printf("FMOD lib fmod_version %08x doesn't match header fmod_version %08x", mFmodVersion, FMOD_VERSION);
	}

	mFmodResult = mFmodSystem->init(32, FMOD_INIT_NORMAL, NULL);
	errorCheck(mFmodResult);
}

void MusicPlayer::updateSystem(void) {
	mFmodResult = mFmodSystem->update();
	errorCheck(mFmodResult);
}

void MusicPlayer::stopMusic(void) {
	bool isPlaying = false;

	if (mFmodChannel) {
		mFmodResult = mFmodChannel->isPlaying(&isPlaying);
		errorCheck(mFmodResult);
	}

	if (isPlaying) {
		mFmodResult = mFmodChannel->stop();
		errorCheck(mFmodResult);
	}
	else {
		mFmodResult = mFmodSystem->playSound(mFmodSound, 0, false, &mFmodChannel);
		errorCheck(mFmodResult);
	}
}

void MusicPlayer::pausedMusic(void) {
	bool paused;
	mFmodResult = mFmodChannel->getPaused(&paused);
	errorCheck(mFmodResult);
	mFmodResult = mFmodChannel->setPaused(!paused);
	errorCheck(mFmodResult);
}

void MusicPlayer::increaseVolume(bool isIncrease) {
	if (isIncrease)
		mFVolume += 0.1f;
	else
		mFVolume -= 0.1f;

	this->setVolume();
}

char* MusicPlayer::getStrMusicState(void) {
	unsigned int	ms = 0;
	unsigned int	lenms = 0;
	bool			playing = false;
	bool			paused = false;

	if (mFmodChannel) {
		mFmodResult = mFmodChannel->isPlaying(&playing);
		if ((mFmodResult != FMOD_OK) && (mFmodResult != FMOD_ERR_INVALID_HANDLE))
			errorCheck(mFmodResult);

		mFmodResult = mFmodChannel->getPaused(&paused);
		if ((mFmodResult != FMOD_OK) && (mFmodResult != FMOD_ERR_INVALID_HANDLE))
			errorCheck(mFmodResult);

		mFmodResult = mFmodChannel->getPosition(&ms, FMOD_TIMEUNIT_MS);
		if ((mFmodResult != FMOD_OK) && (mFmodResult != FMOD_ERR_INVALID_HANDLE))
			errorCheck(mFmodResult);

		mFmodResult = mFmodSound->getLength(&lenms, FMOD_TIMEUNIT_MS);
		if ((mFmodResult != FMOD_OK) && (mFmodResult != FMOD_ERR_INVALID_HANDLE))
			errorCheck(mFmodResult);
	}

	sprintf_s(mStrMusicState, "Time %02d:%02d:%02d/%02d:%02d:%02d : %s Volume:%02.0f", ms / 1000 / 60, ms / 1000 % 60, ms / 10 % 100, lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100, paused ? "Paused " : playing ? "Playing" : "Stopped", 100 * mFVolume);
	return mStrMusicState;
}

bool MusicPlayer::openMusic(void) {
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
		this->playSound();
		this->setVolume();
		this->getStrMusicState();
		this->readMusicTag();

		delete fileDialog;
		return true;
	}

	delete fileDialog;
	return false;
}

char* MusicPlayer::getTitle(void) {
	return mStrTitle;
}

char* MusicPlayer::getArtist(void) {
	return mStrArtist;
}

char* MusicPlayer::getAlbum(void) {
	return mStrAlbum;
}

char* MusicPlayer::getGenre(void) {
	return mStrGenre;
}