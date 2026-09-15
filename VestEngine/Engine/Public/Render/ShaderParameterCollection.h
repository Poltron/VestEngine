#pragma once

#include <vector>
#include <string>

#include "glm/vec3.hpp"

#include "Render/GraphicResourceHandle.h"

class ResourcesManager;
class Shader;

struct ShaderParameterCollection
{
	void applyToShader(const Shader& inShader, const ResourcesManager& inResources);

	void addTexture(const std::string& inName, GraphicResourceHandle inValue);
	void addInt(const std::string& inName, int inValue);
	void addFloat(const std::string& inName, float inValue);
	void addVec3(const std::string& inName, const glm::vec3& inValue);

private:
	std::vector<std::pair<std::string, GraphicResourceHandle>> textureParameters;
	std::vector<std::pair<std::string, int>> intParameters;
	std::vector<std::pair<std::string, float>> floatParameters;
	std::vector<std::pair<std::string, glm::vec3>> vec3Parameters;
};