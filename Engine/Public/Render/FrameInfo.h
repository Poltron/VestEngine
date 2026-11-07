#pragma once

struct FrameInfo
{
	size_t renderedMeshTotal;
	size_t culledMeshTotal;
	size_t debugShapesTotal;

	size_t bindMat;
	size_t drawIndexed;

	void reset()
	{
		renderedMeshTotal = 0;
		culledMeshTotal = 0;
		debugShapesTotal = 0;
		bindMat = 0;
		drawIndexed = 0;
	}
};