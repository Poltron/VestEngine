#pragma once

#include "glm/vec3.hpp"
#include <string>
#include <vector>

#include "Core/ResourceHandle.h"
#include "Render/GPUHandle.h"
#include "RenderBackend_Vulkan/VulkanShaderHelper.h"

class Material;

class MaterialInstance
{
	ResourceHandle materialHandle;

	BufferHandle materialBufferHandle;
	uint32_t descriptorSetHandle;

public:
	BufferHandle getMaterialBufferHandle() const { return materialBufferHandle; }
	uint32_t getDescriptorSetHandle() const { return descriptorSetHandle; }
	ResourceHandle getMaterial() const { return materialHandle; }

	MaterialInstance() = delete;
	MaterialInstance(const Material* inMaterial, ResourceHandle inMaterialHandle);
	~MaterialInstance();

	MaterialInstance(const MaterialInstance& inOther) = delete;
	MaterialInstance& operator=(const MaterialInstance& inOther) = delete;

	MaterialInstance(MaterialInstance&& inOther) noexcept;
	MaterialInstance& operator=(MaterialInstance&& inOther) noexcept;

	uint32_t getUint(std::string_view inName) const;
	glm::vec4 getVec4(std::string_view inName) const;

	void setVec3(std::string_view inName, const glm::vec3& inValue);
	void setVec4(std::string_view inName, const glm::vec4& inValue);
	void setMat4(std::string_view inName, const glm::mat4& inValue);
	void setTexture(std::string_view inName, ResourceHandle inValue);
};