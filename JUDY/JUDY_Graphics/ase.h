#include <iostream>
#include <gl/glut.h>

using namespace std;

enum MESH_PART {
	BODY,
	HEAD,
	LEFT_ARM,
	LEFT_FOOT,
	LEFT_HAND,
	LEFT_LEG,
	RIGHT_ARM,
	RIGHT_FOOT,
	RIGHT_HAND,
	RIGHT_LEG
};

struct Vertex {          // vertex 구조체, x, y, z 의 위치값을 갖는다.
	float x, y, z;
};

struct Face {            // face 구조체, vertex 3개의 인덱스 정보를 갖는다.
	int vertexIndex[3];
};
struct Normal {          // normal 구조체, 각 face 에서의 법선벡터 x, y, z 값을 갖는다.
	float faceNormal[3];
};
struct Color {           // color 구조체,  mesh의 wireframe_color 의 r, g, b 정보를 갖는다.
	float r, g, b;
};

class Mesh {            // mesh 클래스
public:
	FILE* fp;               // -> ASE 파일을 여는 데 필요한 파일포인터
	char lineBuffer[256];   // -> 한 줄씩 읽어 들이기 위한 버퍼
	char stringBuffer[40];  // -> 한 문자열 씩 읽어 들이기 위한 버퍼

	float max;

	int vertexNum;          // -> vertex 개수
	int faceNum;            // -> face 개수
	Vertex* vertexList;     // -> vertex 리스트
	Face* faceList;         // -> face 리스트
	Normal* normalList;     // -> normal 리스트
	Color wireframeColor;   // -> mesh 의 wireframe color


public:
	Mesh() {
		vertexNum = 0;
		faceNum = 0;
		wireframeColor.r = 0.0;
		wireframeColor.g = 0.0;
		wireframeColor.b = 0.0;
	}
	~Mesh() {
		if (vertexNum != 0)
			delete[] vertexList;
		if (faceNum != 0) {
			delete[] faceList;
			delete[] normalList;
		}
	}
	void setColor(float _r, float _g, float _b) {
		wireframeColor.r = _r;
		wireframeColor.g = _g;
		wireframeColor.b = _b;
	}
	void clearAse() {        // -> vertex, face, normal 리스트의 동적할당 해제하는 함수
		if (vertexNum != 0)
			delete[] vertexList;
		if (faceNum != 0) {
			delete[] faceList;
			delete[] normalList;
		}
	}
	void readAse(char* fileName) {         // -> ASE 파일로부터 데이터를 저장하는 함수
		float _x;
		float _y;
		float _z;
		int i = 0;
		int j = 0;
		int num = 0;

		if ((fp = fopen(fileName, "r")) == NULL) {
			cout << "File is Not Found" << endl;
			return;
		}
		while (fgets(lineBuffer, 256, fp) != NULL)
		{
			sscanf(lineBuffer, "%s", stringBuffer);
			if (strcmp(stringBuffer, "*MESH") == 0) {       //“*MESH”라는 문자가 나올 때까지 계속 읽음
				fgets(lineBuffer, 256, fp);  // Read the Line  *TIMEVALUE  
				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_NUMVERTEX
				sscanf(lineBuffer, "%s%d", stringBuffer, &vertexNum);
				vertexList = new Vertex[vertexNum];       // vertex 리스트 동적할당 

				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_NUMFACES 
				sscanf(lineBuffer, "%s %d", stringBuffer, &faceNum);
				faceList = new Face[faceNum];           // face 리스트 동적할당
				normalList = new Normal[faceNum];       // normal 리스트 동적할당 

				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_VERTEX_LIST  
				sscanf(lineBuffer, "%s", stringBuffer);
				if (strcmp(stringBuffer, "*MESH_VERTEX_LIST") == 0) {
					for (i = 0; i < vertexNum; i++) {          // vertex 의 x, y, z 데이터 저장 
						fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_VERTEX_LIST  
						sscanf(lineBuffer, "%s%d%f%f%f", stringBuffer, &num, &_x, &_y, &_z);
						for (j = 0; j < 3; j++) {
							if (max < _x)
								max = _x;
							if (max < _y)
								max = _y;
							if (max < _z)
								max = _z;
						}
						vertexList[i].x = _x;
						vertexList[i].y = _z;
						vertexList[i].z = -1 * _y;
					}
				}

				fgets(lineBuffer, 256, fp);  // Read the Line '}'
				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_FACE_LIST   
				sscanf(lineBuffer, "%s", stringBuffer);
				if (strcmp(stringBuffer, "*MESH_FACE_LIST") == 0) {
					for (i = 0; i < faceNum; i++) {            // face 의 vertex 인덱스 데이터 저장
						fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_FACE_LIST   
						sscanf(lineBuffer, "%s%s%s%d%s%d%s%d%s%d%s%d%s%d%s",
							stringBuffer, stringBuffer, stringBuffer,
							&(faceList[i].vertexIndex[0]), stringBuffer,
							&(faceList[i].vertexIndex[1]), stringBuffer,
							&(faceList[i].vertexIndex[2]), stringBuffer,
							&num, stringBuffer, &num, stringBuffer, &num, stringBuffer);
					}
				}

				fgets(lineBuffer, 256, fp);  // Read the Line '}'
				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_NUMCVERTEX   

				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_NORMALS   
				sscanf(lineBuffer, "%s", stringBuffer);
				if (strcmp(stringBuffer, "*MESH_NORMALS") == 0) {    // normal 의 법선벡터 데이터 저장
					for (i = 0; i < faceNum; i++) {
						fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_FACENORMAL   
						sscanf(lineBuffer, "%s%d%f%f%f", stringBuffer, &num,
							&(normalList[i].faceNormal[0]), &(normalList[i].faceNormal[1]), &(normalList[i].faceNormal[2]));
						fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_VERTEXNORMAL 1 
						fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_VERTEXNORMAL 2 
						fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_VERTEXNORMAL 3
					}
				}
				break;
			}
		}
		fclose(fp);
	}
	void drawMesh(float size) {
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < this->faceNum; i++) {
			glNormal3f(this->normalList[i].faceNormal[0], this->normalList[i].faceNormal[1], this->normalList[i].faceNormal[2]);

			glVertex3f(this->vertexList[this->faceList[i].vertexIndex[0]].x / size,
				this->vertexList[this->faceList[i].vertexIndex[0]].y / size,
				this->vertexList[this->faceList[i].vertexIndex[0]].z / size);

			glVertex3f(this->vertexList[this->faceList[i].vertexIndex[1]].x / size,
				this->vertexList[this->faceList[i].vertexIndex[1]].y / size,
				this->vertexList[this->faceList[i].vertexIndex[1]].z / size);

			glVertex3f(this->vertexList[this->faceList[i].vertexIndex[2]].x / size,
				this->vertexList[this->faceList[i].vertexIndex[2]].y / size,
				this->vertexList[this->faceList[i].vertexIndex[2]].z / size);
		}
		glEnd();
	}
};
