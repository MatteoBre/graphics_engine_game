#include "GraphicsEngine.h"
#include "maths_funcs.h"

#include <iostream>
#include "ModelData.h"
#include "Node.h"
#include "Light.h"
#include "Camera.h"
#include <set>
#include <string>

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
std::vector<Material*> GraphicsEngine::materials = std::vector<Material*>{};

void drawTree(Node* root, std::vector<mat4> matrixHierarchy) {
	matrixHierarchy.push_back(root->getMatrix());

	root->draw(computeMatrix(matrixHierarchy));

	for (Node * child : root->getChildren()) {
		drawTree(child, matrixHierarchy);
	}

	matrixHierarchy.pop_back();
}

void getAllLights(Node* root, std::vector<mat4> matrixHierarchy, std::vector<Light*>& lights, std::vector<mat4>& matrices) {
	matrixHierarchy.push_back(root->getMatrix());

	if (root->getNodeType() == LIGHT) {
		lights.push_back(dynamic_cast<Light*>(root));
		mat4 matrix = computeMatrix(matrixHierarchy);
		matrices.push_back(matrix);
	}

	for (Node* child : root->getChildren()) {
		getAllLights(child, matrixHierarchy, lights, matrices);
	}

	matrixHierarchy.pop_back();
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
	int light_pos_location = glGetUniformLocation(GraphicsEngine::shaderProgramID, "lightPos");
	int light_intensity = glGetUniformLocation(GraphicsEngine::shaderProgramID, "light_intensity");

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);

	// Get all lights from the tree
	std::vector<Light*> lights = {};
	std::vector<mat4> matrices = {};
	getAllLights(GraphicsEngine::root, std::vector<mat4>(), lights, matrices);
	// Draw tree for each light
	glUniform1f(light_intensity, 1.0f/ lights.size());
	glDepthFunc(GL_LEQUAL);
	for (int i = 0; i < lights.size(); i++) {
		if (i >= 1)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
		}
		Light* light = lights.at(i);
		mat4 lightMat = matrices.at(i);
		lightMat = view * lightMat;
		// 12, 13, 14 of the matrix correspond to the x y z of the light
		glUniform3f(light_pos_location, lightMat.m[12], lightMat.m[13], lightMat.m[14]);
		drawTree(GraphicsEngine::root, std::vector<mat4>());
	}
	glDisable(GL_BLEND);

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

Node* createTree(const aiNode* root, const aiScene* scene, std::set<std::string> lightNames) {
	ModelData modelData;
	modelData.mPointCount = 0;
	bool isMesh = root->mNumMeshes > 0;
	for (unsigned int m_i = 0; m_i < root->mNumMeshes; m_i++) {
		int index = root->mMeshes[m_i];
		const aiMesh* mesh = scene->mMeshes[index];
		modelData.mPointCount += mesh->mNumVertices;
		modelData.materialIndex = mesh->mMaterialIndex;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions() && mesh->HasNormals()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
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

	Node* currentNode;
	mat4 matrix = getMat4(root->mTransformation);
	if (isMesh) {
		Mesh* mesh = Mesh::fromModelData(modelData, matrix, MESH, GraphicsEngine::shaderProgramID, GraphicsEngine::materials.at(modelData.materialIndex));
		currentNode = mesh;
	}
	else if (lightNames.find(root->mName.C_Str()) != lightNames.end()) {
		currentNode = new Light(matrix);
	} else {
		currentNode = new Node(matrix, NODE);
	}

	currentNode->setName(root->mName.C_Str());

	for (int i = 0; i < root->mNumChildren; i++) {
		Node* child = createTree(root->mChildren[i], scene, lightNames);
		currentNode->addChild(child);
	}

	return currentNode;
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

	// Materials extraction
	aiMaterial* material;
	for (int i = 0; i < scene->mNumMaterials; i++) {
		material = scene->mMaterials[i];
		materials.push_back(Material::fromAiMaterial(material, GraphicsEngine::shaderProgramID));
	}

	// Lights
	std::set<std::string> lightNames;
	for (int i = 0; i < scene->mNumLights; i++) {
		lightNames.insert(scene->mLights[i]->mName.C_Str());
	}

	Node* treeRoot = createTree(assimpRoot, scene, lightNames);

	// Textures extraction
	aiTexture* texture;
	for (int i = 0; i < scene->mNumTextures; i++) {
		texture = scene->mTextures[i];
	}

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
