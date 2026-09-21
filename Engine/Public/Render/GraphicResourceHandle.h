#pragma once

typedef unsigned int GraphicResourceHandle;

namespace graphicResourceHandle
{
	inline bool isValid(GraphicResourceHandle inHandle)
	{
		return inHandle > 0;
	}
}