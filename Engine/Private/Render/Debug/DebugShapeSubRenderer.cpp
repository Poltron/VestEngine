#include "Render/Debug/DebugShapeSubRenderer.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "tracy/Tracy.hpp"

#include "Core/Engine.h"
#include "Core/Ensure.h"
#include "Core/Maths.h"
#include "Core/ResourcesManager.h"
#include "Core/Resources/Shader.h"
#include "Render/Debug/DebugShapeMeshGenerationHelper.h"
#include "Render/Renderer.h"

void DebugShapeSubRenderer::loadShaders()
{
	const std::string WorkDirTMP = WORKDIR;
	const std::string debugMeshVertexPath = WorkDirTMP + "/Resources/Shaders/debug_mesh.vert";
	const std::string debugFragmentPath = WorkDirTMP + "/Resources/Shaders/debug.frag";
	debugMeshShaderHandle = engine::getResources()->loadShader(debugMeshVertexPath, debugFragmentPath);

	const std::string debugPrimitiveVertexPath = WorkDirTMP + "/Resources/Shaders/debug_primitive.vert";
	debugPrimitiveShaderHandle = engine::getResources()->loadShader(debugPrimitiveVertexPath, debugFragmentPath);
}

void DebugShapeSubRenderer::loadDebugShapes()
{
	ZoneScoped;

	// todo: refacto needed
	// right now its following enum order but i can't resize + assign
	// with enum index since there's no default constructor

	debugShapePrimitives.push_back(DebugShapePrimitive(10));
	debugShapePrimitives.push_back(DebugShapePrimitive(10));

	std::vector<glm::vec3> boxVertices;
	debugShapeMeshGenerationHelper::createBox(boxVertices);
	DebugShapeMesh box(std::move(boxVertices), 500);
	debugShapeMeshes.push_back(std::move(box));

	std::vector<glm::vec3> sphereVertices;
	debugShapeMeshGenerationHelper::createSphere(6, sphereVertices);
	DebugShapeMesh sphere(std::move(sphereVertices), 500);
	debugShapeMeshes.push_back(std::move(sphere));
}

void DebugShapeSubRenderer::updateDebugShapes(Scene& inScene)
{
	updateEntityDebugShapesSystem.update(entitySphereInstances, inScene);
}

void DebugShapeSubRenderer::render(Renderer& inRenderer, FrameInfo& inFrameInfo)
{
	Shader* shader = inRenderer.useShaderProgram(debugPrimitiveShaderHandle);
	ensure(shader);

	glm::mat4 viewProjection = inRenderer.getActiveCamera().getProjectionMatrix() * inRenderer.getActiveCamera().getViewMatrix();
	shader->setMat4("uViewProjection", glm::value_ptr(viewProjection));

	{
		ZoneScopedN("render point primitives");

		DebugShapePrimitive& pointPrimitive = debugShapePrimitives[(unsigned int)EDebugShapePrimitive::POINT];
		pointPrimitive.resetInstanceCount();
		pointPrimitive.updateVBO(pointDebugShapeInstances);
		inRenderer.drawArrays(pointPrimitive.getVAO(), EPrimitiveMode::POINTS, 0, pointDebugShapeInstances.size());
		inFrameInfo.debugShapesTotal += pointDebugShapeInstances.size();
	}

	{
		ZoneScopedN("render line primitives");

		DebugShapePrimitive& linePrimitive = debugShapePrimitives[(unsigned int)EDebugShapePrimitive::LINE];
		linePrimitive.resetInstanceCount();
		linePrimitive.updateVBO(lineDebugShapeInstances);
		inRenderer.drawArrays(linePrimitive.getVAO(), EPrimitiveMode::LINES, 0, lineDebugShapeInstances.size());
		inFrameInfo.debugShapesTotal += lineDebugShapeInstances.size() / 2;
	}

	shader = inRenderer.useShaderProgram(debugMeshShaderHandle);
	ensure(shader);

	shader->setMat4("uViewProjection", glm::value_ptr(viewProjection));

	{
		ZoneScopedN("render box meshes");

		DebugShapeMesh& boxMesh = debugShapeMeshes[(unsigned int)EDebugShapeMesh::BOX];
		boxMesh.resetInstances();
		boxMesh.addInstances(boxDebugShapeInstances);
		inRenderer.drawArraysInstanced(boxMesh.getVAO(), EPrimitiveMode::LINES, 0, boxMesh.getSize(), boxMesh.getInstanceCount());

		inFrameInfo.debugShapesTotal += boxMesh.getInstanceCount();
	}

	{
		ZoneScopedN("render sphere meshes");

		DebugShapeMesh& sphereMesh = debugShapeMeshes[(unsigned int)EDebugShapeMesh::SPHERE];
		sphereMesh.resetInstances();
		sphereMesh.addInstances(sphereDebugShapeInstances);
		sphereMesh.addInstances(entitySphereInstances.instances);
		inRenderer.drawArraysInstanced(sphereMesh.getVAO(), EPrimitiveMode::LINES, 0, sphereMesh.getSize(), sphereMesh.getInstanceCount());

		inFrameInfo.debugShapesTotal += sphereMesh.getInstanceCount();
	}
}

