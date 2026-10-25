#pragma once

#include "Render/Vertex.h"
#include "RenderBackend/RenderBackendDef.h"

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

#ifdef RENDER_OPENGL
	#include "RenderBackend_OpenGL/Mesh_OpenGL.h"
	using Mesh = Mesh_OpenGL;
#elif defined(RENDER_VULKAN)
	#include "RenderBackend_Vulkan/Mesh_Vulkan.h"
	using Mesh = Mesh_Vulkan;
#else
	#include "Core/Ensure.h"
	ensure(false);
#endif