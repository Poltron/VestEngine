#include "Core/Resources/MaterialInstance.h"
#include "Core/Engine.h"
#include "Core/Ensure.h"
#include "Core/ResourcesManager.h"
#include "Render/Color.h"
#include "Render/Renderer.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/RenderDevice.h"

MaterialInstance::MaterialInstance(const Material* inMaterial, ResourceHandle inMaterialHandle)
	: materialHandle(inMaterialHandle), materialBufferHandle(0, 0)
{
	materialHandle = inMaterialHandle;

	const ShaderParameters& shaderParameters = inMaterial->getShaderParameters();
	
	// todo: dynamic fields initialization
	MaterialData defaultMaterialData;
	defaultMaterialData.color = glm::vec4(color::white, 1);
	defaultMaterialData.diffuseTexIndex = 0;

	materialBufferHandle = render::getRenderBackend()->getDevice().pushMaterialData(&defaultMaterialData, shaderParameters.size);
	descriptorSetHandle = render::getRenderBackend()->getDevice().getMaterialInstanceCompatibleDescriptorSet();
}

MaterialInstance::~MaterialInstance()
{
	// todo
	//render::getRenderBackend()->getDevice().destroyMaterialData(parametersBufferHandle);
}

MaterialInstance::MaterialInstance(MaterialInstance&& inOther) noexcept
	: descriptorSetHandle(inOther.descriptorSetHandle), materialBufferHandle(inOther.materialBufferHandle), materialHandle(inOther.materialHandle)
{
	inOther.descriptorSetHandle = 0;
	inOther.materialBufferHandle = BufferHandle::invalid;
	inOther.materialHandle = ResourceHandle::invalid;
}

MaterialInstance& MaterialInstance::operator=(MaterialInstance&& inOther) noexcept
{
	if (this == &inOther)
	{
		descriptorSetHandle = inOther.descriptorSetHandle;
		materialBufferHandle = inOther.materialBufferHandle;
		materialHandle = inOther.materialHandle;

		inOther.descriptorSetHandle = 0;
		inOther.materialBufferHandle = BufferHandle::invalid;
		inOther.materialHandle = ResourceHandle::invalid;
	}

	return *this;
}

uint32_t MaterialInstance::getUint(std::string_view inName) const
{
	const Material* material = engine::getResources()->getMaterial(materialHandle);
	const uint8_t* materialBuffer = render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(materialBufferHandle.offset);
	return material->getShaderParameters().getUint(inName, materialBuffer);
}

glm::vec4 MaterialInstance::getVec4(std::string_view inName) const
{
	const Material* material = engine::getResources()->getMaterial(materialHandle);
	const uint8_t* materialBuffer = render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(materialBufferHandle.offset);
	return material->getShaderParameters().getVec4(inName, materialBuffer);
}

void MaterialInstance::setVec3(std::string_view inName, const glm::vec3& inValue)
{
	const Material* material = engine::getResources()->getMaterial(materialHandle);
	uint32_t offset = material->getShaderParameters().getParameterOffset(inName);
	glm::vec3* parameterMem = (glm::vec3*)render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(materialBufferHandle.offset + offset);
	*parameterMem = inValue;
}

void MaterialInstance::setVec4(std::string_view inName, const glm::vec4& inValue)
{
	const Material* material = engine::getResources()->getMaterial(materialHandle);
	uint32_t paramOffset = material->getShaderParameters().getParameterOffset(inName);
	glm::vec4* parameterMem = (glm::vec4*)render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(materialBufferHandle.offset + paramOffset);
	*parameterMem = inValue;
}

void MaterialInstance::setMat4(std::string_view inName, const glm::mat4& inValue)
{
	const Material* material = engine::getResources()->getMaterial(materialHandle);
	uint32_t offset = material->getShaderParameters().getParameterOffset(inName);
	glm::mat4* parameterMem = (glm::mat4*)render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(materialBufferHandle.offset + offset);
	*parameterMem = inValue;
}

void MaterialInstance::setTexture(std::string_view inName, ResourceHandle inValue)
{
	uint32_t textureIndex = render::getRenderBackend()->getDevice().addTexture(inValue);

	const Material* material = engine::getResources()->getMaterial(materialHandle);
	uint32_t offset = material->getShaderParameters().getParameterOffset(inName);
	uint32_t* parameterMem = (uint32_t*)render::getRenderBackend()->getDevice().getMaterialInstanceBuffer(materialBufferHandle.offset + offset);
	*parameterMem = textureIndex;
}