#pragma once

#include "glm/glm.hpp"

// note : how should we manage model matrix ? 
// - should it update every time a transformation is applied ( so we don't need the individual pos/rot/sca ? )
//		> issue probably will come when adding parenting
// - should we have a dirty flag to only update when necessary ?

// note : rotations > should we use quat or vec3 here ?

struct TransformComponent
{
	unsigned int entityID;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::mat4 model;
};