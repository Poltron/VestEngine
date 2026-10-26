#pragma once

#include "glm/vec3.hpp"
#include <string>
#include <vector>

#include "Core/ResourceHandle.h"

class MaterialInstance
{
	std::vector<std::pair<std::string, ResourceHandle>> textureParameters;
	std::vector<std::pair<std::string, int>> intParameters;
	std::vector<std::pair<std::string, float>> floatParameters;
	std::vector<std::pair<std::string, glm::vec3>> vec3Parameters;

public:
	void clear();
	void applyToShader();

	void addTexture(const std::string& inName, ResourceHandle inValue);
	void addInt(const std::string& inName, int inValue);
	void addFloat(const std::string& inName, float inValue);
	void addVec3(const std::string& inName, const glm::vec3& inValue);
};