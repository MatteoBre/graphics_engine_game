#include "Mesh.h"
// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>

Mesh::Mesh(mat4 matrix, Material* material)
	: Node(matrix, MESH)
{
	this->material = material;
}

void Mesh::init(GLuint shaderProgramID) {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	if (data.mPointCount == 0)
		return;

	vp_vbo = 0;
	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, data.mPointCount * sizeof(vec3), &data.mVertices[0], GL_STATIC_DRAW);

	vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, data.mPointCount * sizeof(vec3), &data.mNormals[0], GL_STATIC_DRAW);

	vt_vbo = 0;
	glGenBuffers (1, &vt_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, data.mPointCount * sizeof (vec2), &data.mTextureCoords[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	matrix_location = glGetUniformLocation(shaderProgramID, "model");
}

Mesh* Mesh::fromModelData(ModelData modelData, mat4 matrix, NodeType nodeType, GLuint shaderProgramID, Material* material)
{
	Mesh* mesh = new Mesh{matrix, material};
	mesh->data = modelData;
	mesh->init(shaderProgramID);
	return mesh;
}

void Mesh::draw(mat4 matrix)
{
	if (data.mPointCount == 0)
		return;

	material->setAsMaterialToDraw();

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, matrix.m);

	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glDrawArrays(GL_TRIANGLES, 0, data.mPointCount);
}
