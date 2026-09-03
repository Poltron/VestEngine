#include "Framework/TestRegistrar.h"
#include "UnitTests/UnitTests_List.h"

int main()
{
	TestRegistrar::get().runTests();
	return 0;
}