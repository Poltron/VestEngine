#include "Render/Resources/TextureProxy.h"

#include "Core/Resources/Texture.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/VulkanDevice.h"

TextureProxy::TextureProxy(const Texture* inTexture)
	: handle(GPUAllocHandle::invalid)
{
	handle = render::getRenderBackend()->getDevice().createTexture(inTexture->getData(), inTexture->getWidth(), inTexture->getHeight(), inTexture->getChannelCount());
}

TextureProxy::~TextureProxy()
{
	if (handle.isValid())
	{
		render::getRenderBackend()->getDevice().destroyImage(handle);
	}
}

TextureProxy::TextureProxy(TextureProxy&& inOther) noexcept
	: handle(inOther.handle)
{
	inOther.handle = GPUAllocHandle::invalid;
}

TextureProxy& TextureProxy::operator=(TextureProxy&& inOther) noexcept
{
	if (this != &inOther)
	{
		handle = inOther.getHandle();

		inOther.handle = GPUAllocHandle::invalid;
	}
	return *this;
}