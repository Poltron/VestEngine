#pragma once

#include <vector>
#include <string>

#include "glad/glad.h"
#include "glm/vec3.hpp"

class ResourcesManager;
class Shader;

struct ShaderParameterCollection
{
	void applyToShader(const Shader& inShader, const ResourcesManager& inResources);

	void addTexture(const std::string& inName, GLuint inValue);
	void addInt(const std::string& inName, int inValue);
	void addFloat(const std::string& inName, float inValue);
	void addVec3(const std::string& inName, float inX, float inY, float inZ);
	void addVec3(const std::string& inName, const glm::vec3& inValue);

private:
	std::vector<std::pair<std::string, GLuint>> textureParameters;
	std::vector<std::pair<std::string, int>> intParameters;
	std::vector<std::pair<std::string, float>> floatParameters;
	std::vector<std::pair<std::string, glm::vec3>> vec3Parameters;
};