#pragma once

#include "Core/ResourceHandle.h"
#include "GraphicResourceHandle.h"

struct PipelineObject
{
	bool bDepthTest;
	bool bWireframe;
	ResourceHandle vertexShader;
	ResourceHandle fragmentShader;

	PipelineObject(bool bInDepthTest, bool bInWireframe, ResourceHandle inVertexShader, ResourceHandle inFragmentShader)
		: bDepthTest(bInDepthTest), bWireframe(bInWireframe), vertexShader(inVertexShader), fragmentShader(inFragmentShader)
	{}
};

struct BufferObject
{

};

struct TextureObject
{

};