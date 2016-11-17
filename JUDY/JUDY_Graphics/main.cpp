#ifndef DEBUG
#define DEBUG
#endif

#include <gl/glut.h>
#include "MusicPlayer.h"
#include "BVH.h"

#define EXIT_SUCCESS 0
#define NUM_OF_TRIGGER 4

using namespace FMOD;

enum MenuItem {
	OPEN_MUSIC_FILE,
	PAUSE_MUSIC,
	STOP_MUSIC,
	SCALE,
	ROTATE,
	EXIT,
	NO_ACTION
};

MusicPlayer		*gPtrMusicPlayer;
int				gWidth = 640, gHeight = 640;
int				gMenuChoice;
int				gBPM = 100;

/*************BVH 관련 변수****************/
static float	gFCameraYaw = 0.0f;
static float	gFCameraPitch = -20.0f;
static float	gFCameraDistance = 5.0f;

int		gMouseMode;
static int		gDragMouseR = 0;
static int		gDragMouseL = 0;
static int		gMouseX, gMouseY;

bool			gOnAnimation = true;
float			gFAnimationTime = 0.0f;
int				gFrameNo = 0;

BVH				*gPtrBvh = NULL;
/****************************************/

/*콜백함수*/
void selectMainMenu(int);
void addMainMenu(void);
void addMusicMenu(void);
void selectMusicMenu(int);
void addControlMenu(void);
void selectControlMenu(int);

void displayFunc(void);
void reshapeFunc(int, int);
void idleFunc(void);

void mouseClickFunc(int, int, int, int);
void mouseMotionFunc(int, int);
void keyboardFunc(unsigned char, int, int);
/*********/

int	 calculateBPM(DWORD triggerTime[]);
void initEnvironment(void);
void drawMessage(int, const char*);

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL);
	glutInitWindowSize(gWidth, gHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("JUDY (DJ your day)");

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseClickFunc);
	glutMotionFunc(mouseMotionFunc);

	GLint musicMenuID = glutCreateMenu(selectMusicMenu);
	addMusicMenu();
	GLint controlMenuID = glutCreateMenu(selectControlMenu);
	addControlMenu();
	GLint mainMenuID = glutCreateMenu(selectMainMenu);
	glutAddSubMenu("음악", musicMenuID);
	glutAddSubMenu("화면제어", controlMenuID);
	addMainMenu();
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	initEnvironment();
	glutMainLoop();

	delete(gPtrMusicPlayer);
	delete(gPtrBvh);
	return EXIT_SUCCESS;
}

void idleFunc(void) {
	gPtrMusicPlayer->updateSystem();

	if (gOnAnimation) {
#ifdef  WIN32
		static DWORD  last_time = 0;
		DWORD  curr_time = timeGetTime();
		float  delta = (curr_time - last_time) * 0.001f;
		if (delta > 0.03f)
			delta = 0.03f;
		last_time = curr_time;
		gFAnimationTime += delta;
#else
		gFAnimationTime += 0.03f;
#endif
		if (gPtrBvh) {
			gFrameNo = gFAnimationTime / gPtrBvh->GetInterval();
			gFrameNo = gFrameNo % gPtrBvh->GetNumFrame();
		}
		else
			gFrameNo = 0;

		glutPostRedisplay();
	}
}

void addMainMenu(void) {
	glutAddMenuEntry("프로그램 종료", EXIT);
}

void addMusicMenu(void) {
	glutAddMenuEntry("파일열기", OPEN_MUSIC_FILE);
	glutAddMenuEntry("일시정지", PAUSE_MUSIC);
	glutAddMenuEntry("정지/재생", STOP_MUSIC);
}

void addControlMenu(void) {
	glutAddMenuEntry("확대/축소", SCALE);
	glutAddMenuEntry("회전", ROTATE);
}

void selectMainMenu(int entryID) {
	switch (entryID) {
	case EXIT:
		delete(gPtrMusicPlayer);
		delete(gPtrBvh);
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

void selectMusicMenu(int entryID) {
	switch (entryID) {
	case OPEN_MUSIC_FILE:
		gPtrMusicPlayer->openMusic();
		break;
	case PAUSE_MUSIC:
		gPtrMusicPlayer->pausedMusic();
		break;
	case STOP_MUSIC:
		gPtrMusicPlayer->stopMusic();
		break;
	case EXIT:
		delete(gPtrMusicPlayer);
		delete(gPtrBvh);
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

void selectControlMenu(int entryID) {
	switch (entryID) {
	case SCALE:
		gMouseMode = SCALE;
		break;
	case ROTATE:
		gMouseMode = ROTATE;
		break;
	default:
		break;
	}
}

void displayFunc(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -gFCameraDistance);
	glRotatef(-gFCameraPitch, 1.0, 0.0, 0.0);
	glRotatef(-gFCameraYaw, 0.0, 1.0, 0.0);
	glTranslatef(0.0, -0.5, 0.0);

	float  light0_position[] = { 10.0, 10.0, 10.0, 1.0 };
	//float	light_color[] = { 0.5, 0.5, 0.5 };
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_color);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	// 배경
	float  size = 1.5f;
	int  num_x = 10, num_z = 10;
	double  ox, oz;
	glBegin(GL_QUADS);
	glNormal3d(0.0, 1.0, 0.0);
	ox = -(num_x * size) / 2;
	for (int x = 0; x < num_x; x++, ox += size)
	{
		oz = -(num_z * size) / 2;
		for (int z = 0; z < num_z; z++, oz += size)
		{
			if (((x + z) % 2) == 0)
				glColor3f(1.0, 1.0, 1.0);
			else
				glColor3f(0.8, 0.8, 0.8);
			glVertex3d(ox, 0.0, oz);
			glVertex3d(ox, 0.0, oz + size);
			glVertex3d(ox + size, 0.0, oz + size);
			glVertex3d(ox + size, 0.0, oz);
		}
	}
	glEnd();
	

	// 모델

	glColor3f(1.0f, 0.0f, 0.0f);
	if (gPtrBvh)
		gPtrBvh->RenderFigure(gFrameNo, 0.04f);


	// 문자열
	char  message[64];
	if (gPtrBvh)
		sprintf_s(message, "%.2f (%d)", gFAnimationTime, gFrameNo);
	else
		sprintf_s(message, "Press 'L' key to Load a BVH file");
	drawMessage(0, message);
	drawMessage(1, gPtrMusicPlayer->getTitle());
	drawMessage(2, gPtrMusicPlayer->getArtist());
	drawMessage(3, gPtrMusicPlayer->getStrMusicState());
	glutSwapBuffers();
	glFlush();
}

void reshapeFunc(int w, int h) {
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double)w / h, 1, 500);

	gWidth = w;
	gHeight = h;
}

