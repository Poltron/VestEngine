#include "ECS/Components/MeshRendererComponent.h"

#include "Core/Engine.h"
#include "Core/ResourcesManager.h"
#include "Render/Renderer.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/RenderDevice.h"
#include "RenderBackend_Vulkan/RenderPipeline.h"

MeshRendererComponent::MeshRendererComponent()
	: entity(ENTITY_INVALID), model(RESOURCE_INVALID)//, shader({}), bOutline(false)
{}

MeshRendererComponent::~MeshRendererComponent()
{}

MeshRendererComponent::MeshRendererComponent(MeshRendererComponent&& inOther) noexcept
	: entity(inOther.entity), model(inOther.model), materialInstance(inOther.materialInstance)//, ubo(inOther.ubo), descriptorSets(inOther.descriptorSets)
{
	inOther.entity = Entity();
	inOther.model = ResourceHandle();
	inOther.materialInstance = ResourceHandle();
	//inOther.ubo = UBOHandle(0, 0);
	//inOther.descriptorSets = {};
}

MeshRendererComponent& MeshRendererComponent::operator=(MeshRendererComponent&& inOther) noexcept
{
	if (this != &inOther)
	{
		entity = inOther.entity;
		model = inOther.model;
		materialInstance = inOther.materialInstance;
		//ubo = inOther.ubo;
		//descriptorSets = inOther.descriptorSets;

		inOther.entity = Entity();
		inOther.model = ResourceHandle();
		inOther.materialInstance = ResourceHandle();
		//inOther.ubo = UBOHandle(0, 0);
		//inOther.descriptorSets = DescriptorSetHandle(0, 0);
	}
	return *this;
}