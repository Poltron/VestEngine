#pragma once

#include <glm/glm.hpp>
#include <string_view>

#include "Platform/WindowHints.h"
#include "Render/GPUHandle.h"
#include "Render/Resources/MeshProxy.h"
#include "Render/Resources/TextureProxy.h"
#include "RenderBackend/RenderBackendDef.h"
#include "RenderBackend/RHICommandRecorder.h"

class Mesh;
class Scene;
class VulkanBuffer;
class VulkanDevice;
class VulkanPipeline;

struct PipelineState;

class RenderBackend
{
	struct Impl;
	Impl* impl;

	PipelineState* state;

	uint32_t frameNumber = 0;

public:
	RenderBackend();

	bool initialize();
	void shutdown();
	std::string getName();
	VulkanDevice& getDevice();

	//
	static void getContextRequirements(std::vector<std::pair<contextHints::Requirements, uint8_t>>& outHints);

	//
	void tracyGPUCollect();

	void render(Scene& inScene);
	void resizeViewport(int inX, int inY, int inWidth, int inHeight);
	void clear();

	bool executeCommands(VkCommandBuffer inCommandBuffer
		, uint8_t* inCommands
		, uint32_t inCommandsSize
		, uint32_t inImageId
		, uint32_t inFrameId);

private:
	void executeStartRenderpass(VkCommandBuffer inCommandBuffer, StartRenderpassCommand* inStartRenderpassCommand, uint8_t inImageIndex);
	void executeEndRenderpass(VkCommandBuffer inCommandBuffer, EndRenderpassCommand* inEndRenderpassCommand);
	void executeBindMaterial(VkCommandBuffer inCommandBuffer, BindMaterialCommand* inBindMaterialCommand, BufferHandle inFrameBufferHandle, VkDescriptorSet inFrameBufferDescriptorSet);
	void executeDrawIndexed(VkCommandBuffer inCommandBuffer, DrawIndexCommand* inDrawIndexCommand, VulkanBuffer& inFrameBuffer);
};

namespace render
{
	RenderBackend* getRenderBackend();
	RHICommandRecorder* getCommandRecorder();

	bool initializeBackend();
	void shutdownBackend();
}