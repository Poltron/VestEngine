#pragma once

#include <glm/glm.hpp>
#include <string_view>

#include "Render/GraphicResourceHandle.h"

#define RENDER_OPENGL 1

class RenderBackend
{
public:
	//
	virtual void startTracyGPUZone(const std::string& inName) = 0;
	virtual void tracyGpuCollect() = 0;

	//
	virtual void clear() = 0;

	//
	virtual void drawArrays(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inCount) = 0;
	virtual void drawArraysInstanced(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inSize, size_t inCount) = 0;
	virtual void drawElements(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, size_t inCount, void* inIndices) = 0;

	//
	virtual void setStencilMask(unsigned int inMask) = 0;
	virtual void setStencilFunc(EComparisonFunc inFunc, int inValue, unsigned int inMask) = 0;

	//
	virtual GPUHandle compileShader(std::string_view inVertexPath, std::string_view inFragmentPath) = 0;
	virtual void useShaderProgram(GPUHandle inHandle) = 0;

	virtual void setBool(std::string_view name, bool value) = 0;
	virtual void setInt(std::string_view name, int value) = 0;
	virtual void setFloat(std::string_view name, float value) = 0;
	virtual void setVec3(std::string_view name, const glm::vec3& inValue) = 0;
	virtual void setVec4(std::string_view name, float x, float y, float z, float w) = 0;
	virtual void setMat4(std::string_view name, glm::f32* value) = 0;
	virtual void setTexture(std::string_view name, GPUHandle value) = 0;

	//
	virtual GPUHandle createTexture(uint8_t* inData) = 0;
	virtual void bindTexture(GPUHandle inHandle, std::string_view inType, size_t inIndex) = 0;
	virtual void deleteTexture(GPUHandle inHandle) = 0;

	//
	virtual GPUHandle createGPUBuffer(EBufferType inType) = 0;
	virtual void allocateGPUBuffer(EBufferType inType, size_t inSize, void* inData, EBufferUsage inUsage) = 0;
	virtual void bindGPUBuffer(EBufferType inType, GPUHandle inHandle) = 0;
	virtual void deleteGPUBuffer(EBufferType inType, GPUHandle inHandle) = 0;
	virtual void setGPUBufferData(EBufferType inType, size_t inOffset, size_t inSize, void* inData) = 0;
};

namespace render
{
	RenderBackend* getRenderBackend();

	bool initializeBackend();
	void shutdownBackend();
}