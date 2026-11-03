#pragma once

#include <string>

#include "Core/ResourceHandle.h"
#include "RenderBackend_Vulkan/ShaderStructs.h"

enum class EMaterialBlendMode
{
	OPAQUE
};

enum class EMaterialType
{
	LIT,
	UNLIT
};

class Material
{
	EMaterialBlendMode blendMode;
	EMaterialType type;
	ResourceHandle defaultMaterialInstance;
	ShaderParameters shaderParameters;

public:
	EMaterialType getType() const { return type; }
	EMaterialBlendMode getBlendMode() const { return blendMode; }
	ResourceHandle getDefaultMatInstance() const { return defaultMaterialInstance; }
	const ShaderParameters& getShaderParameters() const { return shaderParameters; }

	Material() = delete;
	Material(EMaterialBlendMode inBlendMode, EMaterialType inType, const ShaderParameters& inShaderParameters);

	void createDefaultMaterialInstance(ResourceHandle inMaterialHandle);
};