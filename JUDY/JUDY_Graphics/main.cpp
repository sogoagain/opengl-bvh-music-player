#ifndef DEBUG
// #define DEBUG
#endif

#include <gl/glut.h>
#include "MusicPlayer.h"	// 음악 재생을 위한 클래스, FMOD라이브러리 기반
#include "BVH.h"			// BVH 파일 로더

#define EXIT_SUCCESS 0		// 프로그램 성공적인 종료시 리턴 값
#define NUM_OF_TRIGGER 4	// BPM 입력을 위한 트리거 횟수
#define NUM_OF_STAGE 4		// 현재 4개의 STAGE 모드가 제공 (클래식, 댄스, EDM, 기타)

// 마우스 오른쪽 클리식 나타나는 메뉴의 종류
// 음악: 파일열기, 일시정지, 재생or정지
// 화면제어: 확대ro축소, 회전
enum MenuItem {
	OPEN_MUSIC_FILE,
	PAUSE_MUSIC,
	STOP_MUSIC,
	SCALE,
	ROTATE,
	EXIT,
	NO_ACTION
};

// Stage모드 구분을 위한 열거형
enum MusicStage {
	CLASSIC,
	DANCE,
	EDM,
	OTHER_STAGES,
	INTRO
};

// BVH 파일 경로
const char* bvhPath[NUM_OF_STAGE] = { ".\\BVH\\classic.bvh",
".\\BVH\\dance.bvh",
".\\BVH\\edm.bvh",
".\\BVH\\other.bvh" };

// 무대 소품 파일 경로
const char* stageMeshPath[NUM_OF_STAGE] = { ".\\ASE\\Table.ASE",
".\\ASE\\Base.ASE",
".\\ASE\\Speaker.ASE",
".\\ASE\\Column.ASE" };

// 무대 파일 경로
const char* commonMeshPath[NUM_OF_STAGE] = { ".\\ASE\\Stage.ASE",
".\\ASE\\DanceFloor.ASE" };

// Title 파일 경로
const char* titleMeshPath = ".\\ASE\\JUDY.ASE";

MusicPlayer		*gPtrMusicPlayer;				// 음악재생 객체
static int		gWidth = 640, gHeight = 640;	// 윈도우 크기
static int		gMouseMode;					// 현재 선택된 화면제어모드(확대및축소,회전)
static int		gTimerInterval = 40;			// 타이머 콜백 시간 간격
static int		gStage = INTRO;					// 현재 Stage 모드
static int		gLightAngle = 0;				// 빛의 회전 각도
static int		gTitleAngle = 0;				// INTRO모드에서 보여지는 타이틀의 회전 각도

// 카메라 시점 관련 변수
static float	gFCameraYaw = 0.0f;
static float	gFCameraPitch = -20.0f;
static float	gFCameraDistance = 5.0f;

// 마우스 이벤트 관련 변수
static int		gDragMouseR = 0;
static int		gDragMouseL = 0;
static int		gMouseX, gMouseY;

// 애니메이션 관련 변수
static bool		gOnAnimation = true;
static float	gFAnimationTime = 0.0f;
static int		gFrameNo = 0;

BVH				*gPtrBvh[NUM_OF_STAGE] = { NULL, };				// STAGE에 따른 모델
Mesh			*gStageMesh[NUM_OF_STAGE] = { NULL, };			// STAGE의 소품
Mesh			*gCommonStage[NUM_OF_STAGE / 2] = { NULL, };	// STAGE의 공통된 무대
Mesh			*gTitleMesh = NULL;								// 프로그램 Title(JUDY)

// 디스플레이 관련 콜백함수
void displayFunc(void);
void reshapeFunc(int, int);

// 애니메이션 관련 콜백함수
void idleFunc(void);
void timerFunc(int value);

// 표준입력 관련 콜백함수
void mouseClickFunc(int, int, int, int);
void mouseMotionFunc(int, int);
void keyboardFunc(unsigned char, int, int);

// 메뉴 콜백 함수
void addMainMenu(void);
void addMusicMenu(void);
void addControlMenu(void);
void selectMainMenu(int);
void selectMusicMenu(int);
void selectControlMenu(int);

int	calculateBPM(DWORD triggerTime[]);	// BPM (타이머 시간 간격) 계산
int getStage(char*);					// 음악 장르에 기반해 현재 STAGE 결정
void drawMessage(int, const char*);		// 화면에 문자열 그리기
void drawFloor(void);					// 바닥 그리기

void initEnvironment(void);				// 프로그램 초기화
void shutdownEnvironment(void);			// 프로그램에 사용된 메모리 반납


int main(int argc, char* argv[]) {
	// 윈도우 초기화
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL);
	glutInitWindowSize(gWidth, gHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("JUDY (DJ your day)");

	// 콜백함수 등록
	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseClickFunc);
	glutMotionFunc(mouseMotionFunc);
	glutTimerFunc(gTimerInterval, timerFunc, 1);

	// 메뉴 등록
	GLint musicMenuID = glutCreateMenu(selectMusicMenu);
	addMusicMenu();
	GLint controlMenuID = glutCreateMenu(selectControlMenu);
	addControlMenu();
	GLint mainMenuID = glutCreateMenu(selectMainMenu);
	glutAddSubMenu("음악", musicMenuID);
	glutAddSubMenu("화면제어", controlMenuID);
	addMainMenu();
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// 프로그램 초기화
	initEnvironment();
	// 이벤트 루프진입
	glutMainLoop();

	// 프로그램 종료
	shutdownEnvironment();

	return EXIT_SUCCESS;
}

