#pragma once

#include <string>

#include "glad/glad.h"
#include "glm/fwd.hpp"

struct ShaderParameterCollection;

class Shader
{
public:
	Shader() = delete;
	Shader(const char* vertexPath, const char* fragmentPath);
	~Shader();

	Shader(const Shader& inOther) = delete;
	Shader& operator=(const Shader& inOther) = delete;

	Shader(Shader&& inOther) noexcept;
	Shader& operator=(Shader&& inOther) noexcept;

	void use();
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, float x, float y, float z) const;
	void setVec3(const std::string& name, const glm::vec3& inValue) const;
	void setVec4(const std::string& name, float x, float y, float z, float w) const;
	void setMat4(const std::string& name, glm::f32* value) const;
	void setTexture(const std::string& name, GLuint value) const;

	void applyShaderParameterCollection(const ShaderParameterCollection& inParameters) const;

private:
	void load();

	GLuint ID;
	std::string vertexPath;
	std::string fragmentPath;
};

