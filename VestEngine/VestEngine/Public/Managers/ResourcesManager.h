#pragma once

#include <unordered_map>
#include <vector>

#include "Resources/Mesh.h"
#include "Resources/Shader.h"
#include "Resources/Texture.h"

struct ResourceHandle
{
	uint32_t handle;

	ResourceHandle()
		: ResourceHandle(0)
	{}

	ResourceHandle(uint32_t inHandle)
		: handle(inHandle)
	{ }

	bool IsValid() const
	{
		return handle != 0;
	}

	bool operator==(const ResourceHandle& inOther) const
	{
		return handle == inOther.handle;
	}
};

template<>
struct std::hash<ResourceHandle>
{
	size_t operator()(const ResourceHandle& inResource) const
	{
		return std::hash<uint32_t>{}(inResource.handle);
	}
};

class ResourcesManager
{
public:
	ResourcesManager()
	{
		meshes.reserve(10);
		shaders.reserve(10);
		textures.reserve(10);
	}

	ResourceHandle loadTexture(const char* inPath, GLenum inTextureFormat, const std::string& inType);
	ResourceHandle loadMesh(const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices, const std::vector<Texture>& inTextures);
	ResourceHandle loadMesh(const char* inPath);
	ResourceHandle loadShader(const char* inVertexPath, const char* inFragmentPath);

	Texture* getTexture(ResourceHandle handle);
	const Texture* getTexture(ResourceHandle handle) const;
	Mesh* getMesh(ResourceHandle handle);
	const Mesh* getMesh(ResourceHandle handle) const;
	Shader* getShader(ResourceHandle handle);
	const Shader* getShader(ResourceHandle handle) const;

private:
	// note : vector is temporary, currently unsafe if vector is reallocated

	uint32_t textureHandleCounter = 1;
	std::unordered_map<ResourceHandle, Texture*> texturesLookup;
	std::vector<Texture> textures;

	uint32_t meshHandleCounter = 1;
	std::unordered_map<ResourceHandle, Mesh*> meshesLookup;
	std::vector<Mesh> meshes;

	uint32_t shaderHandleCounter = 1;
	std::unordered_map<ResourceHandle, Shader*> shadersLookup;
	std::vector<Shader> shaders;
};

