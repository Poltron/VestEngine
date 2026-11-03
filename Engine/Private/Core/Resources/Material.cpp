#include "Core/Resources/Material.h"

#include <vector>

#include "Core/Engine.h"
#include "Core/Ensure.h"
#include "Core/ResourcesManager.h"
#include "Render/Renderer.h"
#include "Render/ShaderStructs.h"
#include "RenderBackend/RenderBackend.h"

// todo remove
#include "RenderBackend_Vulkan/VulkanDevice.h"

Material::Material(EMaterialBlendMode inBlendMode, EMaterialType inType, const ShaderParameters& inShaderParameters)
	: blendMode(inBlendMode), type(inType), defaultMaterialInstance(ResourceHandle::invalid), shaderParameters(inShaderParameters)
{
	// todo: this should be when recording a command using a specific material, check if the appropriate pipeline is up
	std::vector<ShaderStageInfo> shaderStageInfos;
	render::getRenderBackend()->getDevice().getShaderForMaterial(blendMode, type, shaderStageInfos);
	render::getRenderBackend()->getDevice().checkCompatiblePipeline(shaderStageInfos);	
}

void Material::createDefaultMaterialInstance(ResourceHandle inMaterialHandle)
{
	ensure(!defaultMaterialInstance.isValid());

	defaultMaterialInstance = engine::getResources()->createMaterialInstance(this, inMaterialHandle);
}