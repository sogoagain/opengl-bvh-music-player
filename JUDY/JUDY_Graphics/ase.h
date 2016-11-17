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

struct Vertex {          // vertex ����ü, x, y, z �� ��ġ���� ���´�.
	float x, y, z;
};

struct Face {            // face ����ü, vertex 3���� �ε��� ������ ���´�.
	int vertexIndex[3];
};
struct Normal {          // normal ����ü, �� face ������ �������� x, y, z ���� ���´�.
	float faceNormal[3];
};
struct Color {           // color ����ü,  mesh�� wireframe_color �� r, g, b ������ ���´�.
	float r, g, b;
};

class Mesh {            // mesh Ŭ����
public:
	FILE* fp;               // -> ASE ������ ���� �� �ʿ��� ����������
	char lineBuffer[256];   // -> �� �پ� �о� ���̱� ���� ����
	char stringBuffer[40];  // -> �� ���ڿ� �� �о� ���̱� ���� ����

	float max;

	int vertexNum;          // -> vertex ����
	int faceNum;            // -> face ����
	Vertex* vertexList;     // -> vertex ����Ʈ
	Face* faceList;         // -> face ����Ʈ
	Normal* normalList;     // -> normal ����Ʈ
	Color wireframeColor;   // -> mesh �� wireframe color


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
	void clearAse() {        // -> vertex, face, normal ����Ʈ�� �����Ҵ� �����ϴ� �Լ�
		if (vertexNum != 0)
			delete[] vertexList;
		if (faceNum != 0) {
			delete[] faceList;
			delete[] normalList;
		}
	}
	void readAse(char* fileName) {         // -> ASE ���Ϸκ��� �����͸� �����ϴ� �Լ�
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
			if (strcmp(stringBuffer, "*MESH") == 0) {       //��*MESH����� ���ڰ� ���� ������ ��� ����
				fgets(lineBuffer, 256, fp);  // Read the Line  *TIMEVALUE  
				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_NUMVERTEX
				sscanf(lineBuffer, "%s%d", stringBuffer, &vertexNum);
				vertexList = new Vertex[vertexNum];       // vertex ����Ʈ �����Ҵ� 

				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_NUMFACES 
				sscanf(lineBuffer, "%s %d", stringBuffer, &faceNum);
				faceList = new Face[faceNum];           // face ����Ʈ �����Ҵ�
				normalList = new Normal[faceNum];       // normal ����Ʈ �����Ҵ� 

				fgets(lineBuffer, 256, fp);  // Read the Line  *MESH_VERTEX_LIST  
				sscanf(lineBuffer, "%s", stringBuffer);
				if (strcmp(stringBuffer, "*MESH_VERTEX_LIST") == 0) {
					for (i = 0; i < vertexNum; i++) {          // vertex �� x, y, z ������ ���� 
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
					for (i = 0; i < faceNum; i++) {            // face �� vertex �ε��� ������ ����
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
				if (strcmp(stringBuffer, "*MESH_NORMALS") == 0) {    // normal �� �������� ������ ����
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
