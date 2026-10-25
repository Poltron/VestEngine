#pragma once

#include <string>
#include <vector>

#include "assimp/scene.h"

#include "Core/ResourceHandle.h"
#include "Render/Resources/Mesh.h"

class ResourcesManager;
class Shader;

class Model
{
	std::vector<Mesh> meshes;
	std::string directory;
	std::string path;

public:
	const std::string& getPath() const { return path; }
	const std::vector<Mesh>& getMeshes() const { return meshes; }

	Model() = delete;
	Model(std::string inPath);
	Model(std::vector<Mesh>&& inMeshes, std::string inName);
	~Model();

	Model(const Model& inOther) = delete;
	Model& operator=(const Model& inOther) = delete;

	Model(Model&& inOther) noexcept;
	Model& operator=(Model&& inOther) noexcept;

private:
	void load(std::string inPath);

	void processNode(aiNode* inNode, const aiScene* inScene);
	Mesh processMesh(aiMesh* inMesh, const aiScene* inScene);
	void loadMaterialTextures(aiMaterial* inMat, aiTextureType inType, const char* inTypeName, std::vector<ResourceHandle>& outTextures);
};

