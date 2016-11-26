#ifndef DEBUG
#define DEBUG
#endif

#include <gl/glut.h>
#include "MusicPlayer.h"	// 음악 재생을 위한 클래스, FMOD라이브러리 기반
#include "BVH.h"			// BVH 파일 로더

#define EXIT_SUCCESS 0
#define NUM_OF_TRIGGER 4
#define NUM_OF_STAGE 4

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

enum MusicStage {
	CLASSIC,
	DANCE,
	EDM,
	OTHER_STAGES,
	INTRO
};

MusicPlayer		*gPtrMusicPlayer;
int				gWidth = 640, gHeight = 640;
int				gMenuChoice;
int				gTimerInterval = 40;
int				gStage = INTRO;
int				gLightAngle = 0;
int				gTitleAngle = 0;

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

BVH				*gPtrBvh[NUM_OF_STAGE] = { NULL, };
Mesh			*gStageMesh[NUM_OF_STAGE] = { NULL, };
Mesh			*gCommonStage[NUM_OF_STAGE / 2] = { NULL, };
Mesh			*gTitleMesh = NULL;
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
void timerFunc(int value);

void mouseClickFunc(int, int, int, int);
void mouseMotionFunc(int, int);
void keyboardFunc(unsigned char, int, int);
/*********/

int	calculateBPM(DWORD triggerTime[]);
int getStage(char*);
void initEnvironment(void);
void drawMessage(int, const char*);
void shutdownEnvironment(void);
void drawFloor(void);

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
	glutTimerFunc(gTimerInterval, timerFunc, 1);

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

	shutdownEnvironment();

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
		if (gStage != INTRO) {
			if (gPtrBvh[gStage]) {
				gFrameNo = gFAnimationTime / gPtrBvh[gStage]->GetInterval();
				gFrameNo = gFrameNo % gPtrBvh[gStage]->GetNumFrame();
			}
			else
				gFrameNo = 0;
		}

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
		shutdownEnvironment();
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

void selectMusicMenu(int entryID) {
	switch (entryID) {
	case OPEN_MUSIC_FILE:
		if (gPtrMusicPlayer->openMusic()) {
			gTimerInterval = 40;
			gStage = getStage(gPtrMusicPlayer->getGenre());
#ifdef DEBUG
			printf("STAGE_INDEX: %d\n", gStage);
#endif
		}
		break;
	case PAUSE_MUSIC:
		gPtrMusicPlayer->pausedMusic();
		break;
	case STOP_MUSIC:
		gPtrMusicPlayer->stopMusic();
		break;
	case EXIT:
		shutdownEnvironment();
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

	GLfloat light0_position[] = { 0.0, 10.0, 0.0, 1.0 };

	glPushMatrix();
	glRotatef(gLightAngle, 1.0, 0.0, 0.0);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -gFCameraDistance);
	glRotatef(-gFCameraPitch, 1.0, 0.0, 0.0);
	glRotatef(-gFCameraYaw, 0.0, 1.0, 0.0);
	glTranslatef(0.0, -0.5, 0.0);

	if (gStage == INTRO) {
		glPushMatrix();
		glColor3ub(255, 255, 0);
		glTranslatef(0, 1, 0);
		glRotatef(gTitleAngle, 0, 1, 0);
		gTitleMesh->drawMesh(100);
		glColor3ub(255, 0, 0);
		glPopMatrix();

		drawMessage(20, "********************INSTRUCTIONS********************");
		drawMessage(22, "Menu: Right-click");
		drawMessage(24, "Volume UP/DOWN: U, D");
		drawMessage(26, "Stop Animation: S");
		drawMessage(28, "Restart Animation: R");
		drawMessage(30, "BPM calculation : enter the space bar 4 times according to the beat");
		drawMessage(32, "************************************************************");
	}
	else if (gStage == CLASSIC) {
		drawFloor();

		glPushMatrix();
		glColor3ub(102, 102, 102);
		glTranslatef(-5.0, 0.0, -5.0);
		gStageMesh[CLASSIC]->drawMesh(5);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(5.0, 0.0, -5.0);
		gStageMesh[CLASSIC]->drawMesh(5);
		glPopMatrix();

		glDisable(GL_LIGHTING);
		glPushMatrix();
		glColor3ub(102, 051, 051);
		glTranslatef(0, -0.2, 0);
		gCommonStage[0]->drawMesh(35);
		glPopMatrix();
		// 모델
		if (gPtrBvh[CLASSIC])
			gPtrBvh[CLASSIC]->RenderFigure(gFrameNo, 0.09f);
		glEnable(GL_LIGHTING);
	}
	else if (gStage == DANCE) {
		drawFloor();

		glPushMatrix();
		glColor3ub(255, 000, 000);
		glTranslatef(-5.0, 0.0, -5.0);
		gStageMesh[DANCE]->drawMesh(13);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(5.0, 0.0, -5.0);
		gStageMesh[DANCE]->drawMesh(13);
		glPopMatrix();

		glDisable(GL_LIGHTING);
		glPushMatrix();
		glColor3ub(000, 000, 153);
		glTranslatef(0, -1.1, 0);
		gCommonStage[1]->drawMesh(10);
		glPopMatrix();
		// 모델
		if (gPtrBvh[DANCE])
			gPtrBvh[DANCE]->RenderFigure(gFrameNo, 0.09f);
		glEnable(GL_LIGHTING);
	}
	else if (gStage == EDM) {
		glPolygonMode(GL_FRONT, GL_LINE);
		drawFloor();
		glPushMatrix();
		glColor3ub(051, 051, 051);
		glTranslatef(0, -1.1, 0);
		gCommonStage[1]->drawMesh(10);
		glColor3ub(255, 0, 0);
		glPopMatrix();

		glPushMatrix();
		glColor3ub(255, 000, 000);
		glTranslatef(-5.0, 0.0, -5.0);
		gStageMesh[EDM]->drawMesh(0.3);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(5.0, 0.0, -5.0);
		gStageMesh[EDM]->drawMesh(0.3);
		glPopMatrix();

		glDisable(GL_LIGHTING);
		// 모델
		if (gPtrBvh[EDM])
			gPtrBvh[EDM]->RenderFigure(gFrameNo, 0.09f);
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT, GL_FILL);
	}
	else {
		drawFloor();

		glPushMatrix();
		glColor3ub(102, 102, 102);
		glTranslatef(-5.0, 0.0, -5.0);
		gStageMesh[OTHER_STAGES]->drawMesh(80);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(5.0, 0.0, -5.0);
		gStageMesh[OTHER_STAGES]->drawMesh(80);
		glPopMatrix();

		glDisable(GL_LIGHTING);
		glPushMatrix();
		glColor3ub(102, 051, 051);
		glTranslatef(0, -0.2, 0);
		gCommonStage[0]->drawMesh(35);
		glPopMatrix();
		// 모델
		if (gPtrBvh[OTHER_STAGES])
			gPtrBvh[OTHER_STAGES]->RenderFigure(gFrameNo, 0.09f);
		glEnable(GL_LIGHTING);
	}


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

	switch (uChKeyPressed) {
	case 'U': case 'u':
		gPtrMusicPlayer->increaseVolume(true);
		break;
	case 'D': case'd':
		gPtrMusicPlayer->increaseVolume(false);
		break;
	case 'Q': case 'q':
		shutdownEnvironment();
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
			gTimerInterval = calculateBPM(triggerTime);
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

	timeInterval /= 24;
	printf("BPM = %d\n", BPM);
#ifdef DEBUG
	printf("TimeInterval = %d\n", timeInterval);
#endif
	return timeInterval;
}

