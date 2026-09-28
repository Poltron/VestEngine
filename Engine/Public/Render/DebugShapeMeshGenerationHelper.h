#pragma once

#include "glm/glm.hpp"

namespace debugShapeMeshGenerationHelper
{
	void createBox(std::vector<glm::vec3>& outVertices);
	void createSphere(int inNumSegments, std::vector<glm::vec3>& outVertices);
}