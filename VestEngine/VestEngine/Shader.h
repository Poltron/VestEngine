#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "glm/fwd.hpp"

class Shader
{
public:
	unsigned int ID;

	Shader();

	void load(const char* vertexPath, const char* fragmentPath);
	void use();
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec4(const std::string& name, float x, float y, float z, float w) const;
	void setMat4(const std::string& name, glm::f32* value) const;
};

