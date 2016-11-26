#include "BVH.h"
#include <fstream>
#include <string.h>

// 모델 파일 경로
const char* gMeshFilePath[] = { ".\\ASE\\E_Body.ASE",
						".\\ASE\\E_Head.ASE",
						".\\ASE\\E_LeftArm.ASE",
						".\\ASE\\E_LeftFoot.ASE",
						".\\ASE\\E_LeftHand.ASE",
						".\\ASE\\E_LeftLeg.ASE",
						".\\ASE\\E_RightArm.ASE",
						".\\ASE\\E_RightFoot.ASE",
						".\\ASE\\E_RightHand.ASE",
						".\\ASE\\E_RightLeg.ASE" };

BVH::BVH() {
	// ASE 파일로부터 모델링하는 Mesh객체 생성
	for (int i = 0; i < 10; i++) {
		mesh[i] = new Mesh();
		mesh[i]->readAse(gMeshFilePath[i]);
	}

	motion = NULL;
	Clear();
}

BVH::BVH(const char * bvh_file_name) {
	for (int i = 0; i < 10; i++) {
		mesh[i] = new Mesh(gMeshFilePath[i]);
	}

	motion = NULL;
	Clear();

	Load(bvh_file_name);
}

BVH::~BVH() {
	for (int i = 0; i < 10; i++)
		delete(mesh[i]);
	Clear();
}

void  BVH::Clear() {
	int  i;
	for (i = 0; i < channels.size(); i++)
		delete  channels[i];
	for (i = 0; i < joints.size(); i++)
		delete  joints[i];
	if (motion != NULL)
		delete  motion;

	is_load_success = false;

	file_name = "";
	motion_name = "";

	num_channel = 0;
	channels.clear();
	joints.clear();
	joint_index.clear();

	num_frame = 0;
	interval = 0.0;
	motion = NULL;
}

void  BVH::Load(const char * bvh_file_name) {
#define  BUFFER_LENGTH  1024*4

	ifstream  file;
	char      line[BUFFER_LENGTH];
	char *    token;
	char      separater[] = " :,\t";
	vector< Joint * >   joint_stack;
	Joint *   joint = NULL;
	Joint *   new_joint = NULL;
	bool      is_site = false;
	double    x, y, z;
	int       i, j;

	Clear();

	file_name = bvh_file_name;
	const char *  mn_first = bvh_file_name;
	const char *  mn_last = bvh_file_name + strlen(bvh_file_name);
	if (strrchr(bvh_file_name, '\\') != NULL)
		mn_first = strrchr(bvh_file_name, '\\') + 1;
	else if (strrchr(bvh_file_name, '/') != NULL)
		mn_first = strrchr(bvh_file_name, '/') + 1;
	if (strrchr(bvh_file_name, '.') != NULL)
		mn_last = strrchr(bvh_file_name, '.');
	if (mn_last < mn_first)
		mn_last = bvh_file_name + strlen(bvh_file_name);
	motion_name.assign(mn_first, mn_last);

	file.open(bvh_file_name, ios::in);
	if (file.is_open() == 0)  return;

	while (!file.eof()) {
		if (file.eof())  goto bvh_error;

		file.getline(line, BUFFER_LENGTH);
		token = strtok(line, separater);

		if (token == NULL)  continue;

		if (strcmp(token, "{") == 0) {
			joint_stack.push_back(joint);
			joint = new_joint;
			continue;
		}

		if (strcmp(token, "}") == 0) {
			joint = joint_stack.back();
			joint_stack.pop_back();
			is_site = false;
			continue;
		}

		if ((strcmp(token, "ROOT") == 0) ||
			(strcmp(token, "JOINT") == 0)) {
			new_joint = new Joint();
			new_joint->index = joints.size();
			new_joint->parent = joint;
			new_joint->has_site = false;
			new_joint->offset[0] = 0.0;  new_joint->offset[1] = 0.0;  new_joint->offset[2] = 0.0;
			new_joint->site[0] = 0.0;  new_joint->site[1] = 0.0;  new_joint->site[2] = 0.0;
			joints.push_back(new_joint);
			if (joint)
				joint->children.push_back(new_joint);

			token = strtok(NULL, "");
			while (*token == ' ')  token++;
			new_joint->name = token;

			joint_index[new_joint->name] = new_joint;
			continue;
		}

		if ((strcmp(token, "End") == 0)) {
			new_joint = joint;
			is_site = true;
			continue;
		}

		if (strcmp(token, "OFFSET") == 0) {
			token = strtok(NULL, separater);
			x = token ? atof(token) : 0.0;
			token = strtok(NULL, separater);
			y = token ? atof(token) : 0.0;
			token = strtok(NULL, separater);
			z = token ? atof(token) : 0.0;

			if (is_site) {
				joint->has_site = true;
				joint->site[0] = x;
				joint->site[1] = y;
				joint->site[2] = z;
			}
			else {
				joint->offset[0] = x;
				joint->offset[1] = y;
				joint->offset[2] = z;
			}
			continue;
		}

		if (strcmp(token, "CHANNELS") == 0) {
			token = strtok(NULL, separater);
			joint->channels.resize(token ? atoi(token) : 0);

			for (i = 0; i < joint->channels.size(); i++) {
				Channel *  channel = new Channel();
				channel->joint = joint;
				channel->index = channels.size();
				channels.push_back(channel);
				joint->channels[i] = channel;

				token = strtok(NULL, separater);
				if (strcmp(token, "Xrotation") == 0)
					channel->type = X_ROTATION;
				else if (strcmp(token, "Yrotation") == 0)
					channel->type = Y_ROTATION;
				else if (strcmp(token, "Zrotation") == 0)
					channel->type = Z_ROTATION;
				else if (strcmp(token, "Xposition") == 0)
					channel->type = X_POSITION;
				else if (strcmp(token, "Yposition") == 0)
					channel->type = Y_POSITION;
				else if (strcmp(token, "Zposition") == 0)
					channel->type = Z_POSITION;
			}
		}

		if (strcmp(token, "MOTION") == 0)
			break;
	}

	file.getline(line, BUFFER_LENGTH);
	token = strtok(line, separater);
	if (strcmp(token, "Frames") != 0)  goto bvh_error;
	token = strtok(NULL, separater);
	if (token == NULL)  goto bvh_error;
	num_frame = atoi(token);

	file.getline(line, BUFFER_LENGTH);
	token = strtok(line, ":");
	if (strcmp(token, "Frame Time") != 0)  goto bvh_error;
	token = strtok(NULL, separater);
	if (token == NULL)  goto bvh_error;
	interval = atof(token);

	num_channel = channels.size();
	motion = new double[num_frame * num_channel];

	for (i = 0; i < num_frame; i++) {
		file.getline(line, BUFFER_LENGTH);
		token = strtok(line, separater);
		for (j = 0; j < num_channel; j++) {
			if (token == NULL)
				goto bvh_error;
			motion[i*num_channel + j] = atof(token);
			token = strtok(NULL, separater);
		}
	}

	file.close();
	is_load_success = true;

	return;

bvh_error:
	file.close();
}

