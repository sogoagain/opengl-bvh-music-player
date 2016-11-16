#ifndef DEBUG
#define DEBUG
#endif

#include <gl/glut.h>
#include "MusicPlayer.h"
#include "BVH.h"

#define EXIT_SUCCESS 0
#define NUM_OF_TRIGGER 4

using namespace FMOD;

MusicPlayer	*musicPlayer;
int			gWidth = 640, gHeight = 640;
int			gMenuChoice;
int			gBPM = 100;

/************/
static float   camera_yaw = 0.0f;
static float   camera_pitch = -20.0f;
static float   camera_distance = 5.0f;

static int     drag_mouse_r = 0;
static int     drag_mouse_l = 0;
static int     drag_mouse_m = 0;
static int     last_mouse_x, last_mouse_y;
static int     win_width, win_height;

bool   on_animation = true;
float  animation_time = 0.0f;
int    frame_no = 0;

BVH *   bvh = NULL;
/************/

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
void mouseClickFunc(int, int, int, int);
void mouseMotionFunc(int, int);
void keyboardFunc(unsigned char, int, int);
void renderBitmapCharacter(float, float, float, void*, char*);
int calculateBPM(DWORD triggerTime[]);
void  initEnvironment(void);
void  drawMessage(int, const char*);

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

	GLint MyMainMenuID = glutCreateMenu(selectMenuFunc);
	addMenuEntry();
	gMenuChoice = NO_ACTION;

	musicPlayer = new MusicPlayer();
	musicPlayer->init();

	initEnvironment();

	glutMainLoop();

	delete(musicPlayer);
	return EXIT_SUCCESS;
}

void idleFunc(void) {
	musicPlayer->updateSystem();

	if (on_animation)
	{
#ifdef  WIN32
		static DWORD  last_time = 0;
		DWORD  curr_time = timeGetTime();
		float  delta = (curr_time - last_time) * 0.001f;
		if (delta > 0.03f)
			delta = 0.03f;
		last_time = curr_time;
		animation_time += delta;
#else
		animation_time += 0.03f;
#endif
		if (bvh)
		{
			frame_no = animation_time / bvh->GetInterval();
			frame_no = frame_no % bvh->GetNumFrame();
		}
		else
			frame_no = 0;

		glutPostRedisplay();
	}
}

void addMenuEntry(void) {
	glutAddMenuEntry("음악파일열기", OPEN_MUSIC_FILE);
	glutAddMenuEntry("음악 일시정지", PAUSE_MUSIC);
	glutAddMenuEntry("음악 정지", STOP_MUSIC);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -camera_distance);
	glRotatef(-camera_pitch, 1.0, 0.0, 0.0);
	glRotatef(-camera_yaw, 0.0, 1.0, 0.0);
	glTranslatef(0.0, -0.5, 0.0);

	float  light0_position[] = { 10.0, 10.0, 10.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

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

	glColor3f(1.0f, 0.0f, 0.0f);
	if (bvh)
		bvh->RenderFigure(frame_no, 0.02f);

	char  message[64];
	if (bvh)
		sprintf_s(message, "%.2f (%d)", animation_time, frame_no);
	else
		sprintf_s(message, "Press 'L' key to Load a BVH file");
	drawMessage(0, message);
	drawMessage(1, musicPlayer->getTitle());
	drawMessage(2, musicPlayer->getArtist());
	drawMessage(3, musicPlayer->getStrMusicState());
	glutSwapBuffers();



	glFlush();
}

void reshapeFunc(int w, int h) {
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double)w / h, 1, 500);

	win_width = w;
	win_height = h;
}

void mouseClickFunc(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		drag_mouse_l = 1;
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
		drag_mouse_l = 0;

	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
		drag_mouse_r = 1;
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP))
		drag_mouse_r = 0;

	if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN))
		drag_mouse_m = 1;
	else if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP))
		drag_mouse_m = 0;

	glutPostRedisplay();

	last_mouse_x = x;
	last_mouse_y = y;
}

