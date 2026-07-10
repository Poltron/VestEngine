#include "Resources/Shader.h"

#include <fstream>
#include <iostream>

#include "glm/detail/type_vec3.hpp"

std::string ReadFile(const char* inFileName)
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

GLuint CreateAndCompileShader(GLenum type, const char* shaderSource)
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

Shader::Shader()
	: ID(0)
{
}

void Shader::load(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexShaderContent = ReadFile(vertexPath);
	if (vertexShaderContent.size() == 0)
	{
		std::cout << "ERROR: VertexShader " << vertexPath << " is empty." << std::endl;
		return;
	}
	GLuint vertexShader = CreateAndCompileShader(GL_VERTEX_SHADER, vertexShaderContent.c_str());

	std::string fragmentShaderContent = ReadFile(fragmentPath);
	if (fragmentShaderContent.size() == 0)
	{
		std::cout << "ERROR: VertexShader " << fragmentPath << " is empty." << std::endl;
		return;
	}
	GLuint fragmentShader = CreateAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderContent.c_str());

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