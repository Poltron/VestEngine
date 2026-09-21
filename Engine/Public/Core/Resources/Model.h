#pragma once

#include <string>
#include <vector>

#include "assimp/scene.h"

#include "Core/ResourceHandle.h"
#include "Core/Resources/Mesh.h"

class ResourcesManager;
class Shader;

class Model
{
public:
	Model() = delete;
	Model(std::string inPath);
	Model(std::vector<Mesh>&& inMeshes, std::string inName);
	~Model();

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

	void load(std::string inPath);

	void processNode(aiNode* inNode, const aiScene* inScene);
	Mesh processMesh(aiMesh* inMesh, const aiScene* inScene);
	void loadMaterialTextures(aiMaterial* inMat, aiTextureType inType, const char* inTypeName, std::vector<ResourceHandle>& outTextures);
};

