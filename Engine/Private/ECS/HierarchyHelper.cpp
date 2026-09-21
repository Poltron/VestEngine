#include "ECS/HierarchyHelper.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_common.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#include "Core/Scene.h"
#include "ECS/ComponentManager.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/HierarchyComponent.h"

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
	void attachTo(Scene& inScene
		, HierarchyComponent* inElement
		, HierarchyComponent* inParent
		, EAttachmentRules inAttachmentRules)
	{
		switch (inAttachmentRules)
		{
			case EAttachmentRules::KeepWorld:
			{
				WorldTransformComponent* worldTransform = inScene.getWorldTransformComponents().get(inElement->entity);
				LocalTransformComponent* parentLocalTransform = inScene.getLocalTransformComponents().get(inParent->entity);
				glm::mat4 newLocalTransform = glm::mat4(1.0f);
				GetWorldInLocal(*worldTransform, *parentLocalTransform, newLocalTransform);

				glm::vec3 scale = glm::vec3(1);
				glm::quat rotation = glm::quat();
				glm::vec3 translation = glm::vec3();
				GetPosRotScaleFromMatrix(newLocalTransform, scale, rotation, translation);

				LocalTransformComponent* localTransform = inScene.getLocalTransformComponents().get(inElement->entity);
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
				LocalTransformComponent* localTransform = inScene.getLocalTransformComponents().get(inElement->entity);
				localTransform->setLocalPosition(glm::vec3(0, 0, 0));
				localTransform->setLocalRotation(glm::quat());
				localTransform->setLocalScale(glm::vec3(1, 1, 1));
				break;
			}
		}

		inElement->parent = inParent->entity;
		inElement->bDirty = true;

		inElement->nextSibling = inParent->firstChild;
		inParent->firstChild = inElement->entity;
	}

	void detach(Scene& inScene
		, HierarchyComponent* inElement
		, EAttachmentRules inAttachmentRules)
	{
		if (!EntityFuncs::isEntityValid(inElement->parent))
		{
			return;
		}

		switch (inAttachmentRules)
		{
			case EAttachmentRules::KeepWorld:
			{
				WorldTransformComponent* worldTransform = inScene.getWorldTransformComponents().get(inElement->entity);
				ensure(worldTransform);

				glm::vec3 scale = glm::vec3(1);
				glm::quat rotation = glm::quat();
				glm::vec3 translation = glm::vec3();
				GetPosRotScaleFromMatrix(worldTransform->model, scale, rotation, translation);
				
				LocalTransformComponent* localTransform = inScene.getLocalTransformComponents().get(inElement->entity);
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
				LocalTransformComponent* localTransform = inScene.getLocalTransformComponents().get(inElement->entity);
				ensure(localTransform);

				localTransform->setLocalPosition(glm::vec3(0, 0, 0));
				localTransform->setLocalRotation(glm::quat());
				localTransform->setLocalScale(glm::vec3(1, 1, 1));
				break;
			}
		}

		HierarchyComponent* parentHierarchy = inScene.getHierarchyComponents().get(inElement->parent);
		if (!parentHierarchy)
		{
			return;
		}

		if (parentHierarchy->firstChild == inElement->entity)
		{
			parentHierarchy->firstChild = inElement->nextSibling;

			inElement->parent = 0;
			inElement->nextSibling = 0;
			inElement->bDirty = true;
			return;
		}

		HierarchyComponent* previousSiblingComponent = inScene.getHierarchyComponents().get(parentHierarchy->firstChild);
		ensure(previousSiblingComponent);

		while (previousSiblingComponent->nextSibling != inElement->entity && EntityFuncs::isEntityValid(previousSiblingComponent->nextSibling))
		{
			previousSiblingComponent = inScene.getHierarchyComponents().get(previousSiblingComponent->nextSibling);
		}

		ensure(previousSiblingComponent);
		ensure(previousSiblingComponent->nextSibling == inElement->entity);

		previousSiblingComponent->nextSibling = inElement->nextSibling;
		inElement->parent = 0;
		inElement->nextSibling = 0;
		inElement->bDirty = true;
	}
}