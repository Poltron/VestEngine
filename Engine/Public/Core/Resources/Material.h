#pragma once

#include <vulkan/vulkan.h>

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
	ResourceHandle albedo;

public:
	Material() = delete;
	Material(EMaterialBlendMode inBlendMode, EMaterialType inType, ResourceHandle inAlbedo)
		: blendMode(inBlendMode), type(inType), albedo(inAlbedo)
	{}

	EMaterialType getType() const { return type; }
	EMaterialBlendMode getBlendMode() const { return blendMode; }
	ResourceHandle getAlbedo() const { return albedo; }
};