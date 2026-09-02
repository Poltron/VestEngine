#pragma once

#include <iostream>
#include <functional>
#include <vector>

class TestRegistrar
{
	std::vector<std::function<void()>> tests;

public:
	static TestRegistrar& get()
	{
		static TestRegistrar instance;
		return instance;
	}

	void addTest(const std::function<void()>& inTest)
	{
		tests.push_back(inTest);
	}

	void runTests()
	{
		for (auto test : tests)
		{
			try
			{
				test();
			}
			catch (const std::runtime_error& e)
			{
				std::cerr << "Test failed" << std::endl;
				continue;
			}
		}
	}
};
