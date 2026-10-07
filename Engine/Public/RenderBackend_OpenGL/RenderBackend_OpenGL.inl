#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include "glad/glad.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include "Core/Ensure.h"
#include "RenderBackend/RenderBackend.h"
#include "Render/GraphicResourceHandle.h"

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
}

class RenderBackend_OpenGL : public RenderBackend
{
protected:
	GLuint currentShaderProgram = 0;

public:
	void initialize()
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

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void shutdown()
	{

	}

	virtual void startTracyGPUZone(const std::string& inName) override
	{
		//ZoneScopedN(inName.c_str());
		//TracyGpuZone(inName);
	}

	virtual void tracyGpuCollect()
	{
		TracyGpuCollect;
	}

	virtual void clear() override
	{
		glStencilMask(0xFF);
		glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glStencilMask(0x00);
	}

	virtual void setStencilMask(unsigned int inMask) override
	{
		glStencilMask(inMask);
	}

	virtual void setStencilFunc(EComparisonFunc inFunc, int inValue, unsigned int inMask) override
	{
		GLenum func = getOpenGLComparisonFunc(inFunc);
		glStencilFunc(func, (GLint)inValue, (GLuint)inMask);
	}

	virtual GPUHandle compileShader(std::string_view inVertexPath, std::string_view inFragmentPath) override
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

	virtual void useShaderProgram(GPUHandle inHandle) override
	{
		glUseProgram(inHandle.id);

		while (GLenum err = glGetError())
		{
			if (err != GL_NO_ERROR)
			{
				std::cerr << "glUseProgram \"" << inHandle.id << "\" failed: " << err << std::endl;
			}
		}
	}

	virtual void setBool(std::string_view inName, bool inValue) override
	{
		int location = glGetUniformLocation(currentShaderProgram, inName.data());
		if (!isUniformLocationValid(location, inName))
		{
			return;
		}

		glUniform1i(location, (int)inValue);
	}

	virtual void setInt(std::string_view inName, int inValue) override
	{
		int location = glGetUniformLocation(currentShaderProgram, inName.data());
		if (!isUniformLocationValid(location, inName))
		{
			return;
		}

		glUniform1i(location, inValue);
	}

	virtual void setFloat(std::string_view inName, float inValue) override
	{
		int location = glGetUniformLocation(currentShaderProgram, inName.data());
		if (!isUniformLocationValid(location, inName))
		{
			return;
		}

		glUniform1f(location, inValue);
	}

	virtual void setVec3(std::string_view inName, const glm::vec3& inValue) override
	{
		int location = glGetUniformLocation(currentShaderProgram, inName.data());
		if (!isUniformLocationValid(location, inName))
		{
			return;
		}

		glUniform3f(location, inValue.x, inValue.y, inValue.z);
	}

	virtual void setVec4(std::string_view inName, float x, float y, float z, float w) override
	{
		int location = glGetUniformLocation(currentShaderProgram, inName.data());
		if (!isUniformLocationValid(location, inName))
		{
			return;
		}

		glUniform4f(location, x, y, z, w);
	}

	virtual void setMat4(std::string_view inName, glm::f32* inValue) override
	{
		int location = glGetUniformLocation(currentShaderProgram, inName.data());
		if (!isUniformLocationValid(location, inName))
		{
			return;
		}

		glUniformMatrix4fv(location, 1, GL_FALSE, inValue);
	}

	virtual void setTexture(std::string_view inName, GPUHandle inValue) override
	{
		int location = glGetUniformLocation(currentShaderProgram, inName.data());
		if (!isUniformLocationValid(location, inName.data()))
		{
			return;
		}

		glUniform1i(location, inValue.id);
	}

	//
	virtual void drawArrays(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inCount) override
	{
		bindGPUBuffer(EBufferType::VERTEX_ARRAY, inVAO);

		GLenum primitiveMode = getOpenGLPrimitiveMode(inPrimitiveMode);
		glDrawArrays(primitiveMode, inOffset, (GLsizei)inCount);

		bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);
	}

	virtual void drawArraysInstanced(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inSize, size_t inCount) override
	{
		bindGPUBuffer(EBufferType::VERTEX_ARRAY, inVAO);

		GLenum primitiveMode = getOpenGLPrimitiveMode(inPrimitiveMode);
		glDrawArraysInstanced(primitiveMode, inOffset, (GLsizei)inSize, (GLsizei)inCount);

		bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);
	}

	virtual void drawElements(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, size_t inCount, void* inIndices) override
	{
		bindGPUBuffer(EBufferType::VERTEX_ARRAY, inVAO);

		GLenum primitiveMode = getOpenGLPrimitiveMode(inPrimitiveMode);
		glDrawElements(primitiveMode, (GLsizei)inCount, GL_UNSIGNED_INT, inIndices);

		bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);
	}


	//
	virtual GPUHandle createTexture(uint8_t* inData) override
	{
		return {};
	}

	virtual void bindTexture(GPUHandle inHandle, std::string_view inType, size_t inIndex) override
	{
		glActiveTexture(GL_TEXTURE0 + (GLenum)inIndex);
		glBindTexture(GL_TEXTURE_2D, inHandle.id);
		setInt(std::string("material.").append(inType), (int)inIndex);
	}

	virtual void deleteTexture(GPUHandle inHandle) override
	{
		glDeleteTextures(1, &inHandle.id);
	}

	//
	virtual GPUHandle createGPUBuffer(EBufferType inType) override
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
		return handle;
	}

	virtual void allocateGPUBuffer(EBufferType inType, size_t inCapacity, void* inData, EBufferUsage inUsage) override
	{
		GLenum type = getOpenGLBufferType(inType);
		GLenum usage = getOpenGLBufferUsage(inUsage);
		glBufferData(type, inCapacity, inData, usage);
	}

	virtual void bindGPUBuffer(EBufferType inType, GPUHandle inHandle) override
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
	}

	virtual void deleteGPUBuffer(EBufferType inType, GPUHandle inHandle) override
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
	}

	virtual void setGPUBufferData(EBufferType inType, size_t inOffset, size_t inSize, void* inData) override
	{
		GLenum type = getOpenGLBufferType(inType);
		glBufferSubData(type, inOffset, inSize, inData);
	}
};


namespace render
{
	RenderBackend_OpenGL* g_renderBackEnd = nullptr;

	bool initializeBackend()
	{
		g_renderBackEnd = new RenderBackend_OpenGL();
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