void mouseClickFunc(int button, int state, int x, int y) {
	gMouseX = x;
	gMouseY = y;

	glutPostRedisplay();
}

void mouseMotionFunc(int x, int y) {
	if (gMouseMode == ROTATE) {
		gFCameraYaw -= (x - gMouseX) * 1.0;
		if (gFCameraYaw < 0.0)
			gFCameraYaw += 360.0;
		else if (gFCameraYaw > 360.0)
			gFCameraYaw -= 360.0;

		gFCameraPitch -= (y - gMouseY) * 1.0;
		if (gFCameraPitch < -90.0)
			gFCameraPitch = -90.0;
		else if (gFCameraPitch > 90.0)
			gFCameraPitch = 90.0;
	}

	if (gMouseMode == SCALE) {
		gFCameraDistance += (y - gMouseY) * 0.2;
		if (gFCameraDistance < 2.0)
			gFCameraDistance = 2.0;
	}

	gMouseX = x;
	gMouseY = y;

	glutPostRedisplay();
}

void keyboardFunc(unsigned char uChKeyPressed, int x, int y) {
	static	int		triggerCnt = 0;
	static	DWORD	triggerTime[NUM_OF_TRIGGER] = { 0, };

	if ((uChKeyPressed == 'n') && !gOnAnimation) {
		gFAnimationTime += gPtrBvh->GetInterval();
		gFrameNo++;
		gFrameNo = gFrameNo % gPtrBvh->GetNumFrame();
	}

	if ((uChKeyPressed == 'p') && !gOnAnimation && (gFrameNo > 0) && gPtrBvh) {
		gFAnimationTime -= gPtrBvh->GetInterval();
		gFrameNo--;
		gFrameNo = gFrameNo % gPtrBvh->GetNumFrame();
	}
	
	switch (uChKeyPressed) {
	case 'U': case 'u':
		gPtrMusicPlayer->increaseVolume(true);
		break;
	case 'D': case'd':
		gPtrMusicPlayer->increaseVolume(false);
		break;
	case 'Q': case 'q':
		delete(gPtrMusicPlayer);
		delete(gPtrBvh);
		exit(EXIT_SUCCESS);
		break;
	case 'R': case 'r':
		gFAnimationTime = 0.0f;
		gFrameNo = 0;
		break;
	case 'S': case 's':
		gOnAnimation = !gOnAnimation;
		break;
	case VK_SPACE:
		if (triggerCnt < NUM_OF_TRIGGER) {
			triggerTime[triggerCnt] = timeGetTime();
			printf("%d ", triggerCnt + 1);
		}
		triggerCnt++;

		if (triggerCnt == NUM_OF_TRIGGER) {
			gBPM = calculateBPM(triggerTime);
			triggerCnt = 0;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

int calculateBPM(DWORD triggerTime[]) {
	int	timeInterval = 0;
	int BPM = 0;
	for (int i = 0; i < NUM_OF_TRIGGER - 1; i++) {
		timeInterval += triggerTime[i + 1] - triggerTime[i];
	}
	timeInterval /= (NUM_OF_TRIGGER - 1);

	BPM = 60 * 1000 / timeInterval;
#ifdef DEBUG
	printf("BPM = %d\n", BPM);
#endif
	return BPM;
}

void  initEnvironment(void) {
	float  light0_position[] = { 10.0, 10.0, 10.0, 1.0 };
	float  light0_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	float  light0_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	float  light0_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glClearColor(0.5, 0.5, 0.8, 0.0);

	gPtrBvh = new BVH("D:\\Storage\\GitHub\\BVH\\DancingBVH\\05_03.bvh");

	gPtrMusicPlayer = new MusicPlayer();
	gPtrMusicPlayer->init();

	gMouseMode = SCALE;
}

void  drawMessage(int line_no, const char * message) {
	int   i;
	if (message == NULL)
		return;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, gWidth, gHeight, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glColor3ub(0, 0, 0);
	glRasterPos2i(8, 24 + 18 * line_no);
	for (i = 0; message[i] != '\0'; i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, message[i]);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}