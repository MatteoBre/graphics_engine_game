#pragma once
#include "ModelData.h"
#include "Node.h"
// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

class Mesh : public Node
{
	public:
		Mesh(mat4 matrix, NodeType nodeType);
		static Mesh* fromModelData(ModelData modelData, mat4 matrix, NodeType nodeType, GLuint shaderProgramID);
		virtual void draw(mat4 matrix);

	private:
		void init(GLuint shaderProgramID);
		unsigned int vp_vbo, vn_vbo;
		int matrix_location;

		ModelData data;
		GLuint loc1, loc2, loc3;
};

