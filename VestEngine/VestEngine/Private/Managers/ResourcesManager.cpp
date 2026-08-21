#include "Managers/ResourcesManager.h"


ResourceHandle ResourcesManager::loadTexture(const char* inPath, GLenum inTextureFormat, const std::string& inType)
{
	ResourceHandle handle(textureHandleCounter);
	textures.push_back(Texture());

	Texture& texture = textures.back();
	texture.loadTexture(inPath, inTextureFormat, inType);

	texturesLookup.insert({ handle, &texture });
	textureHandleCounter++;

	return handle;
}

ResourceHandle ResourcesManager::loadMesh(const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices, const std::vector<Texture>& inTextures)
{
	ResourceHandle handle(meshHandleCounter);
	meshes.push_back(Mesh());

	Mesh& mesh = meshes.back();
	mesh.loadMesh(inVertices, inIndices, inTextures);
	meshesLookup.insert({ handle, &mesh });
	meshHandleCounter++;

	return handle;
}

ResourceHandle ResourcesManager::loadMesh(const char* inPath)
{
	ResourceHandle handle(meshHandleCounter);
	meshes.push_back(Mesh());

	Mesh& mesh = meshes.back();
	mesh.loadMesh(inPath);

	meshesLookup.insert({ handle, &mesh });
	meshHandleCounter++;

	return handle;
}

ResourceHandle ResourcesManager::loadShader(const char* inVertexPath, const char* inFragmentPath)
{
	ResourceHandle handle(shaderHandleCounter);
	shaders.push_back(Shader());

	Shader& shader = shaders.back();
	shader.load(inVertexPath, inFragmentPath);

	shadersLookup.insert({ handle, &shader });
	shaderHandleCounter++;

	return handle;
}

Texture* ResourcesManager::getTexture(ResourceHandle handle)
{
	return texturesLookup.at(handle);
}

const Texture* ResourcesManager::getTexture(ResourceHandle handle) const
{
	return texturesLookup.at(handle);
}

Mesh* ResourcesManager::getMesh(ResourceHandle handle)
{
	return meshesLookup.at(handle);
}

const Mesh* ResourcesManager::getMesh(ResourceHandle handle) const 
{
	return meshesLookup.at(handle);
}

Shader* ResourcesManager::getShader(ResourceHandle handle)
{
	return shadersLookup.at(handle);
}

const Shader* ResourcesManager::getShader(ResourceHandle handle) const
{
	return shadersLookup.at(handle);
}