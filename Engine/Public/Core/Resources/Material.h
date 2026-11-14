#pragma once

#include <string>

#include "Core/ResourceHandle.h"
#include "Render/ShaderStructs.h"

enum class EMaterialBlendMode : uint8_t
{
	OPAQUE
};

enum class EMaterialType : uint8_t
{
	LIT,
	UNLIT
};

enum class ERenderPass : uint8_t
{
	GEOMETRY
};

class Material
{
	std::string path;

	std::string name;
	EMaterialType type;
	EMaterialBlendMode blendMode;
	ERenderPass pass;
	std::string vertex;
	std::string fragment;

	ResourceHandle defaultMaterialInstance;
	ShaderParameters shaderParameters;

public:
	std::string_view getName() const { return name; }
	EMaterialType getType() const { return type; }
	EMaterialBlendMode getBlendMode() const { return blendMode; }
	ResourceHandle getDefaultMatInstance() const { return defaultMaterialInstance; }
	const ShaderParameters& getShaderParameters() const { return shaderParameters; }

	Material() = delete;
	Material(const std::string&& inPath);

	void createDefaultMaterialInstance(ResourceHandle inMaterialHandle);

private:
	bool loadFromPath(std::string_view inPath);
};