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
	Model() = delete;
	Model(const char* inPath, ResourcesManager& inResourcesManager);
	Model(std::vector<Mesh>&& inMeshes, const char* inName);
	~Model() = default;

	Model(const Model& inOther) = delete;
	Model& operator=(const Model& inOther) = delete;

	Model(Model&& inOther) noexcept;
	Model& operator=(Model&& inOther) noexcept;

	void bindTextures(const ResourcesManager& inResourcesManager, const Shader& inShader) const;
	void draw() const;

	const std::string& getPath() const { return path; }

private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::string path;

	void load(const std::string& inPath, ResourcesManager& inResourcesManager);

	void processNode(aiNode* inNode, const aiScene* inScene, ResourcesManager& inResourcesManager);
	Mesh processMesh(aiMesh* inMesh, const aiScene* inScene, ResourcesManager& inResourcesManager);
	void loadMaterialTextures(aiMaterial* inMat, aiTextureType inType, const char* inTypeName, ResourcesManager& inResourcesManager, std::vector<ResourceHandle>& outTextures);
};

