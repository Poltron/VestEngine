#pragma once

struct FrameInfo
{
	size_t renderedMeshTotal;
	size_t culledMeshTotal;
	size_t debugShapesTotal;

	size_t shaderPrograms;
	size_t drawCalls;

	void reset()
	{
		renderedMeshTotal = 0;
		culledMeshTotal = 0;
		debugShapesTotal = 0;
		shaderPrograms = 0;
		drawCalls = 0;
	}
};