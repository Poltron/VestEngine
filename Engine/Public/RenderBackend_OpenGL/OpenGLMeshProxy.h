#pragma once

#include <vector>

#include "Core/ResourceHandle.h"
#include "Render/GPUHandle.h"
#include "Render/Vertex.h"

class ResourcesManager;

class OpenGLMeshProxy
{
	GPUAllocHandle VAO;
	GPUAllocHandle VBO;
	GPUAllocHandle EBO;

	uint32_t vertexCount;
	uint32_t indexCount;

public:
	GPUAllocHandle getVAO() const { return VAO; }
	GPUAllocHandle getVBO() const { return VBO; }
	GPUAllocHandle getEBO() const { return EBO; }

	size_t getVertexCount() const { return vertexCount; }
	size_t getIndexCount() const { return indexCount; }
	
public:
	OpenGLMeshProxy() = delete;
	OpenGLMeshProxy(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures);
	~OpenGLMeshProxy();
	
	OpenGLMeshProxy(const OpenGLMeshProxy& inOther) = delete;
	OpenGLMeshProxy& operator=(const OpenGLMeshProxy& inOther) = delete;

	OpenGLMeshProxy(OpenGLMeshProxy&& inOther) noexcept;
	OpenGLMeshProxy& operator=(OpenGLMeshProxy&& inOther) noexcept;

	void bindTextures(const ResourcesManager& inResourcesManager) const;
	
private:
	void setupMesh(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures);
};