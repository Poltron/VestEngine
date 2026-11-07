#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Core/ResourceHandle.h"
#include "ECS/Systems/FrustumCullingSystem.h"
#include "Render/Debug/DebugShapeSubRenderer.h"
#include "Render/FrameInfo.h"
#include "Render/GPUHandle.h"
#include "Render/Resources/MaterialProxy.h"
#include "Render/Resources/MeshProxy.h"
#include "Render/Resources/TextureProxy.h"

#define MAX_POINT_LIGHTS 3

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
	std::vector<MaterialProxy> materialProxies;

	DebugShapeSubRenderer debugShapeSubRenderer;
	FrustumCullingSystem frustumCullingSystem;
	FrameInfo frameInfo;

public:
	Camera& getActiveCamera();
	const Camera& getActiveCamera() const;
	void setActiveCamera(Camera* inCamera);

	ResourceHandle getLitMaterial() { return litMaterialHandle; }
	void setLitMaterial(ResourceHandle inResourceHandle) { litMaterialHandle = inResourceHandle; }
	ResourceHandle getUnlitMaterial() { return unlitMaterialHandle; }
	void setUnlitMaterial(ResourceHandle inResourceHandle) { unlitMaterialHandle = inResourceHandle; }
	ResourceHandle getSolidColorMaterial() { return solidColorMaterialHandle; }
	void setSolidColorMaterial(ResourceHandle inResourceHandle) { solidColorMaterialHandle = inResourceHandle; }

	MaterialProxy& getMaterialProxy(GPUProxyHandle inHandle) { return materialProxies[inHandle.id]; }
	const std::vector<MaterialProxy>& getMaterialProxies() const { return materialProxies; }
	MeshProxy& getMeshProxy(GPUProxyHandle inHandle) { return meshProxies[inHandle.id]; }
	const std::vector<MeshProxy>& getMeshProxies() const { return meshProxies; }
	TextureProxy& getTextureProxy(GPUProxyHandle inHandle) { return textureProxies[inHandle.id]; }
	const std::vector<TextureProxy>& getTextureProxies() const { return textureProxies; }

	DebugShapeSubRenderer& getDebugShapeSubRenderer() { return debugShapeSubRenderer; }
	const FrameInfo& getFrameInfo() const { return frameInfo; }

public:
	void clear();
	void render(Scene& inScene, double inDeltaTime);
	void swap();

	void updateDebugShapes(Scene& inScene, double inDeltaTime);

	GPUProxyHandle createMaterialProxy(void* inData, uint32_t inOffset);
	void destroyMaterialProxy(GPUProxyHandle inProxyHandle);
	GPUProxyHandle createMeshProxy(const Mesh* inMesh);
	void destroyMeshProxy(GPUProxyHandle inProxyHandle);
	GPUProxyHandle createTextureProxy(const Texture* inTexture);
	void destroyTextureProxy(GPUProxyHandle inProxyHandle);

private:
	void sortMainPass(Scene& inScene, std::vector<RenderedItem>& inOutRenderedItems);
	void recordMainPass(Scene& inScene, const std::vector<RenderedItem>& inRenderedItems);
	void recordDebugPass(Scene& inScene);

//
public:
	void loadDefaultMaterials();
	void updateLightParameters(Scene& inScene);
};

namespace render
{
	bool initialize();
	void shutdown();
	Renderer* getRenderer();
}