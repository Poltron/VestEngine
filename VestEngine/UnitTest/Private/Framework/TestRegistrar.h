#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class TestFactoryBase;

class TestRegistrar
{
	std::unordered_map<std::string, std::vector<TestFactoryBase*>> testGroups;

public:
	static TestRegistrar& get()
	{
		static TestRegistrar instance;
		return instance;
	}

	void registerFactory(const std::string& inGroup, TestFactoryBase* inFactory);
	void runTests();
	void unregisterFactories();
};