#include "Render/DrawPrimitivesHelper.h"

#include <functional>

#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Render/PrimitiveMesh.h"

namespace
{
	void generateCircle(const glm::mat4& inRotationMatrix, unsigned int inNumSegments, std::function<float(float)> inXFunc, std::function<float(float)> inYFunc, std::vector<PrimitiveVertex>& outVertices)
	{
		const glm::vec3 startVertex = inRotationMatrix * glm::vec4(inXFunc(0), inYFunc(0), 0, 1);
		glm::vec3 previousVertex = startVertex;
		for (size_t j = 0; j < inNumSegments; ++j)
		{
			const float step = glm::pi<float>() * 2 * float(j) / (inNumSegments);
			const float dX = inXFunc(step);
			const float dY = inYFunc(step);

			const glm::vec3 vertex = inRotationMatrix * glm::vec4(dX, dY, 0, 1);
			outVertices.push_back(previousVertex);
			outVertices.push_back(vertex);
			previousVertex = vertex;
		}

		outVertices.push_back(previousVertex);
		outVertices.push_back(startVertex);
	}
}

namespace primitiveMeshGenerationHelper
{
	PrimitiveMesh createBox()
	{
		//
		//      5---6
		//	  1-+-2 |
		//    | 8-+-7
		//    4---3
		//

		std::vector<PrimitiveVertex> vertices;

		float halfSizeX = 0.5f;
		float halfSizeY = 0.5f;
		float halfSizeZ = 0.5f;

		const glm::vec3 vertex1 = glm::vec4(- halfSizeX,   halfSizeY,   halfSizeZ, 1.0f);
		const glm::vec3 vertex2 = glm::vec4(  halfSizeX,   halfSizeY,   halfSizeZ, 1.0f);
		const glm::vec3 vertex3 = glm::vec4(  halfSizeX, - halfSizeY,   halfSizeZ, 1.0f);
		const glm::vec3 vertex4 = glm::vec4(- halfSizeX, - halfSizeY,   halfSizeZ, 1.0f);
									
		const glm::vec3 vertex5 = glm::vec4(- halfSizeX,   halfSizeY, - halfSizeZ, 1.0f);
		const glm::vec3 vertex6 = glm::vec4(  halfSizeX,   halfSizeY, - halfSizeZ, 1.0f);
		const glm::vec3 vertex7 = glm::vec4(  halfSizeX, - halfSizeY, - halfSizeZ, 1.0f);
		const glm::vec3 vertex8 = glm::vec4(- halfSizeX, - halfSizeY, - halfSizeZ, 1.0f);

		vertices.push_back(vertex1);
		vertices.push_back(vertex2);
		vertices.push_back(vertex2);
		vertices.push_back(vertex3);
		vertices.push_back(vertex3);
		vertices.push_back(vertex4);
		vertices.push_back(vertex4);
		vertices.push_back(vertex1);

		vertices.push_back(vertex1);
		vertices.push_back(vertex2);
		vertices.push_back(vertex2);
		vertices.push_back(vertex3);
		vertices.push_back(vertex3);
		vertices.push_back(vertex4);
		vertices.push_back(vertex4);
		vertices.push_back(vertex1);
		
		vertices.push_back(vertex5);
		vertices.push_back(vertex6);
		vertices.push_back(vertex6);
		vertices.push_back(vertex7);
		vertices.push_back(vertex7);
		vertices.push_back(vertex8);
		vertices.push_back(vertex8);
		vertices.push_back(vertex5);
		
		vertices.push_back(vertex1);
		vertices.push_back(vertex5);
		vertices.push_back(vertex2);
		vertices.push_back(vertex6);
		vertices.push_back(vertex3);
		vertices.push_back(vertex7);
		vertices.push_back(vertex4);
		vertices.push_back(vertex8);

		return PrimitiveMesh(std::move(vertices), render::EPrimitiveType::LINES);
	}

	PrimitiveMesh createSphere(int inNumSegments)
	{
		std::vector<PrimitiveVertex> vertices;

		std::function<float(float)> cosFunc = [](float f) {return cos(f); };
		std::function<float(float)> sinFunc = [](float f) {return sin(f); };

		glm::mat4 rotationMatrix = glm::identity<glm::mat4>();
		const float circleStep = 180.0f / inNumSegments;
		for (size_t i = 0; i < inNumSegments; ++i)
		{
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(circleStep), glm::vec3(0, 1, 0));

			generateCircle(rotationMatrix, inNumSegments * 2, cosFunc, sinFunc, vertices);
		}

		rotationMatrix = glm::identity<glm::mat4>();
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));

		generateCircle(rotationMatrix, inNumSegments * 2, sinFunc, cosFunc, vertices);

		return PrimitiveMesh(std::move(vertices), render::EPrimitiveType::LINES);
	}
}