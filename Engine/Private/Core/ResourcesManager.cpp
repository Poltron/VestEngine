#include "Core/ResourcesManager.h"

// todo: abstract
#include "Render/Renderer.h"

ResourcesManager::ResourcesManager()
{
	models.reserve(MAX_RESOURCES);
	materials.reserve(MAX_RESOURCES);
	materialInstances.reserve(MAX_MATINSTANCES);
	textures.reserve(MAX_RESOURCES);
}

ResourcesManager::~ResourcesManager()
{
	textures.clear();
	materials.clear();
	materialInstances.clear();
	models.clear();
}

ResourceHandle ResourcesManager::loadModel(std::string inPath)
{
	// note: what's the o() of iterating through an unordered_map ?
	for (const auto& loadedModel : modelsLookup)
	{
		if (loadedModel.second->getPath() == inPath)
		{
			//std::cout << "INFO : Found existing model for " << inPath << std::endl;
			return loadedModel.first;
		}
	}

	if (modelHandleCounter > MAX_RESOURCES)
	{
		std::cout << "ERROR : Max model resources reached" << std::endl;
		return ResourceHandle::invalid;
	}

	Model& model = models.emplace_back(std::move(inPath));

	ResourceHandle handle(modelHandleCounter);
	modelsLookup.insert({ handle, &model });
	modelHandleCounter++;

	std::cout << "INFO : Loaded model " << handle.handle << " ( " << model.getPath() << " )" << std::endl;

	return handle;
}

ResourceHandle ResourcesManager::createMaterial(EMaterialBlendMode inBlendMode, EMaterialType inType, const ShaderParameters& inShaderParameters)
{
	if (materialHandleCounter > MAX_RESOURCES)
	{
		std::cout << "ERROR : Max material resources reached" << std::endl;
		return ResourceHandle::invalid;
	}

	Material& material = materials.emplace_back(inBlendMode, inType, inShaderParameters);

	ResourceHandle handle(materialHandleCounter);
	materialsLookup.insert({ handle, &material });
	materialHandleCounter++;

	material.createDefaultMaterialInstance(handle.handle);

	std::cout << "INFO : Created material " << handle.handle << std::endl;

	return handle;
}

ResourceHandle ResourcesManager::createMaterialInstance(const Material* inMaterial, ResourceHandle inMaterialHandle)
{
	if (materialInstanceHandleCounter > MAX_MATINSTANCES)
	{
		std::cout << "ERROR : Max material resources reached" << std::endl;
		return ResourceHandle::invalid;
	}

	MaterialInstance& materialInstance = materialInstances.emplace_back(inMaterial, inMaterialHandle);

	ResourceHandle handle(materialInstanceHandleCounter);
	materialInstancesLookup.insert({ handle, &materialInstance });
	materialInstanceHandleCounter++;

	std::cout << "INFO : Created material " << handle.handle << std::endl;

	return handle;
}

ResourceHandle ResourcesManager::createModel(std::vector<Mesh>&& inMeshes, std::string inName)
{
	// note: what's the o() of iterating through an unordered_map ?
	for (const auto& loadedModel : modelsLookup)
	{
		if (loadedModel.second->getPath() == inName)
		{
			//std::cout << "INFO : Found existing model for " << inName << std::endl;
			return loadedModel.first;
		}
	}

	if (modelHandleCounter > MAX_RESOURCES)
	{
		std::cout << "ERROR : Max model resources reached" << std::endl;
		return ResourceHandle::invalid;
	}

	Model& model = models.emplace_back(std::move(inMeshes), std::move(inName));

	ResourceHandle handle(modelHandleCounter);
	modelsLookup.insert({ handle, &model });
	modelHandleCounter++;

	std::cout << "INFO : Created model " << handle.handle << " ( " << model.getPath() << " )" << std::endl;

	return handle;
}

ResourceHandle ResourcesManager::loadTexture(std::string inPath, std::string inType)
{
	// note: what's the o() of iterating through an unordered_map ?
	for (const auto& loadedTexture : texturesLookup)
	{
		if (loadedTexture.second->getPath() == inPath)
		{
			//std::cout << "INFO : Found existing texture for " << inPath << std::endl;
			return loadedTexture.first;
		}
	}

	if (textureHandleCounter > MAX_RESOURCES)
	{
		std::cout << "ERROR : Max texture resources reached" << std::endl;
		return ResourceHandle::invalid;
	}
	
	Texture& texture = textures.emplace_back(std::move(inPath), std::move(inType));

	ResourceHandle handle(textureHandleCounter);
	texturesLookup.insert({ handle, &texture });
	textureHandleCounter++;

	std::cout << "INFO : Loaded texture " << handle.handle << " ( " << texture.getPath() << " )" << std::endl;

	return handle;
}

ResourceHandle ResourcesManager::loadMaterial()
{
	if (materialHandleCounter > MAX_RESOURCES)
	{
		//std::cout << "ERROR : Max shader resources reached" << std::endl;
		return ResourceHandle::invalid;
	}

	//Material material = Material(std::move(inVertexPath), std::move(inFragmentPath));
	//shaders.push_back(std::move(shader));

	//ResourceHandle handle(shaderHandleCounter);
	//shadersLookup.insert({ handle, &shaders.back() });
	//shaderHandleCounter++;

	//std::cout << "INFO : Loaded shader " << handle.handle << " ( " << shaders.back().getVertexPath() << " + " << shaders.back().getFragmentPath() << " )" << std::endl;

	return ResourceHandle::invalid;
}

//
Model* ResourcesManager::getModel(ResourceHandle handle)
{
	assert(handle.isValid());
	return modelsLookup.at(handle);
}

const Model* ResourcesManager::getModel(ResourceHandle handle) const
{
	assert(handle.isValid());
	return modelsLookup.at(handle);
}

Texture* ResourcesManager::getTexture(ResourceHandle handle)
{
	assert(handle.isValid());
	return texturesLookup.at(handle);
}

const Texture* ResourcesManager::getTexture(ResourceHandle handle) const
{
	assert(handle.isValid());
	return texturesLookup.at(handle);
}

Material* ResourcesManager::getMaterial(ResourceHandle handle)
{
	assert(handle.isValid());
	return materialsLookup.at(handle);
}

const Material* ResourcesManager::getMaterial(ResourceHandle handle) const
{
	assert(handle.isValid());
	return materialsLookup.at(handle);
}

MaterialInstance* ResourcesManager::getMaterialInstance(ResourceHandle handle)
{
	assert(handle.isValid());
	return materialInstancesLookup.at(handle);
}

const MaterialInstance* ResourcesManager::getMaterialInstance(ResourceHandle handle) const
{
	assert(handle.isValid());
	return materialInstancesLookup.at(handle);
}