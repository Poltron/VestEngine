#pragma once

#include "glm/glm.hpp"

#include "Render/PrimitiveMesh.h"

namespace primitiveMeshGenerationHelper
{
	PrimitiveMesh createBox();
	PrimitiveMesh createSphere(int inNumSegments);
}