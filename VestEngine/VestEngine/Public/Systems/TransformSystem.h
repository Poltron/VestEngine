#pragma once

#include "Components/TransformComponent.h"

class TransformSystem
{
public:
	void update(ComponentManager<TransformComponent>& inTransforms, double inDeltaTime)
	{
		for (size_t i = 0; i < inTransforms.size(); ++i)
		{
			TransformComponent* transform = inTransforms.at(i);
			assert(transform != nullptr);

			transform->model = glm::translate(glm::mat4(1.0), transform->position);

			transform->model = glm::rotate(transform->model, transform->rotation.x, glm::vec3(1, 0, 0));
			transform->model = glm::rotate(transform->model, transform->rotation.y, glm::vec3(0, 1, 0));
			transform->model = glm::rotate(transform->model, transform->rotation.z, glm::vec3(0, 0, 1));

			transform->model = glm::scale(transform->model, transform->scale);
		}
	}
};
