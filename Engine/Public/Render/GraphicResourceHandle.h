#pragma once

typedef unsigned int GraphicResourceHandle;

namespace graphicResourceHandle
{
	inline bool isValid(GraphicResourceHandle inHandle)
	{
		return inHandle > 0;
	}
}

namespace render
{
	enum class EPrimitiveType
	{
		POINTS,
		LINES,
		TRIANGLES
	};
}