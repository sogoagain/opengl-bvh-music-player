#ifndef DEBUG
#define DEBUG
#endif

#include <gl/glut.h>
#include <stdio.h>
#include "OpenFileDialog.h"

#define EXIT_SUCCESS 0

int gWidth=300, gHeight=300;
int gMenuChoice;

enum MenuItem {
	OPEN_MUSIC_FILE,
	EXIT,
	NO_ACTION
};

void displayFunc(void);
void reshapeFunc(int w, int h);
void addMenuEntry(void);
void selectMenuFunc(int entryID);
TCHAR* getMusicFilePath(void);

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(gWidth, gHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("JUDY (DJ your day)");

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);

	GLint MyMainMenuID = glutCreateMenu(selectMenuFunc);
	addMenuEntry();
	gMenuChoice = NO_ACTION;

	glutMainLoop();

	return EXIT_SUCCESS;
}

void addMenuEntry(void) {
	glutAddMenuEntry("음악파일열기", OPEN_MUSIC_FILE);
	glutAddMenuEntry("Exit", EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void selectMenuFunc(int entryID) {
	gMenuChoice = entryID;

	switch (gMenuChoice) {
	case OPEN_MUSIC_FILE:
		getMusicFilePath();
		break;
	case EXIT:
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
		glutWireCube(0.3);
	}
	
	glFlush();
}

void reshapeFunc(int w, int h) {
	gWidth = w;
	gHeight = h;
}

TCHAR* getMusicFilePath(void) {
	TCHAR arrFilePath[MAX_PATH];

	OpenFileDialog* fileDialog = new OpenFileDialog();
	fileDialog->InitialDir = _T("C:\\Users\\");
	fileDialog->Title = _T("Open Music File");

	// 파일 경로 복사 TCHAR*, char* 조사 필요
	// _tcscpy_s(arrFilePath, fileDialog->FileName);

#ifdef DEBUG
	if (fileDialog->ShowDialog()) {
		MessageBox(0, fileDialog->FileName, _T("선택된 음악 파일"),
			MB_OK | MB_ICONINFORMATION);
	}

	// printf("%ws", arrFilePath);
#endif

	delete fileDialog;
	return arrFilePath;
}