#pragma once

#include "glm/gtc/matrix_transform.hpp"

namespace maths
{
	void computeTransformMatrix(const glm::vec3& inPosition
		, const glm::vec3& inRotation
		, const glm::vec3& inScale
		, glm::mat4& outMatrix)
	{
		outMatrix = glm::mat4(1.0f);
		outMatrix = glm::translate(outMatrix, inPosition);

		outMatrix = glm::rotate(outMatrix, glm::radians(inRotation.x), glm::vec3(1, 0, 0));
		outMatrix = glm::rotate(outMatrix, glm::radians(inRotation.y), glm::vec3(0, 1, 0));
		outMatrix = glm::rotate(outMatrix, glm::radians(inRotation.z), glm::vec3(0, 0, 1));

		outMatrix = glm::scale(outMatrix, inScale);
	}
}