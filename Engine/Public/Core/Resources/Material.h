#pragma once

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

public:
	Material() = delete;
	Material(EMaterialBlendMode inBlendMode, EMaterialType inType)
		: blendMode(inBlendMode), type(inType)
	{}

	EMaterialType getType() const { return type; }
	EMaterialBlendMode getBlendMode() const { return blendMode; }
};