void mouseMotionFunc(int x, int y) {
	if (drag_mouse_r)
	{
		camera_yaw -= (x - last_mouse_x) * 1.0;
		if (camera_yaw < 0.0)
			camera_yaw += 360.0;
		else if (camera_yaw > 360.0)
			camera_yaw -= 360.0;

		camera_pitch -= (y - last_mouse_y) * 1.0;
		if (camera_pitch < -90.0)
			camera_pitch = -90.0;
		else if (camera_pitch > 90.0)
			camera_pitch = 90.0;
	}

	if (drag_mouse_l)
	{
		camera_distance += (y - last_mouse_y) * 0.2;
		if (camera_distance < 2.0)
			camera_distance = 2.0;
	}

	last_mouse_x = x;
	last_mouse_y = y;

	glutPostRedisplay();
}

void keyboardFunc(unsigned char uChKeyPressed, int x, int y) {
	static	int		triggerCnt = 0;
	static	DWORD	triggerTime[NUM_OF_TRIGGER] = { 0, };

	if (uChKeyPressed == 's')
		on_animation = !on_animation;

	if ((uChKeyPressed == 'n') && !on_animation)
	{
		animation_time += bvh->GetInterval();
		frame_no++;
		frame_no = frame_no % bvh->GetNumFrame();
	}

	if ((uChKeyPressed == 'p') && !on_animation && (frame_no > 0) && bvh)
	{
		animation_time -= bvh->GetInterval();
		frame_no--;
		frame_no = frame_no % bvh->GetNumFrame();
	}

	if (uChKeyPressed == 'r')
	{
		animation_time = 0.0f;
		frame_no = 0;
	}

	if (uChKeyPressed == 'l')
	{
		const int  file_name_len = 256;
		//char  file_name[ file_name_len ] = "";
		TCHAR  file_name[file_name_len] = L"";

		OPENFILENAME	open_file;
		memset(&open_file, 0, sizeof(OPENFILENAME));
		open_file.lStructSize = sizeof(OPENFILENAME);
		open_file.hwndOwner = NULL;
		open_file.lpstrFilter = L"BVH Motion Data (*.bvh)\0*.bvh\0All (*.*)\0*.*\0";
		open_file.nFilterIndex = 1;
		open_file.lpstrFile = file_name;
		open_file.nMaxFile = file_name_len;
		open_file.lpstrTitle = L"Select a BVH file";
		open_file.lpstrDefExt = L"bvh";
		open_file.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		BOOL  ret = GetOpenFileName(&open_file);

		if (ret)
		{
			if (bvh)
				delete  bvh;


			// wchar -> char 변환
			int len = 256;
			char str[256];

			WideCharToMultiByte(CP_ACP, 0, file_name, len, str, len, NULL, NULL);

			//bvh = new BVH( file_name );
			bvh = new BVH(str);

			if (!bvh->IsLoadSuccess())
			{
				delete  bvh;
				bvh = NULL;
			}

			animation_time = 0.0f;
			frame_no = 0;
		}
	}


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

void renderBitmapCharacter(float x, float y, float z, void *font, char* strDisplay) {
	char strOutput[128] = { '\0', };
	int i = 0;

	strcpy_s(strOutput, strDisplay);
	glRasterPos3f(x, y, z);
	for (i = 0; strOutput[i] != '\0'; i++)
		glutBitmapCharacter(font, strOutput[i]);
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

void  initEnvironment(void)
{
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

	//	bvh = new BVH( "B02.bvh" );
	bvh = new BVH("D:\\Storage\\GitHub\\BVH\\DancingBVH\\05_05.bvh");
}

void  drawMessage(int line_no, const char * message)
{
	int   i;
	if (message == NULL)
		return;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, win_width, win_height, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glColor3f(1.0, 0.0, 0.0);
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