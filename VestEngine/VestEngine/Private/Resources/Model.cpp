#include "Resources/Model.h"

#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "Managers/ResourcesManager.h"
#include "Resources/Mesh.h"

// note : how should I access the resources manager ? what should i have access to globally ?
void Model::load(const std::string& inPath, ResourcesManager& inResourcesManager)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inPath, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR: Assimp " << importer.GetErrorString() << std::endl;
		return;
	}

	path = inPath;
	directory = inPath.substr(0, inPath.find_last_of('/'));
	processNode(scene->mRootNode, scene, inResourcesManager);
}

void Model::fill(std::vector<Mesh>& inMesh, const std::string& inDirectory)
{
	meshes = std::move(inMesh);
	directory = inDirectory;
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

void Model::processNode(aiNode* node, const aiScene* scene, ResourcesManager& inResourcesManager)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Mesh meshResource = processMesh(mesh, scene, inResourcesManager);
		meshes.push_back(meshResource);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene, inResourcesManager);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene, ResourcesManager& inResourcesManager)
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
		loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse", inResourcesManager, textures);
		loadMaterialTextures(material, aiTextureType_SPECULAR, "specular", inResourcesManager, textures);
	}

	return Mesh(vertices, indices, textures);
}

void Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, ResourcesManager& inResourcesManager, std::vector<ResourceHandle>& outTextures)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString relativePath;
		mat->GetTexture(type, i, &relativePath);

		// note: what would be the best way to merge two strings ?
		std::string absolutePath = directory.c_str();
		absolutePath.append("/");
		absolutePath.append(relativePath.C_Str());
		ResourceHandle textureHandle = inResourcesManager.loadTexture(absolutePath.c_str(), typeName);
		outTextures.push_back(textureHandle);
	}
}