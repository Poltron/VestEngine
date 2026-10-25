#pragma once

#include <vector>

#include "Core/ResourceHandle.h"
#include "Render/GraphicResourceHandle.h"
#include "Render/Vertex.h"

class ResourcesManager;

class Mesh_OpenGL
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<ResourceHandle> textures;

	GPUHandle VAO;
	GPUHandle VBO;
	GPUHandle EBO;

public:
	GPUHandle getVAO() const { return VAO; }
	GPUHandle getVBO() const { return VBO; }
	GPUHandle getEBO() const { return EBO; }

	size_t getVerticesSize() const { return vertices.size(); }
	size_t getIndicesSize() const { return indices.size(); }
	
public:
	Mesh_OpenGL() = delete;
	Mesh_OpenGL(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures);
	~Mesh_OpenGL();
	
	Mesh_OpenGL(const Mesh_OpenGL& inOther) = delete;
	Mesh_OpenGL& operator=(const Mesh_OpenGL& inOther) = delete;

	Mesh_OpenGL(Mesh_OpenGL&& inOther) noexcept;
	Mesh_OpenGL& operator=(Mesh_OpenGL&& inOther) noexcept;

	void bindTextures(const ResourcesManager& inResourcesManager) const;
	
private:
	void setupMesh();
};