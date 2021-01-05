#include "Material.h"
#include <iostream>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Material::setAsMaterialToDraw()
{
    glUniform3f(material_ambient_location, ambient.v[0], ambient.v[1], ambient.v[2]);
    glUniform3f(material_diffuse_location, diffuse.v[0], diffuse.v[1], diffuse.v[2]);
    glUniform3f(material_specular_location, specular.v[0], specular.v[1], specular.v[2]);
}

Material* Material::fromAiMaterial(aiMaterial* aiMaterial, GLuint shaderProgramID)
{
	Material* material = new Material();

    aiColor3D color = {};
	// Adding Ambient
	aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
    material->ambient = vec3{color.r, color.g, color.b};
    material->material_ambient_location = glGetUniformLocation(shaderProgramID, "material_ambient");

    // Adding Diffuse
    aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    material->diffuse = vec3{ color.r, color.g, color.b };
    material->material_diffuse_location = glGetUniformLocation(shaderProgramID, "material_diffuse");

    // Adding Specular
    aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
    material->specular = vec3{ color.r, color.g, color.b };
    material->material_specular_location = glGetUniformLocation(shaderProgramID, "material_specular");

	// Texture
    /*int numTextures = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
    aiString textureName;

    if (numTextures > 0)
    {
        //Get the file name of the texture by passing the variable by reference again
        //Second param is 0, which is the first diffuse texture
        //There can be more diffuse textures but for now we are only interested in the first one
        aiReturn ret = aiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);

        std::string textureType = "diff_";
        std::string textureFileName = textureType + textureName.data;//The actual name of the texture file
    }*/

	return material;
}
