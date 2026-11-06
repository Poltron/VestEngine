#include "Core/Resources/MaterialInstance.h"
#include "Core/Engine.h"
#include "Core/Ensure.h"
#include "Core/ResourcesManager.h"
#include "Render/Color.h"
#include "Render/Renderer.h"
#include "Render/Resources/MaterialProxy.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/VulkanDevice.h"

namespace
{
	BufferHandle getParameterBufferHandle(GPUProxyHandle inProxyHandle, ResourceHandle inMaterialHandle, std::string_view inName)
	{
		MaterialProxy& proxy = render::getRenderer()->getMaterialProxy(inProxyHandle);
		BufferHandle handle = proxy.getMaterialBufferHandle();

		const Material* material = engine::getResources()->getMaterial(inMaterialHandle);
		handle.offset += material->getShaderParameters().getParameterOffset(inName);

		return handle;
	}
}

MaterialInstance::MaterialInstance(const Material* inMaterial, ResourceHandle inMaterialHandle)
	: materialHandle(inMaterialHandle), proxyHandle(GPUProxyHandle::invalid)
{
	materialHandle = inMaterialHandle;

	const ShaderParameters& shaderParameters = inMaterial->getShaderParameters();
	
	// todo: dynamic fields initialization
	MaterialData defaultMaterialData;
	defaultMaterialData.color = glm::vec4(color::white, 1);
	defaultMaterialData.diffuseTexIndex = 0;

	proxyHandle = render::getRenderer()->createMaterialProxy(&defaultMaterialData, sizeof(MaterialData));
}

MaterialInstance::~MaterialInstance()
{
	// todo
	render::getRenderer()->destroyMaterialProxy(proxyHandle);
}

MaterialInstance::MaterialInstance(MaterialInstance&& inOther) noexcept
	: materialHandle(inOther.materialHandle), proxyHandle(inOther.proxyHandle)
{
	inOther.materialHandle = ResourceHandle::invalid;
	inOther.proxyHandle = GPUProxyHandle::invalid;
}

MaterialInstance& MaterialInstance::operator=(MaterialInstance&& inOther) noexcept
{
	if (this == &inOther)
	{
		materialHandle = inOther.materialHandle;
		proxyHandle = inOther.proxyHandle;

		inOther.materialHandle = ResourceHandle::invalid;
		inOther.proxyHandle = GPUProxyHandle::invalid;
	}

	return *this;
}

uint32_t MaterialInstance::getUint(std::string_view inName) const
{
	MaterialProxy& proxy = render::getRenderer()->getMaterialProxy(proxyHandle);
	const Material* material = engine::getResources()->getMaterial(materialHandle);
	const uint8_t* materialBuffer = render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(proxy.getMaterialBufferHandle());
	return material->getShaderParameters().getUint(inName, materialBuffer);
}

glm::vec4 MaterialInstance::getVec4(std::string_view inName) const
{
	MaterialProxy& proxy = render::getRenderer()->getMaterialProxy(proxyHandle);
	const Material* material = engine::getResources()->getMaterial(materialHandle);
	const uint8_t* materialBuffer = render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(proxy.getMaterialBufferHandle());
	return material->getShaderParameters().getVec4(inName, materialBuffer);
}

void MaterialInstance::setVec3(std::string_view inName, const glm::vec3& inValue)
{
	BufferHandle parameterHandle = getParameterBufferHandle(proxyHandle, materialHandle, inName);
	glm::vec3* parameterMem = (glm::vec3*)render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(parameterHandle);
	*parameterMem = inValue;
}

void MaterialInstance::setVec4(std::string_view inName, const glm::vec4& inValue)
{
	BufferHandle parameterHandle = getParameterBufferHandle(proxyHandle, materialHandle, inName);
	glm::vec4* parameterMem = (glm::vec4*)render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(parameterHandle);
	*parameterMem = inValue;
}

void MaterialInstance::setMat4(std::string_view inName, const glm::mat4& inValue)
{
	BufferHandle parameterHandle = getParameterBufferHandle(proxyHandle, materialHandle, inName);
	glm::mat4* parameterMem = (glm::mat4*)render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(parameterHandle);
	*parameterMem = inValue;
}

void MaterialInstance::setTexture(std::string_view inName, ResourceHandle inValue)
{
	BufferHandle parameterHandle = getParameterBufferHandle(proxyHandle, materialHandle, inName);
	uint32_t* parameterMem = (uint32_t*)render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(parameterHandle);
	uint32_t textureIndex = render::getRenderBackend()->getDevice().addTexture(inValue);
	*parameterMem = textureIndex;
}