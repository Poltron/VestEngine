#include "Framework/TestRegistrar.h"
#include "UnitTests/UnitTests_List.h"

int main()
{
	TestRegistrar::get().runTests();
	ensure(true == true);
	return 0;
}