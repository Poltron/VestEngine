#pragma once

#include "glm/gtc/matrix_transform.hpp"

namespace maths
{
	inline void computeTransformMatrix(const glm::vec3& inPosition
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

	inline glm::vec3 getMatrixTranslation(const glm::mat4& inMatrix)
	{
		return  { inMatrix[3][0], inMatrix[3][1], inMatrix[3][2] };
	}

	inline glm::vec3 getMatrixForward(const glm::mat4& inMatrix)
	{
		return glm::normalize(glm::vec3(inMatrix[2].x, inMatrix[2].y, inMatrix[2].z));
	}

	inline glm::vec3 getMatrixScale(const glm::mat4& inMatrix)
	{
		float scaleX = glm::length(glm::vec3(inMatrix[0][0], inMatrix[0][1], inMatrix[0][2]));
		float scaleY = glm::length(glm::vec3(inMatrix[1][0], inMatrix[1][1], inMatrix[1][2]));
		float scaleZ = glm::length(glm::vec3(inMatrix[2][0], inMatrix[2][1], inMatrix[2][2]));
		return { scaleX, scaleY, scaleZ };
	}

	inline glm::vec3 getMatrixSquaredScale(const glm::mat4& inMatrix)
	{
		float scaleX = inMatrix[0][0] * inMatrix[0][0] + inMatrix[0][1] * inMatrix[0][1] + inMatrix[0][2] * inMatrix[0][2];
		float scaleY = inMatrix[1][0] * inMatrix[1][0] + inMatrix[1][1] * inMatrix[1][1] + inMatrix[1][2] * inMatrix[1][2];
		float scaleZ = inMatrix[2][0] * inMatrix[2][0] + inMatrix[2][1] * inMatrix[2][1] + inMatrix[2][2] * inMatrix[2][2];
		return { scaleX, scaleY, scaleZ };
	}
}