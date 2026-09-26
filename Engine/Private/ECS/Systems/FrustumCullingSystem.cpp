#include "ECS/Systems/FrustumCullingSystem.h"

#include "tracy/Tracy.hpp"

#include "Camera.h"
#include "Core/Engine.h"
#include "Core/Scene.h"
#include "ECS/Entity.h"
#include "ECS/EntityManager.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/SphereColliderComponent.h"
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

	bool isInFrustum(const glm::vec3& inPosition, float inRadius, float inScale, const Frustum& inFrustum)
	{
		float inScaledRadius = inRadius * inScale;
		return isOnOrInFrontOfPlane(inPosition, inScaledRadius, inFrustum.left)
			&& isOnOrInFrontOfPlane(inPosition, inScaledRadius, inFrustum.right)
			&& isOnOrInFrontOfPlane(inPosition, inScaledRadius, inFrustum.top)
			&& isOnOrInFrontOfPlane(inPosition, inScaledRadius, inFrustum.bottom)
			&& isOnOrInFrontOfPlane(inPosition, inScaledRadius, inFrustum.near)
			&& isOnOrInFrontOfPlane(inPosition, inScaledRadius, inFrustum.far);
	}
}

void FrustumCullingSystem::update(Scene& inScene)
{
	ZoneScoped;

	const Frustum& frustum = engine::getScene()->getCamera().getFrustum();

	for (size_t i = 0; i < inScene.getSphereColliderComponents().size(); ++i)
	{
		SphereColliderComponent* sphere = inScene.getSphereColliderComponents().at(i);
		ensure(sphere);

		WorldTransformComponent* worldTransform = inScene.getWorldTransformComponents().get(sphere->entity);
		ensure(worldTransform);

		glm::vec3 squaredScale = worldTransform->getSquaredScale();
		float maxScale = glm::sqrt(std::max({ squaredScale.x, squaredScale.y, squaredScale.z }));

		sphere->bInFrustum = isInFrustum(worldTransform->getPosition(), sphere->radius, maxScale, frustum);
	}
}