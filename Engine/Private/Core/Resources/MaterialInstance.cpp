#include "Core/Resources/MaterialInstance.h"

#include "Core/Engine.h"
#include "Core/Ensure.h"
#include "Core/ResourcesManager.h"
#include "Render/Renderer.h"
#include "RenderBackend/RenderBackend.h"

void MaterialInstance::clear()
{
	intParameters.clear();
	floatParameters.clear();
	vec3Parameters.clear();
	textureParameters.clear();
}

void MaterialInstance::applyToShader()
{
	for (const auto& intParameter : intParameters)
	{
		render::getRenderBackend()->setInt(intParameter.first, intParameter.second);
	}

	for (const auto& floatParameter : floatParameters)
	{
		render::getRenderBackend()->setFloat(floatParameter.first, floatParameter.second);
	}

	for (const auto& vec3Parameter : vec3Parameters)
	{
		render::getRenderBackend()->setVec3(vec3Parameter.first, vec3Parameter.second);
	}

	for (size_t i = 0; i < textureParameters.size(); ++i)
	{
		const auto& textureParameter = textureParameters[i];

		Texture* texture = engine::getResources()->getTexture(textureParameter.second);
		ensure(texture);

		render::getRenderBackend()->bindTexture(texture->getHandle(), texture->getType(), i);
	}
}

void MaterialInstance::addTexture(const std::string& inName, ResourceHandle inValue)
{
	for (auto it = textureParameters.begin(); it != textureParameters.end(); ++it)
	{
		if (it->first == inName)
		{
			it->second = inValue;
			return;
		}
	}

	textureParameters.push_back({ inName, inValue });
}

void MaterialInstance::addInt(const std::string& inName, int inValue)
{
	for (auto it = intParameters.begin(); it != intParameters.end(); ++it)
	{
		if (it->first == inName)
		{
			it->second = inValue;
			return;
		}
	}

	intParameters.push_back({ inName, inValue });
}

void MaterialInstance::addFloat(const std::string& inName, float inValue)
{
	for (auto it = floatParameters.begin(); it != floatParameters.end(); ++it)
	{
		if (it->first == inName)
		{
			it->second = inValue;
			return;
		}
	}

	floatParameters.push_back({ inName, inValue });
}

void MaterialInstance::addVec3(const std::string& inName, const glm::vec3& inValue)
{
	for (auto it = vec3Parameters.begin(); it != vec3Parameters.end(); ++it)
	{
		if (it->first == inName)
		{
			it->second = inValue;
			return;
		}
	}

	vec3Parameters.push_back({ inName, inValue });
}