#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

// note : how should we manage model matrix ? 
// - should it update every time a transformation is applied ( so we don't need the individual pos/rot/sca ? )
//		> issue probably will come when adding parenting
// - should we have a dirty flag to only update when necessary ?

// note : rotations > should we use quat or vec3 here ?

struct TransformComponent
{
	unsigned int entityID = 0;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::mat4 model;

	TransformComponent()
	{
		entityID = 0;

		position = glm::vec3();
		rotation = glm::vec3();
		scale = glm::vec3(1, 1, 1);

		model = glm::identity<glm::mat4>();
	}

	glm::vec3 getModelForward() const
	{
		return glm::normalize(glm::vec3(model[2].x, model[2].y, model[2].z));
	}
};