void DebugShapeSubRenderer::addPoint(const glm::vec3& inPosition, float inSize, const glm::vec3& inColor)
{
	DebugShapePrimitiveInstance pointInstance;
	pointInstance.color = inColor;
	pointInstance.position = inPosition;
	pointInstance.size = inSize;
	pointDebugShapeInstances.push_back(pointInstance);
}

void DebugShapeSubRenderer::addLine(const glm::vec3& inStart, const glm::vec3& inEnd, float inSize, const glm::vec3& inColor)
{
	DebugShapePrimitiveInstance lineStartInstance;
	lineStartInstance.color = inColor;
	lineStartInstance.position = inStart;
	lineStartInstance.size = inSize;
	lineDebugShapeInstances.push_back(lineStartInstance);

	DebugShapePrimitiveInstance lineEndInstance;
	lineEndInstance.color = inColor;
	lineEndInstance.position = inEnd;
	lineEndInstance.size = inSize;
	lineDebugShapeInstances.push_back(lineEndInstance);
}

void DebugShapeSubRenderer::addSphere(const glm::vec3& inPosition, const glm::vec3& inRotation, float inRadius, const glm::vec3& inColor)
{
	DebugShapeMeshInstance sphereInstance;
	sphereInstance.color = inColor;
	maths::computeTransformMatrix(inPosition, inRotation, glm::vec3(inRadius), sphereInstance.model);
	sphereDebugShapeInstances.push_back(sphereInstance);
}

void DebugShapeSubRenderer::addBox(const glm::vec3& inPosition, const glm::vec3& inRotation, const glm::vec3& inScale, const glm::vec3& inColor)
{
	DebugShapeMeshInstance boxInstance;
	boxInstance.color = inColor;
	maths::computeTransformMatrix(inPosition, inRotation, inScale, boxInstance.model);
	boxDebugShapeInstances.push_back(boxInstance);
}

void DebugShapeSubRenderer::addMovableSphere(Entity inEntity, const glm::vec3& inColor)
{
	DebugShapeMeshInstance sphereInstance;
	sphereInstance.color = inColor;

	entitySphereInstances.instances.push_back(sphereInstance);
	entitySphereInstances.entities.push_back(inEntity);
}

void DebugShapeSubRenderer::removeMovableSphere(Entity inEntity)
{
	for (size_t i = 0; i < entitySphereInstances.entities.size(); ++i)
	{
		if (entitySphereInstances.entities[i] != inEntity)
		{
			continue;
		}

		entitySphereInstances.entities[i] = *(entitySphereInstances.entities.end() - 1);
		entitySphereInstances.entities.pop_back();

		entitySphereInstances.instances[i] = *(entitySphereInstances.instances.end() - 1);
		entitySphereInstances.instances.pop_back();
		break;
	}
}