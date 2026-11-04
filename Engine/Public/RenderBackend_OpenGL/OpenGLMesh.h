#pragma once

#include <vector>

#include "Core/ResourceHandle.h"
#include "Render/GPUHandle.h"
#include "Render/Vertex.h"

class ResourcesManager;

class OpenGLMesh
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
	OpenGLMesh() = delete;
	OpenGLMesh(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures);
	~OpenGLMesh();
	
	OpenGLMesh(const OpenGLMesh& inOther) = delete;
	OpenGLMesh& operator=(const OpenGLMesh& inOther) = delete;

	OpenGLMesh(OpenGLMesh&& inOther) noexcept;
	OpenGLMesh& operator=(OpenGLMesh&& inOther) noexcept;

	void bindTextures(const ResourcesManager& inResourcesManager) const;
	
private:
	void setupMesh();
};