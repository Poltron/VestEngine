#include "RenderBackend_OpenGL/OpenGLMeshProxy.h"

#include "Core/ResourcesManager.h"
#include "RenderBackend/RenderBackend.h"

OpenGLMeshProxy::OpenGLMeshProxy(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures)
	: VAO(GPUAllocHandle::invalid), VBO(GPUAllocHandle::invalid), EBO(GPUAllocHandle::invalid)
{
	setupMesh(std::move(inVertices), std::move(inIndices), std::move(inTextures));
}

OpenGLMeshProxy::~OpenGLMeshProxy()
{
	//render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);
	//render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTICES, VBO);
	//render::getRenderBackend()->deleteGPUBuffer(EBufferType::INDICES, EBO);
}

OpenGLMeshProxy::OpenGLMeshProxy(OpenGLMeshProxy&& inOther) noexcept
	: VAO(inOther.VAO), VBO(inOther.VBO), EBO(inOther.EBO), indexCount(inOther.vertexCount), vertexCount(inOther.vertexCount)
{
	inOther.VAO = GPUAllocHandle::invalid;
	inOther.VBO = GPUAllocHandle::invalid;
	inOther.EBO = GPUAllocHandle::invalid;
}

OpenGLMeshProxy& OpenGLMeshProxy::operator=(OpenGLMeshProxy&& inOther) noexcept
{
	if (this != &inOther)
	{
		VAO = inOther.VAO;
		VBO = inOther.VBO;
		EBO = inOther.EBO;

		inOther.VAO = GPUAllocHandle::invalid;
		inOther.VBO = GPUAllocHandle::invalid;
		inOther.EBO = GPUAllocHandle::invalid;
	}
	return *this;
}

void OpenGLMeshProxy::setupMesh(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures)
{
	//VAO = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTEX_ARRAY);
	//VBO = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTICES);

	//// note : temporary for container mesh, to remove when only working with proper models
	//if (inIndices.size() > 0)
	//{
	//	EBO = render::getRenderBackend()->createGPUBuffer(EBufferType::INDICES);
	//}

	//render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);
	//render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTICES, VBO);
	//render::getRenderBackend()->allocateGPUBuffer(EBufferType::VERTICES, inVertices.size() * sizeof(Vertex), inVertices.data(), EBufferUsage::STATIC);
	//
	//// note : temporary for container mesh, to remove when only working with proper models
	//if (EBO.id > 0)
	//{
	//	render::getRenderBackend()->bindGPUBuffer(EBufferType::INDICES, EBO);
	//	render::getRenderBackend()->allocateGPUBuffer(EBufferType::INDICES, inIndices.size() * sizeof(unsigned int), inIndices.data(), EBufferUsage::STATIC);
	//}
	//
	//const unsigned int divisor = 0;
	//render::getRenderBackend()->setVertexLayoutData(0, 3, sizeof(Vertex), (void*)offsetof(Vertex, position), divisor);
	//render::getRenderBackend()->setVertexLayoutData(1, 3, sizeof(Vertex), (void*)offsetof(Vertex, normal), divisor);
	//render::getRenderBackend()->setVertexLayoutData(2, 2, sizeof(Vertex), (void*)offsetof(Vertex, texCoords), divisor);

	//render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);
}

// todo: use proxy
void OpenGLMeshProxy::bindTextures(const ResourcesManager& inResourcesManager) const
{
	//for (std::vector<ResourceHandle>::size_type i = 0; i < textures.size(); ++i)
	//{
	//	const ResourceHandle& textureHandle = textures[i];
	//	if (!textureHandle.isValid())
	//	{
	//		continue;
	//	}

	//	const Texture* texture = inResourcesManager.getTexture(textureHandle);
	//	if (!texture)
	//	{
	//		continue;
	//	}

	//	render::getRenderBackend()->bindTexture(texture->getHandle(), texture->getType(), i);
	//}
}