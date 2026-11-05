#include "Render/Resources/TextureProxy.h"

#include "Core/Resources/Texture.h"
#include "RenderBackend/RenderBackend.h"

TextureProxy::TextureProxy(const Texture* inTexture)
	: handle(GPUAllocHandle::invalid)
{
	handle = render::getRenderBackend()->createTexture(inTexture->getData(), inTexture->getWidth(), inTexture->getHeight());
}

TextureProxy::~TextureProxy()
{
	if (handle.isValid())
	{
		render::getRenderBackend()->deleteTexture(handle);
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