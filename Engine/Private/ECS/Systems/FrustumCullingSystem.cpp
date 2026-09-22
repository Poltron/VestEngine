#include "ECS/Systems/FrustumCullingSystem.h"

#include "tracy/Tracy.hpp"

#include "Camera.h"
#include "Core/Engine.h"
#include "Core/Scene.h"
#include "ECS/Entity.h"
#include "ECS/EntityManager.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/SphereBoundingVolumeComponent.h"
#include "Render/Renderer.h"

namespace
{
	float getSignedDistanceToPlane(const glm::vec3& inPosition, const Plane& inPlane)
	{
		return glm::dot(inPlane.normal, inPosition - inPlane.origin);
	}

	bool isOnOrInFrontOfPlane(const glm::vec3& inPosition, float inRadius, const Plane& inPlane)
	{
		return getSignedDistanceToPlane(inPosition, inPlane) > -inRadius;
	}

	bool isInFrustum(const glm::vec3& inPosition, float inRadius, const Frustum& inFrustum)
	{
		return isOnOrInFrontOfPlane(inPosition, inRadius, inFrustum.left)
			&& isOnOrInFrontOfPlane(inPosition, inRadius, inFrustum.right)
			&& isOnOrInFrontOfPlane(inPosition, inRadius, inFrustum.top)
			&& isOnOrInFrontOfPlane(inPosition, inRadius, inFrustum.bottom)
			&& isOnOrInFrontOfPlane(inPosition, inRadius, inFrustum.near)
			&& isOnOrInFrontOfPlane(inPosition, inRadius, inFrustum.far);
	}
}

void FrustumCullingSystem::update(Scene& inScene)
{
	ZoneScoped;

	const Frustum& frustum = engine::getScene()->getCamera().getFrustum();

	for (size_t i = 0; i < inScene.getSphereBoundingVolumeComponents().size(); ++i)
	{
		SphereBoundingVolumeComponent* sphere = inScene.getSphereBoundingVolumeComponents().at(i);
		ensure(sphere);

		WorldTransformComponent* worldTransform = inScene.getWorldTransformComponents().get(sphere->entity);
		ensure(worldTransform);

		sphere->bInFrustum = isInFrustum(worldTransform->getPosition(), sphere->radius, frustum);
	}
}