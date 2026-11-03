#include "ECS/Components/MeshRendererComponent.h"

MeshRendererComponent::MeshRendererComponent()
	: entity(ENTITY_INVALID), model(ResourceHandle::invalid), materialInstance(ResourceHandle::invalid)//, bOutline(false)
{}

MeshRendererComponent::~MeshRendererComponent()
{}

MeshRendererComponent::MeshRendererComponent(MeshRendererComponent&& inOther) noexcept
	: entity(inOther.entity), model(inOther.model), materialInstance(inOther.materialInstance)
{
	inOther.entity = Entity();
	inOther.model = ResourceHandle::invalid;
	inOther.materialInstance = ResourceHandle::invalid;
}

MeshRendererComponent& MeshRendererComponent::operator=(MeshRendererComponent&& inOther) noexcept
{
	if (this != &inOther)
	{
		entity = inOther.entity;
		model = inOther.model;
		materialInstance = inOther.materialInstance;

		inOther.entity = Entity();
		inOther.model = ResourceHandle::invalid;
		inOther.materialInstance = ResourceHandle::invalid;
	}
	return *this;
}