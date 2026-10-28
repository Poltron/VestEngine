#pragma once

#include <vulkan/vulkan.h>

#include "Render/Renderer.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/RenderDevice.h"

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

	DescriptorSetHandle descriptorSetHandle;

public:
	EMaterialType getType() const { return type; }
	EMaterialBlendMode getBlendMode() const { return blendMode; }
	ResourceHandle getAlbedo() const { return albedo; }
	DescriptorSetHandle getDescriptorSetHandle() const { return descriptorSetHandle; }

	Material() = delete;
	Material(EMaterialBlendMode inBlendMode, EMaterialType inType, ResourceHandle inAlbedo)
		: blendMode(inBlendMode), type(inType), albedo(inAlbedo)
	{
		descriptorSetHandle = render::getRenderBackend()->getDevice().createMaterialDescriptorSets(inAlbedo, RenderDevice::getFrameCount());
	}

	Material(const Material& inOther) = delete;
	Material& operator=(const Material& inOther) = delete;

	Material(Material&& inOther) noexcept
	{
		blendMode = inOther.blendMode;
		type = inOther.type;
		albedo = inOther.albedo;
		descriptorSetHandle = inOther.descriptorSetHandle;

		inOther.albedo = {};
		inOther.descriptorSetHandle = {};
	}

	Material& operator=(Material&& inOther) noexcept
	{
		if (this == &inOther)
		{
			blendMode = inOther.blendMode;
			type = inOther.type;
			albedo = inOther.albedo;
			descriptorSetHandle = inOther.descriptorSetHandle;

			inOther.albedo = {};
			inOther.descriptorSetHandle = {};
		}

		return *this;
	}

	~Material()
	{
		render::getRenderBackend()->getDevice().destroyMaterialDescriptorSets(descriptorSetHandle);
	}
};