#include "Systems/ShaderParameterCollection.h"

#include "Managers/ResourcesManager.h"
#include "Resources/Shader.h"

void ShaderParameterCollection::applyToShader(const Shader& inShader, const ResourcesManager& inResources)
{
	for (const auto& intParameter : intParameters)
	{
		inShader.setInt(intParameter.first, intParameter.second);
	}

	for (const auto& floatParameter : floatParameters)
	{
		inShader.setFloat(floatParameter.first, floatParameter.second);
	}

	for (const auto& vec3Parameter : vec3Parameters)
	{
		inShader.setVec3(vec3Parameter.first, vec3Parameter.second);
	}

	for (size_t i = 0; i < textureParameters.size(); ++i)
	{
		const auto& textureParameter = textureParameters[i];

		const Texture* texture = inResources.getTexture(ResourceHandle(textureParameter.second));
		assert(texture != nullptr);

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture->GetTextureID());

		inShader.setInt(textureParameter.first, (int)i);
	}
}

void ShaderParameterCollection::addTexture(const std::string& inName, GLuint inValue)
{
	textureParameters.push_back({ inName, inValue });
}

void ShaderParameterCollection::addInt(const std::string& inName, int inValue)
{
	intParameters.push_back({ inName, inValue });
}

void ShaderParameterCollection::addFloat(const std::string& inName, float inValue)
{
	floatParameters.push_back({ inName, inValue });
}

void ShaderParameterCollection::addVec3(const std::string& inName, float inX, float inY, float inZ)
{
	addVec3(inName, glm::vec3(inX, inY, inZ));
}

void ShaderParameterCollection::addVec3(const std::string& inName, const glm::vec3& inValue)
{
	vec3Parameters.push_back({ inName, inValue });
}