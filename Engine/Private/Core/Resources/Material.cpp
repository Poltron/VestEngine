#include "Core/Resources/Material.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

#include "Core/Engine.h"
#include "Core/Ensure.h"
#include "Core/ResourcesManager.h"
#include "Render/Renderer.h"
#include "Render/ShaderStructs.h"
#include "RenderBackend/RenderBackend.h"

namespace
{
	EMaterialType getTypeFromStr(std::string_view inString)
	{
		if (inString == "lit")
		{
			return EMaterialType::LIT;
		}
		else if (inString == "unlit")
		{
			return EMaterialType::UNLIT;
		}

		assert(false);
		return EMaterialType::UNLIT;
	}

	EMaterialBlendMode getBlendModeFromStr(std::string_view inString)
	{
		if (inString == "opaque")
		{
			return EMaterialBlendMode::OPAQUE;
		}

		assert(false);
		return EMaterialBlendMode::OPAQUE;
	}
}

Material::Material(const std::string&& inPath)
	: path(inPath), blendMode(EMaterialBlendMode::OPAQUE), type(EMaterialType::UNLIT), vertex(), fragment(), defaultMaterialInstance(ResourceHandle::invalid), shaderParameters(ShaderParameters())
{
	loadFromPath(path);

}

Material::Material(EMaterialBlendMode inBlendMode, EMaterialType inType, const ShaderParameters& inShaderParameters)
	: blendMode(inBlendMode), type(inType), defaultMaterialInstance(ResourceHandle::invalid), shaderParameters(inShaderParameters)
{
	// todo: this should be when recording a command using a specific material, check if the appropriate pipeline is up
	//std::vector<ShaderStageInfo> shaderStageInfos;
	//render::getRenderBackend()->getDevice().getShaderForMaterial(blendMode, type, shaderStageInfos);
	//render::getRenderBackend()->getDevice().get(shaderStageInfos);	
}

bool Material::loadFromPath(std::string_view inPath)
{
	using nlohmann::json;

	std::ifstream f(path);
	json data = json::parse(f);
	if (!json::accept(f))
	{
		std::cerr << "error: Material load failed, invalid json." << path << std::endl;
		return false;
	}

	name = data["name"].get<std::string>();
	type = getTypeFromStr(data["type"].get<std::string>());
	blendMode = getBlendModeFromStr(data["blendMode"].get<std::string>());
	vertex = data["vertex"].get<std::string>();
	fragment = data["fragment"].get<std::string>();
}

{
	ensure(!defaultMaterialInstance.isValid());

	defaultMaterialInstance = engine::getResources()->createMaterialInstance(this, inMaterialHandle);
}