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

using namespace std;
GLuint shaderProgramID;

ModelData mesh_data;
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

Camera* camera;

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

int main(int argc, char** argv) {
	//Create the Graphics Engine
	GraphicsEngine graphicsEngine = GraphicsEngine{};

	//Initialize Graphics Engine, if initialization fails, exit with code 1
	if(!graphicsEngine.init(argc, argv, width, height)) {
		return 1;
	}
	Node* root = graphicsEngine.load_mesh("level1.dae");
	graphicsEngine.setRootNode(root);

	camera = new Camera{ glm::vec3(9.0f, 1.8f, 0.0f) };
	graphicsEngine.setCamera(camera);

	//shaderProgramID = graphicsEngine.shaderProgramID;

	// Tell glut where the display function is
	//glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutMouseFunc(mouseCallback);
	//glutPassiveMotionFunc(cameraUpdate);
	glutMotionFunc(cameraUpdateMouse);
	glutKeyboardFunc(keyboardCallback);

	// Set up your objects and shaders
	//init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
