#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include "glad/glad.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include "Core/Ensure.h"
#include "Platform/WindowHints.h"
#include "Platform/WindowManager.h"
#include "RenderBackend/RenderBackend.h"
#include "Render/GraphicResourceHandle.h"

struct RenderBackend::Impl
{
	GLuint currentShaderProgram = 0;
};

namespace
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
			return 0;
		}
		return shaderID;
	}

	bool isUniformLocationValid(int inLocation, std::string_view inName)
	{
		if (inLocation == -1)
		{
			// todo: muting cause i get flooded because i have some entities with unlit shader and globalparameters contain light values
			//std::cerr << "GetUniformLocation failed " << inName << std::endl;
			return false;
		}

		return true;
	}

	GLenum getOpenGLPrimitiveMode(EPrimitiveMode inMode)
	{
		switch (inMode)
		{
		case EPrimitiveMode::POINTS:
			return GL_POINTS;
			break;
		case EPrimitiveMode::LINES:
			return GL_LINES;
			break;
		case EPrimitiveMode::TRIANGLES:
			return GL_TRIANGLES;
			break;
		}
		ensure(false);
		return GL_INVALID_ENUM;
	}

	GLenum getOpenGLBufferType(EBufferType inType)
	{
		switch (inType)
		{
		case EBufferType::VERTEX_ARRAY:
			ensure(false);
			return GL_INVALID_ENUM; // should not be needed
			break;
		case EBufferType::VERTICES:
			return GL_ARRAY_BUFFER;
			break;
		case EBufferType::INDICES:
			return GL_ELEMENT_ARRAY_BUFFER;
			break;
		}
		ensure(false);
		return GL_INVALID_ENUM;
	}

	GLenum getOpenGLBufferUsage(EBufferUsage inUsage)
	{
		switch (inUsage)
		{
		case EBufferUsage::STATIC:
			return GL_STATIC_DRAW;
			break;
		case EBufferUsage::STREAM:
			return GL_STREAM_DRAW;
			break;
		case EBufferUsage::DYNAMIC:
			return GL_DYNAMIC_DRAW;
			break;
		}
		ensure(false);
		return GL_INVALID_ENUM;
	}

	GLenum getOpenGLComparisonFunc(EComparisonFunc inFunc)
	{
		switch (inFunc)
		{
		case EComparisonFunc::NEVER:
			return GL_NEVER;
			break;
		case EComparisonFunc::LESS:
			return GL_LESS;
			break;
		case EComparisonFunc::LEQUAL:
			return GL_LEQUAL;
			break;
		case EComparisonFunc::EQUAL:
			return GL_EQUAL;
			break;
		case EComparisonFunc::NOTEQUAL:
			return GL_NOTEQUAL;
			break;
		case EComparisonFunc::GEQUAL:
			return GL_GEQUAL;
			break;
		case EComparisonFunc::GREATER:
			return GL_GREATER;
			break;
		case EComparisonFunc::ALWAYS:
			return GL_ALWAYS;
			break;
		}
		ensure(false);
		return GL_INVALID_ENUM;
	}

	GLenum getOpenGLTextureFormat(ETextureFormat inTextureFormat)
	{
		switch (inTextureFormat)
		{
		case ETextureFormat::RGB:
			return GL_RGB;
			break;
		case ETextureFormat::RGBA:
			return GL_RGBA;
			break;
		}
		ensure(false);
		return GL_INVALID_ENUM;
	}
}

RenderBackend::RenderBackend()
{
	// needs to be located following the renderbackend instance
	impl = new Impl();
}

void RenderBackend::getContextRequirements(std::vector<std::pair<contextHints::Requirements, uint8_t>>& outHints)
{
	outHints.push_back({ contextHints::Requirements::ClientAPI, (uint8_t)contextHints::EClientAPI::OPENGL });
	outHints.push_back({ contextHints::Requirements::ContextCreationAPI, (uint8_t)contextHints::EContextCreationAPI::NATIVE });
	outHints.push_back({ contextHints::Requirements::MajorVersion, 3 });
	outHints.push_back({ contextHints::Requirements::MinorVersion, 3 });
}

bool RenderBackend::initWindowGraphicLibrary(void* inProcAddress, size_t inWidth, size_t inHeight)
{
	if (!gladLoadGLLoader((GLADloadproc)inProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	TracyGpuContext;
	glViewport(0, 0, (GLsizei)inWidth, (GLsizei)inHeight);

	return true;
}

void RenderBackend::initialize()
{
	glEnable(GL_PROGRAM_POINT_SIZE);

	// depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// stencil test
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0x00);
}

void RenderBackend::shutdown()
{ }

void RenderBackend::tracyGPUCollect()
{
	TracyGpuCollect;
}

void RenderBackend::clear()
{
	ZoneScopedN("Renderer::clear");
	TracyGpuZone("Renderer::clear");

	glStencilMask(0xFF);
	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);
}

