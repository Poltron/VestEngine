#include "Resources/Shader.h"

#include <fstream>
#include <iostream>

#include "glm/detail/type_vec3.hpp"

#include "Systems/ShaderParameterCollection.h"

namespace ShaderPrivate
{
	std::string readFile(const char* inFileName)
	{
		std::string line, text;

		std::ifstream in(inFileName);
		if (in.fail())
		{
			std::cout << "ERROR: Could not read file " << inFileName << std::endl;
			return text;
		}

		while (std::getline(in, line))
		{
			text += line + "\n";
		}
		return text;
	}

	GLuint createAndCompileShader(GLenum type, const char* shaderSource)
	{
		GLuint shaderID;
		shaderID = glCreateShader(type);

		glShaderSource(shaderID, 1, &shaderSource, NULL);
		glCompileShader(shaderID);

		int success;
		char infoLog[512];
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
			std::cout << "ERROR: Shader " << type << " compilation failed\n" << infoLog << std::endl;
		}
		return shaderID;
	}
}

Shader::Shader(const char* inVertexPath, const char* inFragmentPath)
	: ID(0), vertexPath(inVertexPath), fragmentPath(inFragmentPath)
{
	load();
}

Shader::~Shader()
{
	glDeleteProgram(ID);
}

Shader::Shader(Shader&& inOther) noexcept
	: ID(inOther.ID), vertexPath(inOther.vertexPath), fragmentPath(inOther.fragmentPath)
{ 
	inOther.ID = 0;
	inOther.vertexPath.clear();
	inOther.fragmentPath.clear();
}

Shader& Shader::operator=(Shader&& inOther) noexcept
{
	if (this == &inOther)
	{
		ID = inOther.ID;
		vertexPath = inOther.vertexPath;
		fragmentPath = inOther.fragmentPath;

		inOther.ID = 0;
		inOther.vertexPath.clear();
		inOther.fragmentPath.clear();
	}
	return *this;
}

void Shader::load()
{
	std::string vertexShaderContent = ShaderPrivate::readFile(vertexPath.c_str());
	if (vertexShaderContent.size() == 0)
	{
		std::cout << "ERROR: VertexShader " << vertexPath << " is empty." << std::endl;
		return;
	}
	GLuint vertexShader = ShaderPrivate::createAndCompileShader(GL_VERTEX_SHADER, vertexShaderContent.c_str());

	std::string fragmentShaderContent = ShaderPrivate::readFile(fragmentPath.c_str());
	if (fragmentShaderContent.size() == 0)
	{
		std::cout << "ERROR: VertexShader " << fragmentPath << " is empty." << std::endl;
		return;
	}
	GLuint fragmentShader = ShaderPrivate::createAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderContent.c_str());

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
	int location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	int location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	int location = glGetUniformLocation(ID, name.c_str());
	glUniform1f(location, value);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	int location = glGetUniformLocation(ID, name.c_str());
	glUniform3f(location, x, y, z);
}

void Shader::setVec3(const std::string& name, const glm::vec3& inValue) const
{
	int location = glGetUniformLocation(ID, name.c_str());
	glUniform3f(location, inValue.x, inValue.y, inValue.z);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
	int location = glGetUniformLocation(ID, name.c_str());
	glUniform4f(location, x, y, z, w);
}

void Shader::setMat4(const std::string& name, glm::f32* value) const
{
	int location = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void Shader::setTexture(const std::string& name, GLuint value) const
{
	int location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, value);
}

void Shader::applyShaderParameterCollection(const ShaderParameterCollection& inParameters) const
{
}