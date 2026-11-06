#include "Render/Resources/MaterialProxy.h"

#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/VulkanDevice.h"

VulkanMaterialProxy::VulkanMaterialProxy(void* inMaterialData, uint32_t inMaterialDataSize)
	: materialBufferHandle(BufferHandle::invalid)
{
	materialBufferHandle = render::getRenderBackend()->getDevice().pushMaterialData(inMaterialData, inMaterialDataSize);
	materialDescriptorSetHandle = render::getRenderBackend()->getDevice().getMaterialInstanceCompatibleDescriptorSet();
	textureDescriptorSetHandle = 0; // todo
}

VulkanMaterialProxy::~VulkanMaterialProxy()
{
	// todo : remove material data in buffer, ...
	// or let the destroy method do it and let proxy be a POD struct ?
	// dont know :)
}

VulkanMaterialProxy::VulkanMaterialProxy(VulkanMaterialProxy&& inOther) noexcept
	: materialBufferHandle(inOther.materialBufferHandle)
	, materialDescriptorSetHandle(inOther.materialDescriptorSetHandle)
	, textureDescriptorSetHandle(inOther.textureDescriptorSetHandle)
{
	inOther.materialBufferHandle = BufferHandle::invalid;
	inOther.materialDescriptorSetHandle = 0;
	inOther.textureDescriptorSetHandle = 0;
}

VulkanMaterialProxy& VulkanMaterialProxy::operator=(VulkanMaterialProxy&& inOther) noexcept
{
	if (this == &inOther)
	{
		materialBufferHandle = inOther.materialBufferHandle;
		materialDescriptorSetHandle = inOther.materialDescriptorSetHandle;
		textureDescriptorSetHandle = inOther.textureDescriptorSetHandle;

		inOther.materialBufferHandle = BufferHandle::invalid;
		inOther.materialDescriptorSetHandle = 0;
		inOther.textureDescriptorSetHandle = 0;
	}

	return *this;
}