void RenderBackend::resizeViewport(int inX, int inY, int inWidth, int inHeight)
{
	glViewport(inX, inY, inWidth, inHeight);
}

void RenderBackend::setStencilMask(unsigned int inMask)
{
	glStencilMask((GLuint)inMask);
}

void RenderBackend::setStencilFunc(EComparisonFunc inFunc, int inValue, unsigned int inMask)
{
	GLenum func = getOpenGLComparisonFunc(inFunc);
	glStencilFunc(func, (GLint)inValue, (GLuint)inMask);
}

GPUHandle RenderBackend::compileShader(std::string_view inVertexPath, std::string_view inFragmentPath)
{
	std::string content = readFile(inVertexPath.data());
	if (content.size() == 0)
	{
		std::cout << "ERROR: Shader " << inVertexPath << " is empty." << std::endl;
		return {};
	}

	GLuint vertexShader = createAndCompileShader(GL_VERTEX_SHADER, content.data());
	if (vertexShader == 0)
	{
		return {};
	}

	content = readFile(inFragmentPath.data());
	if (content.size() == 0)
	{
		std::cout << "ERROR: FragmentShader " << inFragmentPath << " is empty." << std::endl;
		return {};
	}
	GLuint fragmentShader = createAndCompileShader(GL_FRAGMENT_SHADER, content.c_str());
	if (fragmentShader == 0)
	{
		return {};
	}

	GLuint ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	GLint success;
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "Shader Linking Failed: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return GPUHandle(ID);
}

void RenderBackend::useShaderProgram(GPUHandle inHandle)
{
	ZoneScoped;

	if (impl->currentShaderProgram == inHandle.id)
	{
		return;
	}

	glUseProgram(inHandle.id);
	impl->currentShaderProgram = inHandle.id;

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "glUseProgram \"" << inHandle.id << "\" failed: " << err << std::endl;
		}
	}
}

void RenderBackend::setBool(std::string_view inName, bool inValue)
{
	int location = glGetUniformLocation(impl->currentShaderProgram, inName.data());
	if (!isUniformLocationValid(location, inName))
	{
		return;
	}

	glUniform1i(location, (int)inValue);
}

void RenderBackend::setInt(std::string_view inName, int inValue)
{
	int location = glGetUniformLocation(impl->currentShaderProgram, inName.data());
	if (!isUniformLocationValid(location, inName))
	{
		return;
	}

	glUniform1i(location, inValue);
}

void RenderBackend::setFloat(std::string_view inName, float inValue)
{
	int location = glGetUniformLocation(impl->currentShaderProgram, inName.data());
	if (!isUniformLocationValid(location, inName))
	{
		return;
	}

	glUniform1f(location, inValue);
}

void RenderBackend::setVec3(std::string_view inName, const glm::vec3& inValue)
{
	int location = glGetUniformLocation(impl->currentShaderProgram, inName.data());
	if (!isUniformLocationValid(location, inName))
	{
		return;
	}

	glUniform3f(location, inValue.x, inValue.y, inValue.z);
}

void RenderBackend::setVec4(std::string_view inName, float x, float y, float z, float w)
{
	int location = glGetUniformLocation(impl->currentShaderProgram, inName.data());
	if (!isUniformLocationValid(location, inName))
	{
		return;
	}

	glUniform4f(location, x, y, z, w);
}

void RenderBackend::setMat4(std::string_view inName, glm::f32* inValue)
{
	int location = glGetUniformLocation(impl->currentShaderProgram, inName.data());
	if (!isUniformLocationValid(location, inName))
	{
		return;
	}

	glUniformMatrix4fv(location, 1, GL_FALSE, inValue);
}

void RenderBackend::setTexture(std::string_view inName, GPUHandle inValue)
{
	int location = glGetUniformLocation(impl->currentShaderProgram, inName.data());
	if (!isUniformLocationValid(location, inName.data()))
	{
		return;
	}

	glUniform1i(location, inValue.id);
}

//
void RenderBackend::drawArrays(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inCount)
{
	bindGPUBuffer(EBufferType::VERTEX_ARRAY, inVAO);

	GLenum primitiveMode = getOpenGLPrimitiveMode(inPrimitiveMode);
	glDrawArrays(primitiveMode, inOffset, (GLsizei)inCount);

	bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "glDrawArrays failed: " << err << std::endl;
		}
	}
}

void RenderBackend::drawArraysInstanced(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inSize, size_t inCount)
{
	bindGPUBuffer(EBufferType::VERTEX_ARRAY, inVAO);

	GLenum primitiveMode = getOpenGLPrimitiveMode(inPrimitiveMode);
	glDrawArraysInstanced(primitiveMode, inOffset, (GLsizei)inSize, (GLsizei)inCount);

	bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "glDrawArraysInstanced failed: " << err << std::endl;
		}
	}
}

