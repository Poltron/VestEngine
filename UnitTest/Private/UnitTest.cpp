#include "Framework/TestRegistrar.h"
#include "UnitTests/UnitTests_List.h"
#include <iostream>
#include "tracy/Tracy.hpp"

namespace test
{
	void run()
	{
		ZoneScoped;

		std::cout << std::endl << "Unit Tests running..." << std::endl;
		ensure_behavior::bShouldThrow = true;
		TestRegistrar::get().runTests();
		ensure_behavior::bShouldThrow = false;
		TestRegistrar::get().unregisterFactories();
		std::cout << "Unit Tests ended." << std::endl << std::endl;
	}
}