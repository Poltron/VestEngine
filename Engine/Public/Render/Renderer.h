#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Core/ResourceHandle.h"
#include "ECS/Systems/FrustumCullingSystem.h"
#include "Render/Debug/DebugShapeSubRenderer.h"
#include "Render/FrameInfo.h"
#include "Render/GPUHandle.h"
#include "Render/Resources/MeshProxy.h"
#include "Render/Resources/TextureProxy.h"

#define MAX_POINT_LIGHTS 3

// todo : need everything for the renderbackend to not need to go into ecs or resources
struct RenderedMesh
{
	GPUProxyHandle mesh = GPUProxyHandle::invalid;
	// textures
	uint64_t materialHash;
	glm::mat4 modelMat;
};

class Camera;
class Mesh;
class Model;
class Scene;
class Texture;

class Renderer
{
	Camera* activeCamera;

	ResourceHandle litMaterialHandle = ResourceHandle::invalid;
	ResourceHandle unlitMaterialHandle = ResourceHandle::invalid;
	ResourceHandle solidColorMaterialHandle = ResourceHandle::invalid;

	std::vector<MeshProxy> meshProxies;
	std::vector<TextureProxy> textureProxies;

protected:
	DebugShapeSubRenderer debugShapeSubRenderer;
	FrustumCullingSystem frustumCullingSystem;

	FrameInfo frameInfo;

public:
	void clear();
	void render(Scene& inScene, double inDeltaTime);
	void swap();

	void updateDebugShapes(Scene& inScene, double inDeltaTime);

	GPUProxyHandle createMeshProxy(const Mesh* inMesh);
	void destroyMeshProxy(GPUProxyHandle inProxyHandle);
	GPUProxyHandle createTextureProxy(const Texture* inTexture);
	void destroyTextureProxy(GPUProxyHandle inProxyHandle);

private:
	void renderMainPass(Scene& inScene, std::vector<RenderedMesh>& inRenderedMeshes);
	void renderDebugPass(Scene& inScene);

//
public:
	void drawModel(const Model& inModel);

//
public:
	void loadDefaultMaterials();
	void updateLightParameters(Scene& inScene);

	ResourceHandle getLitMaterial() { return litMaterialHandle; }
	void setLitMaterial(ResourceHandle inResourceHandle) { litMaterialHandle = inResourceHandle; }
	ResourceHandle getUnlitMaterial() { return unlitMaterialHandle; }
	void setUnlitMaterial(ResourceHandle inResourceHandle) { unlitMaterialHandle = inResourceHandle; }
	ResourceHandle getSolidColorMaterial() { return solidColorMaterialHandle; }
	void setSolidColorMaterial(ResourceHandle inResourceHandle) { solidColorMaterialHandle = inResourceHandle; }

	void setActiveCamera(Camera* inCamera);
	Camera& getActiveCamera();

	DebugShapeSubRenderer& getDebugShapeSubRenderer() { return debugShapeSubRenderer; }
	const FrameInfo& getFrameInfo() const { return frameInfo; }
};

namespace render
{
	bool initialize();
	void shutdown();
	Renderer* getRenderer();
}