#pragma once
#include "maths_funcs.h"
// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

class Material
{
	public:
		void setAsMaterialToDraw();
		static Material* fromAiMaterial(aiMaterial* material, GLuint shaderProgramID);
	private:
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;

		int material_ambient_location;
		int material_diffuse_location;
		int material_specular_location;
		int material_shininess_location;
};