void  initEnvironment(void) {

	GLfloat light0_ambient[] = { 0.5, 0.4, 0.3, 1.0 };
	GLfloat light0_diffuse[] = { 0.8, 0.7, 0.6, 1.0 };
	GLfloat light0_specular[] = { 0.0, 0.0, 0.0, 0.0 };

	GLfloat material_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat material_diffuse[] = { 0.9, 0.9, 0.9, 1.0 };
	GLfloat material_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat material_shininess[] = { 25.0 };

	glShadeModel(GL_SMOOTH);	// 구로 셰이딩
	glEnable(GL_DEPTH_TEST);	// 깊이 버퍼 활성화
	glEnable(GL_LIGHTING);		// 조명 활성화

	glEnable(GL_COLOR_MATERIAL);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

	glClearColor(0.6, 0.6, 1.0, 0.0);

	gPtrBvh[CLASSIC] = new BVH(".\\BVH\\classic.bvh");
	gPtrBvh[DANCE] = new BVH(".\\BVH\\dance.bvh");
	gPtrBvh[EDM] = new BVH(".\\BVH\\edm.bvh");
	gPtrBvh[OTHER_STAGES] = new BVH(".\\BVH\\other.bvh");

	gStageMesh[CLASSIC] = new Mesh();
	gStageMesh[CLASSIC]->readAse(".\\ASE\\Table.ASE");
	gStageMesh[DANCE] = new Mesh();
	gStageMesh[DANCE]->readAse(".\\ASE\\Base.ASE");
	gStageMesh[EDM] = new Mesh();
	gStageMesh[EDM]->readAse(".\\ASE\\Speaker.ASE");
	gStageMesh[OTHER_STAGES] = new Mesh();
	gStageMesh[OTHER_STAGES]->readAse(".\\ASE\\Column.ASE");

	gCommonStage[0] = new Mesh();
	gCommonStage[0]->readAse(".\\ASE\\Stage.ASE");
	gCommonStage[1] = new Mesh();
	gCommonStage[1]->readAse(".\\ASE\\DanceFloor.ASE");

	gTitleMesh = new Mesh();
	gTitleMesh->readAse(".\\ASE\\JUDY.ASE");
	gTitleMesh->setColor(0.6, 0.6, 0.0);

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

int getStage(char* musicGenre) {
	if (!strcmp(musicGenre, "Classical"))
		return CLASSIC;
	else if (!strcmp(musicGenre, "Dance"))
		return DANCE;
	else if (!strcmp(musicGenre, "Electronic"))
		return EDM;

	return OTHER_STAGES;
}

void timerFunc(int value) {
	if (gStage == INTRO) {
		gTitleAngle = (gTitleAngle + 3) % 360;
	}
	else {
		gLightAngle = (gLightAngle + 15) % 360;
	}
	glutTimerFunc(gTimerInterval, timerFunc, 1);
	glutPostRedisplay();
}

void shutdownEnvironment(void) {
	delete(gPtrMusicPlayer);
	for (int i = 0; i < NUM_OF_STAGE; i++) {
		delete(gPtrBvh[i]);
		delete(gStageMesh[i]);
	}
	for (int i = 0; i < NUM_OF_STAGE / 2; i++)
		delete(gCommonStage[i]);
	delete(gTitleMesh);
}

void drawFloor(void) {
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
}