#pragma once

#include <functional>
#include <vector>

class TestFactoryBase;

class TestRegistrar
{
	std::vector<TestFactoryBase*> factories;
public:
	static TestRegistrar& get()
	{
		static TestRegistrar instance;
		return instance;
	}

	void registerFactory(TestFactoryBase* inFactory);
	void runTests();
	void unregisterFactories();
};