void RenderBackend::drawElements(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, size_t inCount, void* inIndices)
{
	bindGPUBuffer(EBufferType::VERTEX_ARRAY, inVAO);

	GLenum primitiveMode = getOpenGLPrimitiveMode(inPrimitiveMode);
	glDrawElements(primitiveMode, (GLsizei)inCount, GL_UNSIGNED_INT, inIndices);

	bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "glDrawElements failed: " << err << std::endl;
		}
	}
}


//
GPUHandle RenderBackend::createTexture(uint8_t* inData, size_t inWidth, size_t inHeight, ETextureFormat inInternalFormat)
{
	GLenum glInternalFormat = getOpenGLTextureFormat(inInternalFormat);

	GPUHandle handle;
	glGenTextures(1, &handle.id);
	glBindTexture(GL_TEXTURE_2D, handle.id);
	glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, (GLsizei)inWidth, (GLsizei)inHeight, 0, glInternalFormat, GL_UNSIGNED_BYTE, inData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "createTexture " << handle.id << " failed: " << err << std::endl;
		}
	}

	return handle;
}

void RenderBackend::bindTexture(GPUHandle inHandle, std::string_view inType, size_t inIndex)
{
	glActiveTexture(GL_TEXTURE0 + (GLenum)inIndex);
	glBindTexture(GL_TEXTURE_2D, inHandle.id);

	setInt(std::string("material.").append(inType), (int)inIndex);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "bindTexture " << inHandle.id << " failed: " << err << std::endl;
		}
	}
}

void RenderBackend::deleteTexture(GPUHandle inHandle)
{
	glDeleteTextures(1, &inHandle.id);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "deleteTexture " << inHandle.id << " failed: " << err << std::endl;
		}
	}
}

//
GPUHandle RenderBackend::createGPUBuffer(EBufferType inType)
{
	GPUHandle handle = {};
	switch (inType)
	{
	case EBufferType::VERTEX_ARRAY:
		glGenVertexArrays(1, &handle.id);
		break;
	case EBufferType::VERTICES:
	case EBufferType::INDICES:
		glGenBuffers(1, &handle.id);
		break;
	}

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "createGPUBuffer " << handle.id << " failed: " << err << std::endl;
		}
	}

	return handle;
}

void RenderBackend::allocateGPUBuffer(EBufferType inType, size_t inCapacity, void* inData, EBufferUsage inUsage)
{
	GLenum type = getOpenGLBufferType(inType);
	GLenum usage = getOpenGLBufferUsage(inUsage);
	glBufferData(type, inCapacity, inData, usage);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "allocateGPUBuffer " << inCapacity << "bytes failed: " << err << std::endl;
		}
	}
}

void RenderBackend::bindGPUBuffer(EBufferType inType, GPUHandle inHandle)
{
	switch (inType)
	{
	case EBufferType::VERTEX_ARRAY:
		glBindVertexArray(inHandle.id);
		break;
	case EBufferType::VERTICES:
		glBindBuffer(GL_ARRAY_BUFFER, inHandle.id);
		break;
	case EBufferType::INDICES:
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, inHandle.id);
		break;
	}

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "bindGPUBuffer " << inHandle.id << "bytes failed: " << err << std::endl;
		}
	}
}

void RenderBackend::deleteGPUBuffer(EBufferType inType, GPUHandle inHandle)
{
	switch (inType)
	{
	case EBufferType::VERTEX_ARRAY:
		glDeleteVertexArrays(1, &inHandle.id);
		break;
	case EBufferType::VERTICES:
	case EBufferType::INDICES:
		glDeleteBuffers(1, &inHandle.id);
		break;
	}

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "deleteGPUBuffer " << inHandle.id << " failed: " << err << std::endl;
		}
	}
}

void RenderBackend::setGPUBufferData(EBufferType inType, size_t inOffset, size_t inSize, void* inData)
{
	GLenum type = getOpenGLBufferType(inType);
	glBufferSubData(type, inOffset, inSize, inData);
}

void RenderBackend::setVertexLayoutData(unsigned int inLayoutIndex, size_t inSizeInBytes, size_t inVertexStride, void* inDataOffset, unsigned int inDivisor)
{
	glEnableVertexAttribArray((GLuint)inLayoutIndex);
	glVertexAttribPointer((GLuint)inLayoutIndex, (GLint)inSizeInBytes, GL_FLOAT, GL_FALSE, (GLsizei)inVertexStride, inDataOffset);
	glVertexAttribDivisor((GLuint)inLayoutIndex, (GLuint)inDivisor);
}


namespace render
{
	RenderBackend* g_renderBackEnd = nullptr;

	bool initializeBackend()
	{
		g_renderBackEnd = new RenderBackend();
		g_renderBackEnd->initialize();
		return true;
	}

	void shutdownBackend()
	{
		if (g_renderBackEnd)
		{
			g_renderBackEnd->shutdown();
			delete g_renderBackEnd;
		}
		g_renderBackEnd = nullptr;
	}

	RenderBackend* getRenderBackend()
	{
		return g_renderBackEnd;
	}
}