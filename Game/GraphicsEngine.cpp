#include "GraphicsEngine.h"

#include <iostream>
#include "ModelData.h"
#include "Node.h"
#include "Camera.h"

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLuint GraphicsEngine::shaderProgramID = 0;
Node* GraphicsEngine::root = NULL;
Camera* GraphicsEngine::camera = NULL;

mat4 computeMatrix(std::vector<mat4> matrixHierarchy) {
	mat4 result = identity_mat4();
	for (int i = matrixHierarchy.size() - 1; i >= 0; i--) {
		result = matrixHierarchy.at(i) * result;
	}
	return result;
}

void drawTree(Node* root, std::vector<mat4> matrixHierarchy) {
	matrixHierarchy.push_back(root->getMatrix());

	root->draw(computeMatrix(matrixHierarchy));

	for (Node * child : root->getChildren()) {
		drawTree(child, matrixHierarchy);
	}

	matrixHierarchy.pop_back();
}

mat4 toMathFunctionLib(glm::mat4 m) {
	return mat4(m[0][0], m[1][0], m[2][0], m[3][0],
				m[0][1], m[1][1], m[2][1], m[3][1],
				m[0][2], m[1][2], m[2][2], m[3][2],
				m[0][3], m[1][3], m[2][3], m[3][3]
	);
}

void displayFunction()
{
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(GraphicsEngine::shaderProgramID);

	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	// Root of the Hierarchy
	mat4 view = toMathFunctionLib(GraphicsEngine::camera->GetViewMatrix());
	mat4 persp_proj = perspective(GraphicsEngine::camera->GetZoom(), (float)m_viewport[2] / (float)m_viewport[3], 0.1f, 1000.0f);
	mat4 model = identity_mat4();

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(GraphicsEngine::shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(GraphicsEngine::shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(GraphicsEngine::shaderProgramID, "proj");

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);

	//draw
	drawTree(GraphicsEngine::root, std::vector<mat4>());

	glutSwapBuffers();
}

bool GraphicsEngine::init(int argc, char** argv, int width, int height) {
	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Game");

	// Set up OpenGl functions
	glutDisplayFunc(displayFunction);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();

	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return false;
	}

	// Initialize Shaders
	initShaders();

	// Bind vao
	unsigned int vao = 0;
	glBindVertexArray(vao);

	return true;
}

static mat4 getMat4(aiMatrix4x4 m) {
	return mat4(m.a1, m.a2, m.a3, m.a4,
				m.b1, m.b2, m.b3, m.b4,
				m.c1, m.c2, m.c3, m.c4, 
				m.d1, m.d2, m.d3, m.d4
	);
}

Node* createTree(const aiNode* root, const aiScene* scene) {
	ModelData modelData;
	for (unsigned int m_i = 0; m_i < root->mNumMeshes; m_i++) {
		int index = root->mMeshes[m_i];
		const aiMesh* mesh = scene->mMeshes[index];
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
			}
		}
	}

	mat4 matrix = getMat4(root->mTransformation);
	Mesh* mesh = Mesh::fromModelData(modelData, matrix, MESH, GraphicsEngine::shaderProgramID);

	for (int i = 0; i < root->mNumChildren; i++) {
		Node* child = createTree(root->mChildren[i], scene);
		mesh->addChild(child);
	}

	return mesh;
}

Node* GraphicsEngine::load_mesh(const char* file_name) {
	ModelData modelData;

	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return NULL;
	}

	aiNode* assimpRoot = scene->mRootNode;
	Node* treeRoot = createTree(assimpRoot, scene);

	aiReleaseImport(scene);
	return treeRoot;
}

char* GraphicsEngine::readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


void GraphicsEngine::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint GraphicsEngine::CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

void GraphicsEngine::initShaders()
{
	shaderProgramID = CompileShaders();
}

void GraphicsEngine::setRootNode(Node * node)
{
	root = node;
}

void GraphicsEngine::setCamera(Camera* camera)
{
	GraphicsEngine::camera = camera;
}
