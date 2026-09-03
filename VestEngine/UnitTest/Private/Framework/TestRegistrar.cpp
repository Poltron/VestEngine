#include "Framework/TestRegistrar.h"

#include "Framework/TestFactory.h"

#include <stdexcept>

void TestRegistrar::registerFactory(TestFactoryBase* inTestFactory)
{
	factories.push_back(inTestFactory);
}

void TestRegistrar::runTests()
{
	for (auto& factory : factories)
	{
		factory->runTest();
	}
}

void TestRegistrar::unregisterFactories()
{
	for (auto& factory : factories)
	{
		delete factory;
	}
}