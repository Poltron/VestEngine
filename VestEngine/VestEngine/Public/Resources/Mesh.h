#pragma once

#include <vector>

// todo : 
// - vector usage is temporary, use better data structure

class Mesh
{
public:
	static void createTextureCube(Mesh& outMesh);
	static void createNormalCube(Mesh& outMesh);
	static void createNormalTextureCube(Mesh& outMesh);

	std::vector<float> vertices;
};