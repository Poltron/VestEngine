#include "Managers/ResourcesManager.h"

ResourceHandle ResourcesManager::loadModel(const char* inPath)
{
	// note: what's the o() of iterating through an unordered_map ?
	for (const auto& loadedModel : modelsLookup)
	{
		if (loadedModel.second->getPath() == inPath)
		{
			std::cout << "INFO : Found existing model for " << inPath << std::endl;
			return loadedModel.first;
		}
	}

	if (modelHandleCounter > MAX_RESOURCES)
	{
		std::cout << "ERROR : Max model resources reached" << std::endl;
		return ResourceHandle();
	}

	Model model = Model(inPath, *this);
	models.push_back(std::move(model));

	ResourceHandle handle(modelHandleCounter);
	modelsLookup.insert({ handle, &models.back() });
	modelHandleCounter++;

	std::cout << "INFO : Loaded model " << handle.handle << " ( " << inPath << " )" << std::endl;

	return handle;
}

ResourceHandle ResourcesManager::createModel(std::vector<Mesh>&& inMeshes, const char* inName)
{
	// note: what's the o() of iterating through an unordered_map ?
	for (const auto& loadedModel : modelsLookup)
	{
		if (loadedModel.second->getPath() == inName)
		{
			std::cout << "INFO : Found existing model for " << inName << std::endl;
			return loadedModel.first;
		}
	}

	if (modelHandleCounter > MAX_RESOURCES)
	{
		std::cout << "ERROR : Max model resources reached" << std::endl;
		return ResourceHandle();
	}

	Model model(std::move(inMeshes), inName);
	models.push_back(std::move(model));
	
	ResourceHandle handle(modelHandleCounter);
	modelsLookup.insert({ handle, &models.back() });
	modelHandleCounter++;

	std::cout << "INFO : Created model " << handle.handle << " ( " << inName << " )" << std::endl;

	return handle;
}

ResourceHandle ResourcesManager::loadTexture(const char* inPath, const char* inType)
{
	// note: what's the o() of iterating through an unordered_map ?
	for (const auto& loadedTexture : texturesLookup)
	{
		if (loadedTexture.second->getPath() == inPath)
		{
			std::cout << "INFO : Found existing texture for " << inPath << std::endl;
			return loadedTexture.first;
		}
	}

	if (textureHandleCounter > MAX_RESOURCES)
	{
		std::cout << "ERROR : Max texture resources reached" << std::endl;
		return ResourceHandle();
	}

	Texture texture(inPath, inType);
	textures.push_back(std::move(texture));

	ResourceHandle handle(textureHandleCounter);
	texturesLookup.insert({ handle, &textures.back() });
	textureHandleCounter++;

	std::cout << "INFO : Loaded texture " << handle.handle << " ( " << inPath << " )" << std::endl;

	return handle;
}

ResourceHandle ResourcesManager::loadShader(const char* inVertexPath, const char* inFragmentPath)
{
	if (shaderHandleCounter > MAX_RESOURCES)
	{
		std::cout << "ERROR : Max shader resources reached" << std::endl;
		return ResourceHandle();
	}


	Shader shader = Shader(inVertexPath, inFragmentPath);
	shaders.push_back(std::move(shader));

	ResourceHandle handle(shaderHandleCounter);
	shadersLookup.insert({ handle, &shaders.back() });
	shaderHandleCounter++;

	std::cout << "INFO : Loaded shader " << handle.handle << " ( " << inVertexPath << " + " << inFragmentPath << " )" << std::endl;

	return handle;
}


Model* ResourcesManager::getModel(ResourceHandle handle)
{
	assert(handle.IsValid());
	return modelsLookup.at(handle);
}

const Model* ResourcesManager::getModel(ResourceHandle handle) const
{
	assert(handle.IsValid());
	return modelsLookup.at(handle);
}

Texture* ResourcesManager::getTexture(ResourceHandle handle)
{
	assert(handle.IsValid());
	return texturesLookup.at(handle);
}

const Texture* ResourcesManager::getTexture(ResourceHandle handle) const
{
	assert(handle.IsValid());
	return texturesLookup.at(handle);
}

Shader* ResourcesManager::getShader(ResourceHandle handle)
{
	assert(handle.IsValid());
	return shadersLookup.at(handle);
}

const Shader* ResourcesManager::getShader(ResourceHandle handle) const
{
	assert(handle.IsValid());
	return shadersLookup.at(handle);
}