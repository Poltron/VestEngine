#include "Render/DebugShapes.h"

#include "glad/glad.h"
#include "Render/GraphicResourceHandle.h"

#include <iostream>

DebugShapePrimitive::DebugShapePrimitive()
{
	VAO = 0;
	VBO = 0;

	setupMesh();
}

DebugShapePrimitive::~DebugShapePrimitive()
{
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

DebugShapePrimitive::DebugShapePrimitive(DebugShapePrimitive&& inOther) noexcept
	: VAO(inOther.VAO), VBO(inOther.VBO)
{
	inOther.VAO = 0;
	inOther.VBO = 0;
}

DebugShapePrimitive& DebugShapePrimitive::operator=(DebugShapePrimitive&& inOther) noexcept
{
	if (this != &inOther)
	{
		VAO = inOther.VAO;
		VBO = inOther.VBO;

		inOther.VAO = 0;
		inOther.VBO = 0;
	}
	return *this;
}

void DebugShapePrimitive::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	// note: unsafe if more than 100 primitives
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DebugShapePrimitive) * 100, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugShapePrimitiveInstance), (void*)(offsetof(DebugShapePrimitiveInstance, position)));

	glEnableVertexAttribArray(1); // color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugShapePrimitiveInstance), (void*)(offsetof(DebugShapePrimitiveInstance, color)));

	glEnableVertexAttribArray(2); // color
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(DebugShapePrimitiveInstance), (void*)(offsetof(DebugShapePrimitiveInstance, size)));

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);

	glBindVertexArray(0);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "glUseProgram failed: " << err << std::endl;
		}
	}
}

//
DebugShapeMesh::DebugShapeMesh(std::vector<glm::vec3>&& inVertices)
	: vertices(inVertices)
{
	VAO = 0;
	VBOmesh = 0;
	VBOinstances = 0;

	setupMesh();
}

DebugShapeMesh::~DebugShapeMesh()
{
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &VBOmesh);
	glDeleteBuffers(1, &VBOinstances);
}

DebugShapeMesh::DebugShapeMesh(DebugShapeMesh&& inOther) noexcept
	: VAO(inOther.VAO), VBOmesh(inOther.VBOmesh), VBOinstances(inOther.VBOinstances)
{
	vertices.clear();
	vertices = std::move(inOther.vertices);

	inOther.VAO = 0;
	inOther.VBOmesh = 0;
	inOther.VBOinstances = 0;
}

DebugShapeMesh& DebugShapeMesh::operator=(DebugShapeMesh&& inOther) noexcept
{
	if (this != &inOther)
	{
		vertices.clear();
		vertices = std::move(inOther.vertices);

		VAO = inOther.VAO;
		VBOmesh = inOther.VBOmesh;
		VBOinstances = inOther.VBOinstances;

		inOther.VAO = 0;
		inOther.VBOmesh = 0;
		inOther.VBOinstances = 0;
	}
	return *this;
}

void DebugShapeMesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBOmesh);
	glGenBuffers(1, &VBOinstances);

	glBindVertexArray(VAO);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBOmesh);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glVertexAttribDivisor(0, 0);

	// instances
	glBindBuffer(GL_ARRAY_BUFFER, VBOinstances);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, color)));

	glEnableVertexAttribArray(2); // model[0]
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model)));
	glEnableVertexAttribArray(3); // model[1]
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 1));
	glEnableVertexAttribArray(4); // model[2]
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 2));
	glEnableVertexAttribArray(5); // model[3]
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 3));

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glBindVertexArray(0);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "glUseProgram failed: " << err << std::endl;
		}
	}
}