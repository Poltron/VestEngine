#include "Render/PrimitiveMesh.h"

#include "glad/glad.h"
#include "Render/GraphicResourceHandle.h"

PrimitiveMesh::PrimitiveMesh(std::vector<PrimitiveVertex>&& inVertices, render::EPrimitiveType inPrimitiveType)
	: vertices(inVertices), primitiveType(inPrimitiveType)
{
	VAO = 0;
	VBO = 0;

	setupMesh();
}

PrimitiveMesh::~PrimitiveMesh()
{
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

PrimitiveMesh::PrimitiveMesh(PrimitiveMesh&& inOther) noexcept
	: primitiveType(inOther.primitiveType), VAO(inOther.VAO), VBO(inOther.VBO)
{
	vertices.clear();
	vertices = std::move(inOther.vertices);

	inOther.VAO = 0;
	inOther.VBO = 0;
}

PrimitiveMesh& PrimitiveMesh::operator=(PrimitiveMesh&& inOther) noexcept
{
	if (this != &inOther)
	{
		vertices.clear();
		vertices = std::move(inOther.vertices);
		primitiveType = inOther.primitiveType;

		VAO = inOther.VAO;
		VBO = inOther.VBO;

		inOther.VAO = 0;
		inOther.VBO = 0;
	}
	return *this;
}

void PrimitiveMesh::bind() const
{
	glBindVertexArray(VAO);
}

void PrimitiveMesh::unbind() const
{
	glBindVertexArray(0);
}

void PrimitiveMesh::draw() const
{
	GLenum glPrimitiveType = 0;
	switch (primitiveType)
	{
		case render::EPrimitiveType::POINTS:
		{
			glPrimitiveType = GL_POINTS;
			break;
		}
		case render::EPrimitiveType::LINES:
		{
			glPrimitiveType = GL_LINES;
			break;
		}
		case render::EPrimitiveType::TRIANGLES:
		{
			glPrimitiveType = GL_TRIANGLES;
			break;
		}
	}

	glDrawArrays(glPrimitiveType, 0, (GLsizei)vertices.size());
}

void PrimitiveMesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PrimitiveVertex), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PrimitiveVertex), (void*)0);

	glBindBuffer(GL_VERTEX_ARRAY_BINDING, 0);
}