#pragma once

#include "ECS/Entity.h"

template<typename T>
class ComponentManager;
class Scene;

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
	void attachTo(Scene& inScene
		, HierarchyComponent* inElement
		, HierarchyComponent* inParent
		, EAttachmentRules inAttachmentRules);
	void detach(Scene& inScene
		, HierarchyComponent* inElement
		, EAttachmentRules inAttachmentRules);
}