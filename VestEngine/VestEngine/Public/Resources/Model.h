#pragma once

#include <string>
#include <vector>

#include "assimp/scene.h"

#include "Resources/Mesh.h"
#include "Resources/ResourceHandle.h"

class ResourcesManager;
class Shader;

class Model
{
public:
	void load(const std::string& inPath, ResourcesManager& inResourcesManager);
	void fill(std::vector<Mesh>& inMesh, const std::string& inName);

	void bindTextures(const ResourcesManager& inResourcesManager, const Shader& inShader) const;
	void draw() const;

	const std::string& getPath() const { return path; }

private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::string path;

	void processNode(aiNode* inNode, const aiScene* inScene, ResourcesManager& inResourcesManager);
	Mesh processMesh(aiMesh* inMesh, const aiScene* inScene, ResourcesManager& inResourcesManager);
	void loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, ResourcesManager& inResourcesManager, std::vector<ResourceHandle>& outTextures);
};

