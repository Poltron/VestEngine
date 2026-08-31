#pragma once

#include <unordered_map>
#include <vector>

#include "Resources/Model.h"
#include "Resources/ResourceHandle.h"
#include "Resources/Shader.h"
#include "Resources/Texture.h"

#define MAX_RESOURCES 10

class ResourcesManager
{
public:
	ResourcesManager()
	{
		models.reserve(MAX_RESOURCES);
		shaders.reserve(MAX_RESOURCES);
		textures.reserve(MAX_RESOURCES);
	}

	ResourceHandle loadTexture(const char* inPath, const char* inType);
	ResourceHandle loadShader(const char* inVertexPath, const char* inFragmentPath);
	ResourceHandle loadModel(const char* inPath);
	ResourceHandle createModel(std::vector<Mesh>&& inMeshes, const char* inName);

	Texture* getTexture(ResourceHandle handle);
	const Texture* getTexture(ResourceHandle handle) const;
	Model* getModel(ResourceHandle handle);
	const Model* getModel(ResourceHandle handle) const;
	Shader* getShader(ResourceHandle handle);
	const Shader* getShader(ResourceHandle handle) const;

private:
	// note : vector is temporary, currently unsafe if vector is reallocated

	uint32_t textureHandleCounter = 1;
	std::unordered_map<ResourceHandle, Texture*> texturesLookup;
	std::vector<Texture> textures;

	uint32_t modelHandleCounter = 1;
	std::unordered_map<ResourceHandle, Model*> modelsLookup;
	std::vector<Model> models;

	uint32_t shaderHandleCounter = 1;
	std::unordered_map<ResourceHandle, Shader*> shadersLookup;
	std::vector<Shader> shaders;
};

