#ifndef DEBUG
#define DEBUG
#endif

#include <gl/glut.h>
#include "MusicPlayer.h"

#define EXIT_SUCCESS 0

using namespace FMOD;

MusicPlayer *musicPlayer;
int	gWidth = 600, gHeight = 600;
int	gMenuChoice;

enum MenuItem {
	OPEN_MUSIC_FILE,
	PAUSE_MUSIC,
	STOP_MUSIC,
	EXIT,
	NO_ACTION
};

void displayFunc(void);
void reshapeFunc(int, int);
void idleFunc(void);
void addMenuEntry(void);
void selectMenuFunc(int);
void renderBitmapCharacter(float, float, float, void*, char*);
void keyboardFunc(unsigned char, int, int);

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(gWidth, gHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("JUDY (DJ your day)");

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);

	GLint MyMainMenuID = glutCreateMenu(selectMenuFunc);
	addMenuEntry();
	gMenuChoice = NO_ACTION;

	musicPlayer = new MusicPlayer();
	musicPlayer->init();

	glutMainLoop();

	delete(musicPlayer);
	return EXIT_SUCCESS;
}

void idleFunc(void) {
	musicPlayer->updateSystem();
	glutPostRedisplay();
}

void addMenuEntry(void) {
	glutAddMenuEntry("À½¾ÇÆÄÀÏ¿­±â", OPEN_MUSIC_FILE);
	glutAddMenuEntry("À½¾Ç ÀÏ½ÃÁ¤Áö", PAUSE_MUSIC);
	glutAddMenuEntry("À½¾Ç Á¤Áö", STOP_MUSIC);
	glutAddMenuEntry("Exit", EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void selectMenuFunc(int entryID) {
	gMenuChoice = entryID;

	switch (gMenuChoice) {
	case OPEN_MUSIC_FILE:
		musicPlayer->openMusic();
		break;
	case PAUSE_MUSIC:
		musicPlayer->pausedMusic();
		break;
	case STOP_MUSIC:
		musicPlayer->stopMusic();
		break;
	case EXIT:
		delete(musicPlayer);
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

void displayFunc(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3ub(255, 255, 255);
		renderBitmapCharacter(-0.95, 0.9, 0, GLUT_BITMAP_TIMES_ROMAN_24, musicPlayer->getTitle());
		renderBitmapCharacter(-0.95, 0.83, 0, GLUT_BITMAP_HELVETICA_18, musicPlayer->getArtist());
		renderBitmapCharacter(-0.95, 0.7, 0, GLUT_BITMAP_HELVETICA_18, musicPlayer->getStrMusicState());
		glutWireCube(0.3);

	glFlush();
}

void reshapeFunc(int w, int h) {
	gWidth = w;
	gHeight = h;
}

void keyboardFunc(unsigned char uChKeyPressed, int x, int y) {
	switch (uChKeyPressed) {
	case 'U': case 'u':
		musicPlayer->increaseVolume(true);
		break;
	case 'D': case'd':
		musicPlayer->increaseVolume(false);
		break;
	case 'Q': case 'q':
		delete(musicPlayer);
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void renderBitmapCharacter(float x, float y, float z, void *font, char* strDisplay) {
	char strOutput[128] = { '\0', };
	int i = 0;

	strcpy_s(strOutput, strDisplay);
	glRasterPos3f(x, y, z);
	for (i = 0; strOutput[i] != '\0'; i++)
		glutBitmapCharacter(font, strOutput[i]);
}