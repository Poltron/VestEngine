#pragma once

#include "Render/GPUHandle.h"

class VulkanMaterialProxy
{
	BufferHandle materialBufferHandle;
	uint32_t materialDescriptorSetHandle;
	uint32_t textureDescriptorSetHandle;

public:
	BufferHandle getMaterialBufferHandle() const { return materialBufferHandle; }
	uint32_t getMaterialDescriptorSetHandle() const { return materialDescriptorSetHandle; }
	uint32_t getTextureDescriptorSetHandle() const { return textureDescriptorSetHandle; }

	VulkanMaterialProxy() = delete;
	VulkanMaterialProxy(void* inMaterialData, uint32_t inMaterialDataSize);
	~VulkanMaterialProxy();

	VulkanMaterialProxy(const VulkanMaterialProxy& inOther) = delete;
	VulkanMaterialProxy& operator=(const VulkanMaterialProxy& inOther) = delete;

	VulkanMaterialProxy(VulkanMaterialProxy&& inOther) noexcept;
	VulkanMaterialProxy& operator=(VulkanMaterialProxy&& inOther) noexcept;
};