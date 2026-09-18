#pragma once

#include "ECS/EntityManager.h"

struct DirectionalLightComponent;
struct HierarchyComponent;
struct LocalTransformComponent;
struct MeshRendererComponent;
struct PointLightComponent;
struct RigidbodyComponent;
struct WorldTransformComponent;

struct ResourceHandle;

template<typename T>
class ComponentManager;

namespace EntityFactory
{
};