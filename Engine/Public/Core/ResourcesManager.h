#pragma once

#include <unordered_map>
#include <vector>

#include "Core/ResourceHandle.h"
#include "Core/Resources/Model.h"
#include "Core/Resources/Material.h"
#include "Render/Resources/Texture.h"

#define MAX_RESOURCES 10

class ResourcesManager
{
public:
	ResourcesManager();
	~ResourcesManager();

	ResourceHandle loadTexture(std::string inPath, std::string inType);
	ResourceHandle loadMaterial();
	ResourceHandle loadModel(std::string inPath);
	ResourceHandle createMaterial(EMaterialBlendMode inBlendMode, EMaterialType inType);
	ResourceHandle createModel(std::vector<Mesh>&& inMeshes, std::string inName);

	Texture* getTexture(ResourceHandle handle);
	const Texture* getTexture(ResourceHandle handle) const;
	Model* getModel(ResourceHandle handle);
	const Model* getModel(ResourceHandle handle) const;
	Material* getMaterial(ResourceHandle handle);
	const Material* getMaterial(ResourceHandle handle) const;

private:
	// note : vector is temporary, currently unsafe if vector is reallocated since lookup table is not updated
	uint32_t textureHandleCounter = 0;
	std::unordered_map<ResourceHandle, Texture*> texturesLookup;
	std::vector<Texture> textures;

	uint32_t modelHandleCounter = 0;
	std::unordered_map<ResourceHandle, Model*> modelsLookup;
	std::vector<Model> models;

	uint32_t materialHandleCounter = 0;
	std::unordered_map<ResourceHandle, Material*> materialsLookup;
	std::vector<Material> materials;
};

