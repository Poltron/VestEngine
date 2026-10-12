#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include "vulkan/vulkan.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyVulkan.hpp"

#include "Core/Ensure.h"
#include "Platform/WindowHints.h"
#include "Platform/WindowManager.h"
#include "RenderBackend/RenderBackend.h"
#include "Render/GraphicResourceHandle.h"

struct RenderBackend::Impl
{
};

namespace
{
	std::string readFile(const char* inFileName)
	{
		std::string line, text;

		std::ifstream in(inFileName);
		if (in.fail())
		{
			std::cout << "ERROR: Could not read file " << inFileName << std::endl;
			return text;
		}

		while (std::getline(in, line))
		{
			text += line + "\n";
		}
		return text;
	}

	unsigned int getVkPrimitiveMode(EPrimitiveMode inMode)
	{
		switch (inMode)
		{
		case EPrimitiveMode::POINTS:
			return 0;
			break;
		case EPrimitiveMode::LINES:
			return 0;
			break;
		case EPrimitiveMode::TRIANGLES:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	unsigned int getVkBufferType(EBufferType inType)
	{
		switch (inType)
		{
		case EBufferType::VERTEX_ARRAY:
			ensure(false);
			return 0; // should not be needed
			break;
		case EBufferType::VERTICES:
			return 0;
			break;
		case EBufferType::INDICES:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	unsigned int getVkBufferUsage(EBufferUsage inUsage)
	{
		switch (inUsage)
		{
		case EBufferUsage::STATIC:
			return 0;
			break;
		case EBufferUsage::STREAM:
			return 0;
			break;
		case EBufferUsage::DYNAMIC:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	unsigned int getVkComparisonFunc(EComparisonFunc inFunc)
	{
		switch (inFunc)
		{
		case EComparisonFunc::NEVER:
			return 0;
			break;
		case EComparisonFunc::LESS:
			return 0;
			break;
		case EComparisonFunc::LEQUAL:
			return 0;
			break;
		case EComparisonFunc::EQUAL:
			return 0;
			break;
		case EComparisonFunc::NOTEQUAL:
			return 0;
			break;
		case EComparisonFunc::GEQUAL:
			return 0;
			break;
		case EComparisonFunc::GREATER:
			return 0;
			break;
		case EComparisonFunc::ALWAYS:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	unsigned int getOpenGLTextureFormat(ETextureFormat inTextureFormat)
	{
		switch (inTextureFormat)
		{
		case ETextureFormat::RGB:
			return 0;
			break;
		case ETextureFormat::RGBA:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}
}

RenderBackend::RenderBackend()
{
	// needs to be located following the renderbackend instance
	impl = new Impl();
}

void RenderBackend::getContextRequirements(std::vector<std::pair<contextHints::Requirements, uint8_t>>& outHints)
{
	outHints.push_back({ contextHints::Requirements::ClientAPI, (uint8_t)contextHints::EClientAPI::NONE });
}

bool RenderBackend::initWindowGraphicLibrary(void* inProcAddress, size_t inWidth, size_t inHeight)
{

	return true;
}

void RenderBackend::initialize()
{
}

void RenderBackend::shutdown()
{ }

void RenderBackend::tracyGPUCollect()
{
}

void RenderBackend::clear()
{
	ZoneScopedN("Renderer::clear");
}

void RenderBackend::resizeViewport(int inX, int inY, int inWidth, int inHeight)
{
}

void RenderBackend::setStencilMask(unsigned int inMask)
{
}

void RenderBackend::setStencilFunc(EComparisonFunc inFunc, int inValue, unsigned int inMask)
{
}

GPUHandle RenderBackend::compileShader(std::string_view inVertexPath, std::string_view inFragmentPath)
{
	return {};
}

void RenderBackend::useShaderProgram(GPUHandle inHandle)
{
	ZoneScoped;
}

void RenderBackend::setBool(std::string_view inName, bool inValue)
{
}

void RenderBackend::setInt(std::string_view inName, int inValue)
{
}

void RenderBackend::setFloat(std::string_view inName, float inValue)
{
}

void RenderBackend::setVec3(std::string_view inName, const glm::vec3& inValue)
{
}

void RenderBackend::setVec4(std::string_view inName, float x, float y, float z, float w)
{
}

void RenderBackend::setMat4(std::string_view inName, glm::f32* inValue)
{
}

void RenderBackend::setTexture(std::string_view inName, GPUHandle inValue)
{
}

//
void RenderBackend::drawArrays(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inCount)
{
}

void RenderBackend::drawArraysInstanced(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inSize, size_t inCount)
{
}

void RenderBackend::drawElements(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, size_t inCount, void* inIndices)
{
}


//
GPUHandle RenderBackend::createTexture(uint8_t* inData, size_t inWidth, size_t inHeight, ETextureFormat inInternalFormat)
{
	return {};
}

void RenderBackend::bindTexture(GPUHandle inHandle, std::string_view inType, size_t inIndex)
{
}

void RenderBackend::deleteTexture(GPUHandle inHandle)
{
}

//
GPUHandle RenderBackend::createGPUBuffer(EBufferType inType)
{
	return {};
}

void RenderBackend::allocateGPUBuffer(EBufferType inType, size_t inCapacity, void* inData, EBufferUsage inUsage)
{
}

void RenderBackend::bindGPUBuffer(EBufferType inType, GPUHandle inHandle)
{
}

void RenderBackend::deleteGPUBuffer(EBufferType inType, GPUHandle inHandle)
{
}

void RenderBackend::setGPUBufferData(EBufferType inType, size_t inOffset, size_t inSize, void* inData)
{
}

void RenderBackend::setVertexLayoutData(unsigned int inLayoutIndex, size_t inSizeInBytes, size_t inVertexStride, void* inDataOffset, unsigned int inDivisor)
{
}


namespace render
{
	RenderBackend* g_renderBackEnd = nullptr;

	bool initializeBackend()
	{
		g_renderBackEnd = new RenderBackend();
		g_renderBackEnd->initialize();
		return true;
	}

	void shutdownBackend()
	{
		if (g_renderBackEnd)
		{
			g_renderBackEnd->shutdown();
			delete g_renderBackEnd;
		}
		g_renderBackEnd = nullptr;
	}

	RenderBackend* getRenderBackend()
	{
		return g_renderBackEnd;
	}
}