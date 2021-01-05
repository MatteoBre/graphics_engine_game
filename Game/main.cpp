// Windows includes (For Time, IO, etc.)
#define NOMINMAX
#include <limits>
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.
#include <time.h>

#include "Camera.h"

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"

//Include graphics engine
#include "GraphicsEngine.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define SPEED 0.02f
#define SECONDS_TO_EXPLORE 2
#define SECONDS_TO_PLAY 40

using namespace std;
GLuint shaderProgramID;

ModelData mesh_data;
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

Camera* camera;
Node* root;

// Lever variables
Node* lever;
mat4* leverMatrix;
Node* leverHandle;
bool leverActivated = false;
bool canActivateLever = false;
float leverAngle = 0.0f;

// Objects to find
int level1ObjsIndex = 0;
int lastLevel1ObjsIndex = -1;
string level1Objs[3];

// Countdown
clock_t time_started;
clock_t current_timestamp;
int last_typed = 0;

enum Mode {
	EXPLORE, PLAY
};
Mode mode = EXPLORE;

bool removeChildByName(Node* root, string name) {
	if (root == NULL)
		return false;
	if (root->getName() == name)
		return true;

	bool result;
	for (int i = 0; i < root->getNumberOfChildren(); i++) {
		result = removeChildByName(root->getChildren()[i], name);
		if (result == true) {
			root->getChildren().erase(root->getChildren().begin() + i);
			break;
		}
	}

	return false;
}

void activateLever() {
	if (!leverActivated || leverAngle >= 3.14f)
		return;

	float angleSensibility = 0.01f;
	glm::mat4 leverHandleMatrix = toGlm(leverHandle->getMatrix());
	leverHandleMatrix = glm::rotate(leverHandleMatrix, angleSensibility, glm::vec3(1.0f, 0.0f, 0.0f));
	leverAngle += angleSensibility;
	leverHandle->setMatrix(toMathFunctionLib(leverHandleMatrix));

	// remove wall from level1
	if (leverAngle >= 3.14f) {
		removeChildByName(root, "WallToRemove");
	}
}

Node* findByName(Node* root, string name) {
	if (root == NULL)
		return NULL;
	if (root->getName() == name)
		return root;

	Node* result;
	for (int i = 0; i < root->getNumberOfChildren(); i++) {
		result = findByName(root->getChildren()[i], name);
		if (result != NULL)
			return result;
	}

	return NULL;
}

mat4* findMatrixByNameOfNode(Node* root, string name, std::vector<mat4> matrixHierarchy) {
	if (root == NULL)
		return NULL;
	matrixHierarchy.push_back(root->getMatrix());
	if (root->getName() == name)
		return new mat4(computeMatrix(matrixHierarchy));

	mat4* result;
	for (int i = 0; i < root->getNumberOfChildren(); i++) {
		result = findMatrixByNameOfNode(root->getChildren()[i], name, matrixHierarchy);
		if (result != NULL)
			return result;
	}

	matrixHierarchy.pop_back();

	return NULL;
}

void assignLeverComponents(Node* root) {
	lever = findByName(root, "Lever");
	leverHandle = findByName(root, "Handle");
	std::vector<mat4> matrixHierarchy = {};
	leverMatrix = findMatrixByNameOfNode(root, "Lever", matrixHierarchy);
}

void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Draw the next frame
	glutPostRedisplay();
}

float euclideanDistance(vec3 v1, vec3 v2) {
	float sum = 0;
	for (int i = 0; i < 3; i++) {
		sum += std::pow(v1.v[i] - v2.v[i], 2);
	}
	return std::sqrt(sum);
}

void keyboardCallback(unsigned char key, int x, int y) {
	if (key == 'w') {
		camera->ProcessKeyboard(FORWARD, SPEED);
	}
	if (key == 's') {
		camera->ProcessKeyboard(BACKWARD, SPEED);
	}
	if (key == 'a') {
		camera->ProcessKeyboard(LEFT, SPEED);
	}
	if (key == 'd') {
		camera->ProcessKeyboard(RIGHT, SPEED);
	}
	if (key == 'l' && canActivateLever) {
		vec3 v1 = vec3(leverMatrix->m[12], leverMatrix->m[13], leverMatrix->m[14]);
		vec3 v2 = vec3(camera->position.x, camera->position.y, camera->position.z);
		if (euclideanDistance(v1, v2) < 1.5f && !leverActivated) {
			leverActivated = true;
			cout << "Lever activated! A new passage is available." << endl;
		}
	}
}

