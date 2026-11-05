#pragma once

#include <vector>

#include "Core/ResourceHandle.h"
#include "Render/GPUHandle.h"
#include "Render/Vertex.h"

namespace static_mesh
{
	inline std::vector<Vertex> getCube()
	{
		return {
			// positions          // normals           // texture coords
			{ -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f },
			{ 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f },
			{ 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f },
			{  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f },
			{ -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f },
			{ -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f },
	
			{ -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f },
			{ 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f },
			{ 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f },
			{ 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f },
			{ -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f },
			{ -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f },
	
			{ -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f },
			{ -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f },
			{ -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f },
			{ -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f },
			{ -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f },
			{ -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f },
	
			{ 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f },
			{ 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f },
			{ 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f },
			{ 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f },
			{ 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f },
			{ 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f },
	
			{ -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f },
			{ 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f },
			{ 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f },
			{ 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f },
			{ -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f },
			{ -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f },
	
			{ -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f },
			{ 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f },
			{ 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f },
			{ 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f },
			{ -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f },
			{ -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f }
		};
	}

	inline std::vector<Vertex> getPlaneVertices()
	{
		const std::vector<Vertex> vertices =
		{
			// pos					// normal			// texcoords
			{-0.5f, -0.5f, 0.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f},
			{ 0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f},
			{ 0.5f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f},
			{-0.5f,  0.5f, 0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f},
		};
		return vertices;
	}

	inline uint32_t getPlaneIndicesCount() { return 6; }
	inline std::vector<uint16_t> getPlaneIndices()
	{
		const std::vector<uint16_t> indices =
		{
			0, 1, 2, 2, 3, 0
		};
		return indices;
	}
}

class Mesh
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	std::vector<ResourceHandle> textures;

	GPUProxyHandle proxyHandle;

public:
	const std::vector<Vertex>& getVertices() const { return vertices; }
	uint32_t getVertexCount() const { return (uint32_t)vertices.size(); }
	const std::vector<uint16_t>& getIndices() const{ return indices; }
	uint32_t getIndexCount() const { return (uint32_t)indices.size(); }
	const std::vector<ResourceHandle>& getTextures() const{ return textures; }
	GPUProxyHandle getProxyHandle() const { return proxyHandle; }

	Mesh() = delete;
	Mesh(std::vector<Vertex>&& inVertices, std::vector<uint16_t>&& inIndices, std::vector<ResourceHandle>&& inTextures);
	~Mesh();

	Mesh(const Mesh& inOther) = delete;
	Mesh& operator=(const Mesh& inOther) = delete;

	Mesh(Mesh&& inOther) noexcept;
	Mesh& operator=(Mesh&& inOther) noexcept;
};