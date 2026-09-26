#pragma once

#include "glm/glm.hpp"

#include "Render/DebugShapes.h"

namespace debugShapeMeshGenerationHelper
{
	DebugShapeMesh createBox();
	DebugShapeMesh createSphere(int inNumSegments);
}