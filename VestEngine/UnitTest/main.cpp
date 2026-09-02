#include <functional>
#include <iostream>

#include "Utils/DenseArray.h"

std::vector<std::function<void()>> tests;

#define TEST(name) \
class name \
{ \
public: \
std::string label = #name; \
name(); \
void test(); \
}; \
static name testInstance_##name##;\
name::name() { tests.push_back(std::bind(&name::test, testInstance_##name##)); } \
void name::test()

#define ENSURE(value) \
if (value) \
{ \
	std::cout << label << " : " << #value << " succeeded." << std::endl; \
} \
else \
{ \
	std::cerr << label << " : " << #value << " failed : " << std::endl; \
}

TEST(Test1_Capacity1)
{
	DenseArray<int> ints;
	ints.initialize(1);

	ENSURE(ints.getCapacity() == 1)
}

TEST(Test1_Capacity100)
{
	DenseArray<int> ints;
	ints.initialize(100);

	ENSURE(ints.getCapacity() == 100)
}

TEST(Test1_Capacity0)
{
	DenseArray<int> ints;
	ints.initialize(0);

	ENSURE(ints.getCapacity() == 0)
}

int main()
{
	for (auto test : tests)
	{
		test();
	}

	return 0;
}