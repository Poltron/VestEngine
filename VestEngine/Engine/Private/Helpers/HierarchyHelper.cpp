#include "Helpers/HierarchyHelper.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_common.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#include "Components/TransformComponent.h"
#include "Components/HierarchyComponent.h"
#include "Managers/ComponentManager.h"

namespace
{
	void GetWorldInLocal(const WorldTransformComponent& inWorld, const LocalTransformComponent& inLocal, glm::mat4& outNewLocal)
	{
		glm::mat4 invParentLocal = glm::inverse(inLocal.getLocalModelMatrix());
		outNewLocal = invParentLocal * inWorld.model;
	}

	void GetPosRotScaleFromMatrix(const glm::mat4& inMatrix, glm::vec3& outScale, glm::quat& outRotation, glm::vec3& outTranslation)
	{
		glm::vec3 skew;
		glm::vec4 perspective;

		bool bSuccess = glm::decompose(inMatrix
			, outScale
			, outRotation
			, outTranslation
			, skew
			, perspective);
		ensure(bSuccess);
	}


}

namespace hierarchyHelper
{
	void attachTo(HierarchyComponent* inElement
		, HierarchyComponent* inParent
		, EAttachmentRules inAttachmentRules
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms)
	{
		switch (inAttachmentRules)
		{
			case EAttachmentRules::KeepWorld:
			{
				WorldTransformComponent* worldTransform = inWorldTransforms.get(inElement->entity);
				LocalTransformComponent* parentLocalTransform = inLocalTransforms.get(inParent->entity);
				glm::mat4 newLocalTransform = glm::mat4(1.0f);
				GetWorldInLocal(*worldTransform, *parentLocalTransform, newLocalTransform);

				glm::vec3 scale = glm::vec3(1);
				glm::quat rotation = glm::quat({ 0,0,0 });
				glm::vec3 translation = glm::vec3(0);
				GetPosRotScaleFromMatrix(newLocalTransform, scale, rotation, translation);

				LocalTransformComponent* localTransform = inLocalTransforms.get(inElement->entity);
				localTransform->setLocalPosition(translation);
				localTransform->setLocalRotation(rotation);
				localTransform->setLocalScale(scale);
				break;
			}
			case EAttachmentRules::KeepRelative:
			{
				break;
			}
			case EAttachmentRules::SnapToTarget:
			{
				LocalTransformComponent* localTransform = inLocalTransforms.get(inElement->entity);
				localTransform->setLocalPosition(glm::vec3(0, 0, 0));
				localTransform->setLocalRotation(glm::vec3(0, 0, 0));
				localTransform->setLocalScale(glm::vec3(1, 1, 1));
				break;
			}
		}

		inElement->parent = inParent->entity;

		inElement->nextSibling = inParent->firstChild;
		inParent->firstChild = inElement->entity;
	}

	void detach(HierarchyComponent* inElement
		, EAttachmentRules inAttachmentRules
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms)
	{
		if (!EntityFuncs::isEntityValid(inElement->parent))
		{
			return;
		}

		switch (inAttachmentRules)
		{
			case EAttachmentRules::KeepWorld:
			{
				WorldTransformComponent* worldTransform = inWorldTransforms.get(inElement->entity);
				ensure(worldTransform);

				glm::vec3 scale = glm::vec3(1);
				glm::quat rotation = glm::quat({ 0,0,0 });
				glm::vec3 translation = glm::vec3(0);
				GetPosRotScaleFromMatrix(worldTransform->model, scale, rotation, translation);

				LocalTransformComponent* localTransform = inLocalTransforms.get(inElement->entity);
				localTransform->setLocalPosition(translation);
				localTransform->setLocalRotation(rotation);
				localTransform->setLocalScale(scale);
				break;
			}
			case EAttachmentRules::KeepRelative:
			{
				break;
			}
			case EAttachmentRules::SnapToTarget:
			{
				LocalTransformComponent* localTransform = inLocalTransforms.get(inElement->entity);
				ensure(localTransform);

				localTransform->setLocalPosition(glm::vec3(0, 0, 0));
				localTransform->setLocalRotation(glm::vec3(0, 0, 0));
				localTransform->setLocalScale(glm::vec3(1, 1, 1));
				break;
			}
		}

		HierarchyComponent* parentHierarchy = inHierarchies.get(inElement->parent);
		if (!parentHierarchy)
		{
			return;
		}

		if (parentHierarchy->firstChild == inElement->entity)
		{
			parentHierarchy->firstChild = inElement->nextSibling;

			inElement->parent = 0;
			inElement->nextSibling = 0;
			return;
		}

		HierarchyComponent* previousSiblingComponent = inHierarchies.get(parentHierarchy->firstChild);
		ensure(previousSiblingComponent);

		while (previousSiblingComponent->nextSibling != inElement->entity && EntityFuncs::isEntityValid(previousSiblingComponent->nextSibling))
		{
			previousSiblingComponent = inHierarchies.get(previousSiblingComponent->nextSibling);
		}

		ensure(previousSiblingComponent);
		ensure(previousSiblingComponent->nextSibling == inElement->entity);

		previousSiblingComponent->nextSibling = inElement->nextSibling;
		inElement->parent = 0;
		inElement->nextSibling = 0;
	}
}