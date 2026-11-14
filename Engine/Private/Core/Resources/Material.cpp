#include "Core/Resources/Material.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

#include "Core/Engine.h"
#include "Core/Ensure.h"
#include "Core/FileSystem.h"
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

	ERenderPass getPassFromStr(std::string_view inString)
	{
		if (inString == "geometry")
		{
			return ERenderPass::GEOMETRY;
		}

		assert(false);
		return ERenderPass::GEOMETRY;
	}
}

Material::Material(const std::string&& inPath)
	: path(inPath), defaultMaterialInstance(ResourceHandle::invalid), shaderParameters(ShaderParameters())
{
	loadFromPath(File::getResourcesDir() + path);

	render::getRenderer()->loadShaders(vertex, fragment);
}

bool Material::loadFromPath(std::string_view inPath)
{
	using nlohmann::json;
	
	std::vector<char> content;
	if (!File::readFile(inPath.data(), content))
	{
		return false;
	}

	if (!json::accept(content))
	{
		std::cerr << "error: Material contains invalid json " << inPath << std::endl;
		return false;
	}

	json data = json::parse(content);
	name = data["name"].get<std::string>();
	type = getTypeFromStr(data["type"].get<std::string>());
	blendMode = getBlendModeFromStr(data["blendMode"].get<std::string>());
	pass = getPassFromStr(data["pass"].get<std::string>());
	vertex = data["vertex"].get<std::string>();
	fragment = data["fragment"].get<std::string>();

	return true;
}

void Material::createDefaultMaterialInstance(ResourceHandle inMaterialHandle)
{
	ensure(!defaultMaterialInstance.isValid());

	defaultMaterialInstance = engine::getResources()->createMaterialInstance(this, inMaterialHandle);
}