#include <math.h>
#include <gl/glut.h>

void  BVH::RenderFigure(int frame_no, float scale) {
	RenderFigure(joints[0], motion + frame_no * num_channel, scale);
}


void  BVH::RenderFigure(const Joint * joint, const double * data, float scale) {

	glPushMatrix();

	if (joint->parent == NULL) {
		glTranslatef(data[0] * scale, data[1] * scale, data[2] * scale);
	}
	else {
		glTranslatef(joint->offset[0] * scale, joint->offset[1] * scale, joint->offset[2] * scale);
	}

	int  i, j;

	for (i = 0; i < joint->channels.size(); i++) {
		Channel *  channel = joint->channels[i];
		if (channel->type == X_ROTATION)
			glRotatef(data[channel->index], 1.0f, 0.0f, 0.0f);
		else if (channel->type == Y_ROTATION)
			glRotatef(data[channel->index], 0.0f, 1.0f, 0.0f);
		else if (channel->type == Z_ROTATION)
			glRotatef(data[channel->index], 0.0f, 0.0f, 1.0f);
	}

	if (joint->children.size() == 0) {
		if (!(joint->name).compare("Head")) {
			glColor3ub(153, 153, 153);
			glTranslatef(0.0, 0.0, 0.5);
			glRotatef(-30, 1, 0, 0);
			mesh[HEAD]->drawMesh(100);
		}
	}
	if (joint->children.size() == 1) {
		Joint *  child = joint->children[0];

		if (!(child->name).compare("LeftLeg")) {
			glPushMatrix();
			glColor3ub(051, 051, 153);
			glTranslatef(0.25, 0.2, 0.0);
			glRotatef(25, 0, 0, 1);
			mesh[LEFT_LEG]->drawMesh(100);
			glPopMatrix();
		}
		if (!(child->name).compare("RightLeg")) {
			glPushMatrix();
			glColor3ub(051, 051, 153);
			glTranslatef(-0.3, 0.2, 0.0);
			glRotatef(-25, 0, 0, 1);
			mesh[RIGHT_LEG]->drawMesh(100);
			glPopMatrix();
		}
		if (!(child->name).compare("LeftHand")) {
			glPushMatrix();
			glColor3ub(102, 000, 102);
			glRotatef(90, 0, 0, 1);
			mesh[LEFT_HAND]->drawMesh(100);
			glPopMatrix();
		}
		if (!(child->name).compare("RightHand")) {
			glPushMatrix();
			glColor3ub(102, 000, 102);
			glRotatef(-90, 0, 0, 1);
			mesh[RIGHT_HAND]->drawMesh(100);
			glPopMatrix();
		}
		if (!(child->name).compare("LeftForeArm")) {
			glPushMatrix();
			glColor3ub(204, 000, 204);
			glRotatef(90, 0, 0, 1);
			mesh[LEFT_ARM]->drawMesh(100);
			glPopMatrix();
		}
		if (!(child->name).compare("RightForeArm")) {
			glPushMatrix();
			glColor3ub(204, 000, 204);
			glRotatef(-90, 0, 0, 1);
			mesh[RIGHT_ARM]->drawMesh(100);
			glPopMatrix();
		}
		if (!(child->name).compare("LeftFoot")) {
			glPushMatrix();
			glColor3ub(51, 51, 51);
			glTranslatef(0.6, 0.0, 0.0);
			glRotatef(25, 0, 0, 1);
			mesh[LEFT_FOOT]->drawMesh(100);
			glPopMatrix();
		}
		if (!(child->name).compare("RightFoot")) {
			glPushMatrix();
			glColor3ub(51, 51, 51);
			glTranslatef(-0.6, 0.0, 0.0);
			glRotatef(-25, 0, 0, 1);
			mesh[RIGHT_FOOT]->drawMesh(100);
			glPopMatrix();
		}
		if (!(child->name).compare("Spine")) {
			glPushMatrix();
			glColor3ub(204, 204, 051);
			glTranslatef(0.0, -0.2, 0.0);
			mesh[BODY]->drawMesh(100);
			glPopMatrix();
		}

	}

	for (i = 0; i < joint->children.size(); i++) {
		RenderFigure(joint->children[i], data, scale);
	}
	glPopMatrix();
}
