#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <vector>
#include "Mesh.h"
#include "Camera.h"

// Project includes
#include "maths_funcs.h"

// Assimp includes
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

class GraphicsEngine
{
	public:
		bool init(int argc, char** argv, int width, int height);
		Node* load_mesh(const char * file_name);
		void setRootNode(Node* node);
		void setCamera(Camera* camera);

		static GLuint shaderProgramID;
		static Camera* camera;
		static Node* root;

	private:
		char* readShaderSource(const char * shaderFile);
		void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
		GLuint CompileShaders();
		void initShaders();

};
