#pragma once

#include <vector>

// todo : 
// - vector usage is temporary, use better data structure

class Mesh
{
public:
	static void createCube(Mesh& outMesh);

	std::vector<float> vertices;
};