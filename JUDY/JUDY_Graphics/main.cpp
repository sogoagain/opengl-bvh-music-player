#ifndef DEBUG
#define DEBUG
#endif

#include <gl/glut.h>
#include "MusicPlayer.h"

#define EXIT_SUCCESS 0

using namespace FMOD;

MusicPlayer *musicPlayer;
int	gWidth = 300, gHeight = 300;
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
void renderBitmapCharacter(float, float, float, void*);

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(gWidth, gHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("JUDY (DJ your day)");

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutIdleFunc(idleFunc);

	GLint MyMainMenuID = glutCreateMenu(selectMenuFunc);
	addMenuEntry();
	gMenuChoice = NO_ACTION;

	musicPlayer = new MusicPlayer();
	musicPlayer->FMOD_init();

	glutMainLoop();

	delete(musicPlayer);
	return EXIT_SUCCESS;
}

void idleFunc(void) {
	musicPlayer->FMOD_update();
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
		musicPlayer->FMOD_pausedMusic();
		break;
	case STOP_MUSIC:
		musicPlayer->FMOD_stopMusic();
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

	glViewport(0, 0, gWidth, gHeight); {
		renderBitmapCharacter(-0.9, 0.9, 0, GLUT_BITMAP_HELVETICA_18);
		glutWireCube(0.3);
	}

	glFlush();
}

void reshapeFunc(int w, int h) {
	gWidth = w;
	gHeight = h;
}

void renderBitmapCharacter(float x, float y, float z, void *font) {
	char strOutput[128] = { '\0', };
	int i = 0;
	musicPlayer->FMOD_getNowState();
	strcpy_s(strOutput, musicPlayer->musicState);
	glRasterPos3f(x, y, z);
	for (i = 0; strOutput[i] != '\0'; i++)
		glutBitmapCharacter(font, strOutput[i]);
}