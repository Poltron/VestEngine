#pragma once

//#include "Render/ShaderParameterCollection.h"

#include "Core/ResourceHandle.h"
#include "ECS/Entity.h"
#include "Render/GPUHandle.h"

struct MeshRendererComponent 
{
	Entity entity;
	
	ResourceHandle model;
	ResourceHandle materialInstance;

	//GPUHandle shader;
	//bool bOutline;

	//ShaderParameterCollection shaderParameters;

	MeshRendererComponent();
	~MeshRendererComponent();

	MeshRendererComponent(const MeshRendererComponent& inOther) = delete;
	MeshRendererComponent& operator=(const MeshRendererComponent& inOther) = delete;

	MeshRendererComponent(MeshRendererComponent&& inOther) noexcept;
	MeshRendererComponent& operator=(MeshRendererComponent&& inOther) noexcept;
};