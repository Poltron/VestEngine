#include "Core/Resources/Material.h"

#include <vector>

#include "Core/Engine.h"
#include "Core/Ensure.h"
#include "Core/ResourcesManager.h"
#include "Render/Renderer.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/RenderDevice.h"
#include "RenderBackend_Vulkan/ShaderStructs.h"

Material::Material(EMaterialBlendMode inBlendMode, EMaterialType inType, const ShaderParameters& inShaderParameters)
	: blendMode(inBlendMode), type(inType), defaultMaterialInstance(ResourceHandle::invalid), shaderParameters(inShaderParameters)
{
	std::vector<ShaderStageInfo> shaderStageInfos;
	render::getRenderBackend()->getDevice().getShaderForMaterial(blendMode, type, shaderStageInfos);
	render::getRenderBackend()->getDevice().checkCompatiblePipeline(shaderStageInfos);
}

void Material::createDefaultMaterialInstance(ResourceHandle inMaterialHandle)
{
	ensure(!defaultMaterialInstance.isValid());

	defaultMaterialInstance = engine::getResources()->createMaterialInstance(this, inMaterialHandle);
}