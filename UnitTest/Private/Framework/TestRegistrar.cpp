#include "Framework/TestRegistrar.h"

#include "tracy/Tracy.hpp"
#include "Framework/TestFactory.h"

void TestRegistrar::registerFactory(const std::string& inGroup, TestFactoryBase* inTestFactory)
{
	if (auto it = testGroups.find(inGroup); it != testGroups.end())
	{
		it->second.push_back(inTestFactory);
		return;
	}

	testGroups.insert({ inGroup, { inTestFactory } });
}

void TestRegistrar::runTests()
{
	ZoneScoped;

	for (auto& factoryGroup : testGroups)
	{
		std::cout << "> " << factoryGroup.first << std::endl;
		for (auto& factoryTest : factoryGroup.second)
		{
			factoryTest->runTest();
		}
		std::cout << std::endl << std::endl;;
	}
}

void TestRegistrar::unregisterFactories()
{
	for (auto& factoryGroup : testGroups)
	{
		for (auto& factoryTest : factoryGroup.second)
		{
			delete factoryTest;
		}
		factoryGroup.second.clear();
	}
	testGroups.clear();
}