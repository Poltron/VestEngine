#pragma once

#include "ECS/Entity.h"

template<typename T>
class ComponentManager;

struct HierarchyComponent;
struct LocalTransformComponent;
struct WorldTransformComponent;

enum class EAttachmentRules : unsigned int
{
	KeepWorld = 0,
	KeepRelative = 1,
	SnapToTarget = 2
};

namespace hierarchyHelper
{
	void attachTo(HierarchyComponent* inElement
		, HierarchyComponent* inParent
		, EAttachmentRules inAttachmentRules
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms);
	void detach(HierarchyComponent* inElement
		, EAttachmentRules inAttachmentRules
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms);
}