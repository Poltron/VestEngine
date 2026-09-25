#pragma once

#include "glm/glm.hpp"

#include "Render/GraphicResourceHandle.h"

struct PointPrimitiveInstance
{
	glm::vec3 position;
	float size;
	glm::vec3 color;
};

struct LinePrimitiveInstance
{
	glm::vec3 start;
	glm::vec3 end;
	glm::vec3 color;
};

struct BoxPrimitiveInstance
{
	glm::mat4 model;
	glm::vec3 color;
};

struct SpherePrimitiveInstance
{
	glm::mat4 model;
	glm::vec3 color;
};

struct PrimitiveVertex
{
	glm::vec3 position;

	PrimitiveVertex(float inX, float inY, float inZ)
	{
		position = glm::vec3(inX, inY, inZ);
	}

	PrimitiveVertex(const glm::vec3& inXYZ)
	{
		position = inXYZ;
	}
};

class PrimitiveMesh
{
	std::vector<PrimitiveVertex> vertices;
	render::EPrimitiveType primitiveType;

	GraphicResourceHandle VAO;
	GraphicResourceHandle VBO;
	GraphicResourceHandle EBO;

public:
	PrimitiveMesh() = delete;
	PrimitiveMesh(std::vector<PrimitiveVertex>&& inVertices, render::EPrimitiveType inPrimitiveType);
	~PrimitiveMesh();

	PrimitiveMesh(const PrimitiveMesh& inOther) = delete;
	PrimitiveMesh& operator=(const PrimitiveMesh& inOther) = delete;

	PrimitiveMesh(PrimitiveMesh&& inOther) noexcept;
	PrimitiveMesh& operator=(PrimitiveMesh&& inOther) noexcept;

	void bind() const;
	void draw() const;
	void unbind() const;

	render::EPrimitiveType getPrimitiveType() const { return primitiveType; };
	GraphicResourceHandle getVAO() const { return VAO; }
	GraphicResourceHandle getVBO() const { return VBO; }
	GraphicResourceHandle getEBO() const { return EBO; }


private:
	void setupMesh();
};

