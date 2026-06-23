#include "Shader.h"

std::string ReadFile(const char* InFileName)
{
	std::string line, text;
	std::ifstream in(InFileName);
	while (std::getline(in, line))
	{
		text += line + "\n";
	}
	return text;
}

unsigned int CreateAndCompileShader(GLenum type, const char* shaderSource)
{
	unsigned int shaderID;
	shaderID = glCreateShader(type);

	glShaderSource(shaderID, 1, &shaderSource, NULL);
	glCompileShader(shaderID);

	int success;
	char infoLog[512];
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
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
	unsigned int vertexShader = CreateAndCompileShader(GL_VERTEX_SHADER, vertexShaderContent.c_str());

	std::string fragmentShaderContent = ReadFile(fragmentPath);
	unsigned int fragmentShader = CreateAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderContent.c_str());

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