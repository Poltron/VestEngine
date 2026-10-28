#pragma once

#include <glm/glm.hpp>
#include <string_view>

#include "Platform/WindowHints.h"
#include "Render/GPUHandle.h"
#include "Render/Resources/Mesh.h"
#include "RenderBackend/RenderBackendDef.h"

class RenderDevice;
class RenderPipeline;
class Scene;

class RenderBackend
{
	struct Impl;
	Impl* impl;

	uint32_t frameNumber = 0;

public:
	RenderBackend();

	bool initialize();
	void shutdown();
	std::string getName();
	RenderDevice& getDevice();
	RenderPipeline& getGeometryPipeline();

	//
	static void getContextRequirements(std::vector<std::pair<contextHints::Requirements, uint8_t>>& outHints);

	//
	void tracyGPUCollect();

	void drawGeometry(Scene& inScene);
	void resizeViewport(int inX, int inY, int inWidth, int inHeight);
	void clear();

	//
	void drawArrays(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inCount);
	void drawArraysInstanced(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inSize, size_t inCount);
	void drawElements(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, size_t inCount, void* inIndices);
	void drawMesh(const Mesh& inMesh);

	//
	void setStencilMask(unsigned int inMask);
	void setStencilFunc(EComparisonFunc inFunc, int inValue, unsigned int inMask);

	//
	GPUHandle compileShader(std::string_view inVertexPath, std::string_view inFragmentPath);
	void useShaderProgram(GPUHandle inHandle);

	void setBool(std::string_view name, bool value);
	void setInt(std::string_view name, int value);
	void setFloat(std::string_view name, float value);
	void setVec3(std::string_view name, const glm::vec3& inValue);
	void setVec4(std::string_view name, float x, float y, float z, float w);
	void setMat4(std::string_view name, glm::f32* value);
	void setTexture(std::string_view name, GPUHandle value);

	//
	GPUHandle createTexture(uint8_t* inData, size_t inWidth, size_t inHeight);
	void bindTexture(GPUHandle inHandle, std::string_view inType, size_t inIndex);
	void deleteTexture(GPUHandle inHandle);

	//
	GPUHandle createGPUBuffer(EBufferType inType);
	void allocateGPUBuffer(EBufferType inType, size_t inSize, void* inData, EBufferUsage inUsage);
	void bindGPUBuffer(EBufferType inType, GPUHandle inHandle);
	void deleteGPUBuffer(EBufferType inType, GPUHandle inHandle);
	void setGPUBufferData(EBufferType inType, size_t inOffset, size_t inSize, void* inData);
	void setVertexLayoutData(unsigned int inLayoutIndex, size_t inSizeInBytes, size_t inVertexStride, void* inDataOffset, unsigned int inDivisor);
};

namespace render
{
	RenderBackend* getRenderBackend();

	bool initializeBackend();
	void shutdownBackend();
}