GLfloat lastX = width / 2.0;
GLfloat lastY = height / 2.0;

void mouseCallback(int button, int state, int x, int y) {
	if (state == 0) {
		lastX = x;
		lastY = y;
	}
}

void cameraUpdateMouse(int x, int y) {
	GLfloat xOffset = x - lastX;
	GLfloat yOffset = lastY - y;

	lastX = x;
	lastY = y;

	camera->ProcessMouseMovement(xOffset, yOffset);
}

void populateLevel1Objs() {
	vector<string> values = {};
	values.push_back("Cat");
	values.push_back("Deer");
	values.push_back("Sword");

	int index;
	for (int i = 0; i < 3; i++) {
		index = rand() % values.size();
		level1Objs[i] = values.at(index);
		values.erase(values.begin() + index);
	}
}

void resetLevel1() {
	leverActivated = false;

	glm::mat4 leverHandleMatrix = toGlm(leverHandle->getMatrix());
	leverHandleMatrix = glm::rotate(leverHandleMatrix, 6.28f - leverAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	leverHandle->setMatrix(toMathFunctionLib(leverHandleMatrix));

	leverAngle = 0.0f;

	level1ObjsIndex = 0;
	populateLevel1Objs();

	mode = EXPLORE;
	canActivateLever = false;
}

void restartPosition() {
	camera->position = glm::vec3(9.0f, 1.8f, 0.0f);
}

void startPlayRound() {
	cout << "You can start playing now, find all the objects/animals"<< endl;
	cout << "Good Luck!" << endl;
}

void typeOnScreen(int val) {
	if (val == last_typed) {
		return;
	}
	last_typed = val;
	cout << last_typed << endl;
}

void updateCountdowns() {
	current_timestamp = clock() - time_started;

	if (current_timestamp > SECONDS_TO_EXPLORE * 1000 && mode != PLAY) {
		mode = PLAY;
		canActivateLever = true;
		last_typed = 0;
		restartPosition();
		startPlayRound();
	}

	int secondsToType = current_timestamp / 1000;
	if (mode != EXPLORE) {
		secondsToType -= SECONDS_TO_EXPLORE;
	}

	typeOnScreen(secondsToType);
}

void gameplay() {
	if (mode == PLAY) {
		if (lastLevel1ObjsIndex != level1ObjsIndex) {
			lastLevel1ObjsIndex = level1ObjsIndex;
			cout << "Find the " << level1Objs[level1ObjsIndex] << endl;
		}
	}
}

int main(int argc, char** argv) {
	//Create the Graphics Engine
	GraphicsEngine graphicsEngine = GraphicsEngine{};

	//Initialize Graphics Engine, if initialization fails, exit with code 1
	if(!graphicsEngine.init(argc, argv, width, height)) {
		return 1;
	}

	srand(time(NULL));
	populateLevel1Objs();

	root = graphicsEngine.load_mesh("level1.dae");
	graphicsEngine.setRootNode(root);

	camera = new Camera{ glm::vec3(9.0f, 1.8f, 0.0f) };
	camera->fixPositionInY(1.8f);

	// Mouse sensitivity is 0.25, I need a -90 degrees rotation, so -90/0.25 = -360
	camera->ProcessMouseMovement(-360.0f, 0.0f);
	graphicsEngine.setCamera(camera);

	glutIdleFunc(updateScene);
	glutMouseFunc(mouseCallback);
	glutMotionFunc(cameraUpdateMouse);
	glutKeyboardFunc(keyboardCallback);

	assignLeverComponents(root);

	cout << "Welcome to the Labyrinth!\n" << endl;
	cout << "You have " << SECONDS_TO_EXPLORE << " seconds to explore" << endl;
	cout << "You cannot pull levers at this point" << endl;
	time_started = clock();
	current_timestamp = clock() - time_started;

	// Loop
	while (true) {
		glutMainLoopEvent();
		glutPostRedisplay();
		activateLever();
		updateCountdowns();
		gameplay();
	}
	return 0;
}
