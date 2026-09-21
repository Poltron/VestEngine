#include "Core/Resources/Model.h"

#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "Core/Engine.h"
#include "Core/ResourcesManager.h"
#include "Core/Resources/Mesh.h"


Model::Model(std::string inPath)
{
	load(std::move(inPath));
}

Model::Model(std::vector<Mesh>&& inMeshes, std::string inName)
{
	meshes = std::move(inMeshes);
	path = std::move(inName);
	directory = "";
}

Model::~Model()
{
	meshes.clear();
}

Model::Model(Model&& inOther) noexcept
	: meshes(std::move(inOther.meshes)), directory(std::move(inOther.directory)), path(std::move(inOther.path))
{
	inOther.meshes.clear();
	inOther.directory.clear();
	inOther.path.clear();
}

Model& Model::operator=(Model&& inOther) noexcept
{
	if (this == &inOther)
	{
		meshes = std::move(inOther.meshes);
		directory = inOther.directory;
		path = inOther.path;

		inOther.meshes.clear();
		inOther.directory.clear();
		inOther.path.clear();
	}
	return *this;
}

void Model::load(std::string inPath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inPath, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR: Assimp " << importer.GetErrorString() << std::endl;
		return;
	}

	path = std::move(inPath);
	directory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene);
}

void Model::bindTextures(const ResourcesManager& inResourcesManager, const Shader& inShader) const
{
	for (const Mesh& mesh : meshes)
	{
		mesh.bindTextures(inResourcesManager, inShader);
	}
}

void Model::draw() const
{
	for (const Mesh& mesh : meshes)
	{
		mesh.draw();
	}
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Mesh meshResource = processMesh(mesh, scene);
		meshes.push_back(std::move(meshResource));
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<ResourceHandle> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		if (aiVector3D* texCoords = mesh->mTextureCoords[0])
		{
			vertex.texCoords.x = texCoords[i].x;
			vertex.texCoords.y = texCoords[i].y;
		}
		
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse", textures);
		loadMaterialTextures(material, aiTextureType_SPECULAR, "specular", textures);
	}

	return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

void Model::loadMaterialTextures(aiMaterial* inMat, aiTextureType inType, const char* inTypeName, std::vector<ResourceHandle>& outTextures)
{
	for (unsigned int i = 0; i < inMat->GetTextureCount(inType); ++i)
	{
		aiString relativePath;
		inMat->GetTexture(inType, i, &relativePath);

		std::string absolutePath = directory + "/" + relativePath.C_Str();
		ResourceHandle textureHandle = engine::getResources()->loadTexture(absolutePath, inTypeName);
		outTextures.push_back(textureHandle);
	}
}