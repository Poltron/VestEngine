#include "Core/Resources/Mesh.h"

#include "glm/detail/type_vec3.hpp"

#include "Core/ResourcesManager.h"
#include "Core/Resources/Mesh.h"
#include "Render/Renderer.h"

Mesh::Mesh(std::vector<Vertex>&& inVertices, std::vector<uint16_t>&& inIndices, std::vector<ResourceHandle>&& inTextures)
	: vertices(inVertices), indices(inIndices), textures(inTextures), proxyHandle(GPUProxyHandle::invalid)
{
	proxyHandle = render::getRenderer()->createMeshProxy(this);
}

Mesh::~Mesh()
{
	if (proxyHandle.isValid())
	{
		render::getRenderer()->destroyMeshProxy(proxyHandle);
	}
}

Mesh::Mesh(Mesh&& inOther) noexcept
	: vertices(inOther.vertices), indices(inOther.indices), textures(inOther.textures), proxyHandle(inOther.proxyHandle)
{
	inOther.vertices = {};
	inOther.indices = {};
	inOther.textures = {};
	inOther.proxyHandle = GPUProxyHandle::invalid;
}

Mesh& Mesh::operator=(Mesh&& inOther) noexcept
{
	if (this == &inOther)
	{
		vertices = inOther.vertices;
		indices = inOther.indices;
		textures = inOther.textures;
		proxyHandle = inOther.proxyHandle;
		
		inOther.vertices = {};
		inOther.indices = {};
		inOther.textures = {};
		inOther.proxyHandle = GPUProxyHandle::invalid;
	}
	return *this;
}