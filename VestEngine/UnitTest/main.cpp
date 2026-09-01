#include <functional>
#include <iostream>

#include "Utils/DenseArray.h"

std::vector<std::function<void()>> tests;

#define TESTBEGIN(name) \
class name { \
public: \
std::string label = #name; \
name() { tests.push_back(std::bind(&name::test, this)); } \
void test() \

#define TESTEND(name) \
}; \
name name; \


#define TEST(value, failMessage) \
if (value) \
{ \
	std::cout << label << " succeeded." << std::endl; \
} \
else \
{ \
	std::cerr << label << " failed : " << failMessage << std::endl; \
}

int main()
{
	TESTBEGIN(Test1_Capacity1)
	{
		DenseArray<int> ints;
		ints.initialize(1);

		TEST(ints.getCapacity() == 1, "Capacity != 1")
	}
	TESTEND(Test1_Capacity1)

	TESTBEGIN(Test1_Capacity100)
	{
		DenseArray<int> ints;
		ints.initialize(100);

		TEST(ints.getCapacity() == 100, "Capacity != 100")
	}
	TESTEND(Test1_Capacity100)


	TESTBEGIN(Test1_Capacity0)
	{
		DenseArray<int> ints;
		ints.initialize(0);

		TEST(ints.getCapacity() == 0, "Capacity != 0")
	}
	TESTEND(Test1_Capacity0)

	for (auto test : tests)
	{
		test();
	}

	return 0;
}