void displayFunc(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/********************조명설정************************/
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
	/***************************************************/

	/*
		음악장르에 따라 구별된 Stage를 기반으로 display한다.
	*/

	// INTRO 스테이지: 프로그램 타이틀과 메뉴얼을 출력한다.
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

	// CLASSIC 스테이지
	else if (gStage == CLASSIC) {
		// 바닥
		drawFloor();

		// 소품
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
		// 무대
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

	// DANCE 스테이지
	else if (gStage == DANCE) {
		// 바닥
		drawFloor();

		// 소품
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
		// 무대
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

	// EDM 스테이지
	else if (gStage == EDM) {
		// EDM은 특수효과 측면으로 모든 모델들을 LINE으로 그린다.
		glPolygonMode(GL_FRONT, GL_LINE);

		// 바닥
		drawFloor();

		// 소품
		glPushMatrix();
		glColor3ub(051, 051, 051);
		glTranslatef(0, -1.1, 0);
		gCommonStage[1]->drawMesh(10);
		glColor3ub(255, 0, 0);
		glPopMatrix();

		// 무대
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

	// 그 외의 STAGE
	else {
		// 바닥
		drawFloor();

		// 소품
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
		// 무대
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


	// 문자열 출력
	char  message[64];

	if (gStage != INTRO) {
		if (gPtrBvh[gStage])
			sprintf_s(message, "%.2f (%d)", gFAnimationTime, gFrameNo);
		else
			sprintf_s(message, "JUDY can't load bvh file", gFAnimationTime, gFrameNo);
	} else
		sprintf_s(message, "Shall we dance?", gFAnimationTime, gFrameNo);

	drawMessage(0, message);
	drawMessage(1, gPtrMusicPlayer->getTitle());
	drawMessage(2, gPtrMusicPlayer->getArtist());
	drawMessage(3, gPtrMusicPlayer->getStrMusicState());
	glutSwapBuffers();
	glFlush();
}

void reshapeFunc(int w, int h) {
	// 변경된 윈도우 크기로 뷰포트 재설정
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double)w / h, 1, 500);

	gWidth = w;
	gHeight = h;
}

void idleFunc(void) {
	// FMOD system update
	gPtrMusicPlayer->updateSystem();

	// BVH 애니메이션
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

void timerFunc(int value) {
	if (gStage == INTRO) {
		// INTRO 모드시 TITLE의 회전 각도
		gTitleAngle = (gTitleAngle + 3) % 360;
	}
	else {
		// 모델이 그려지고 있을 시 조명의 회전 각도
		gLightAngle = (gLightAngle + 15) % 360;
	}

	// BPM을 바탕으로 측정된 gTimerInterval을 간격으로 타이머 콜백 호출
	glutTimerFunc(gTimerInterval, timerFunc, 1);
	glutPostRedisplay();
}

void mouseClickFunc(int button, int state, int x, int y) {
	// 마우스 클릭시 좌표 저장
	gMouseX = x;
	gMouseY = y;

	glutPostRedisplay();
}

void mouseMotionFunc(int x, int y) {
	// 마우스 좌표 변화량에 따라 카메라 시점 변경
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
	case 'U': case 'u':		// 볼륨 UP
		gPtrMusicPlayer->increaseVolume(true);
		break;
	case 'D': case'd':		// 볼륨 DOWN
		gPtrMusicPlayer->increaseVolume(false);
		break;
	case 'Q': case 'q':		// 프로그램 종료
		shutdownEnvironment();
		exit(EXIT_SUCCESS);
		break;
	case 'R': case 'r':		// Restart 애니메이션
		gFAnimationTime = 0.0f;
		gFrameNo = 0;
		break;
	case 'S': case 's':		// Start 애니메이션
		gOnAnimation = !gOnAnimation;
		break;
	case VK_SPACE:			// Space키 4번 입력을 통해 BPM 계산
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

int getStage(char* musicGenre) {
	if (!strcmp(musicGenre, "Classical"))
		return CLASSIC;
	else if (!strcmp(musicGenre, "Dance"))
		return DANCE;
	else if (!strcmp(musicGenre, "Electronic"))
		return EDM;

	return OTHER_STAGES;
}

void drawMessage(int line_no, const char * message) {
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

void initEnvironment(void) {
	// 모델, 소품 객체 생성
	for (int i = 0; i < NUM_OF_STAGE; i++) {
		gPtrBvh[i] = new BVH(bvhPath[i]);
		gStageMesh[i] = new Mesh(stageMeshPath[i]);
	}

	// 무대 객체 생성
	for (int i = 0; i < NUM_OF_STAGE / 2; i++) {
		gCommonStage[i] = new Mesh(commonMeshPath[i]);
	}

	// INTRO시 보여지는 타이틀 객체 생성
	gTitleMesh = new Mesh(titleMeshPath);

	// MP3 재생을 위한 MusicPlayer 객체 생성
	gPtrMusicPlayer = new MusicPlayer();
	gPtrMusicPlayer->init();

	// 0번 조명 설정
	GLfloat light0_ambient[] = { 0.5, 0.4, 0.3, 1.0 };
	GLfloat light0_diffuse[] = { 0.8, 0.7, 0.6, 1.0 };
	GLfloat light0_specular[] = { 0.0, 0.0, 0.0, 0.0 };

	// 물체 특성
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

	// 배경색 설정
	glClearColor(0.6, 0.6, 1.0, 0.0);

	gMouseMode = SCALE;
}

void shutdownEnvironment(void) {
	// 각 객체들의 메모리 해제
	delete(gPtrMusicPlayer);
	for (int i = 0; i < NUM_OF_STAGE; i++) {
		delete(gPtrBvh[i]);
		delete(gStageMesh[i]);
	}
	for (int i = 0; i < NUM_OF_STAGE / 2; i++)
		delete(gCommonStage[i]);
	delete(gTitleMesh);
}
