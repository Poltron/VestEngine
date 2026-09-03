#pragma once

#include <stdexcept>
#include <functional>
#include <iostream>

#include "Framework/TestFactory.h"
#include "Framework/TestRegistrar.h"

class Test
{
public:
	virtual void run() { std::cout << "Test::run()" << std::endl; }
};


#define DECLARE_TEST(name) \
\
class name : public Test \
{ \
public: \
std::string label = #name; \
name() = default; \
virtual void run() override; \
}; \
\
namespace \
{ \
	struct FAutoRegister##name##  \
	{ \
		FAutoRegister##name##() \
		{ \
			TestRegistrar::get().registerFactory(new TestFactory<name>()); \
		} \
	}; \
	static FAutoRegister##name autoRegister##name##; \
} \
void name::run()


#define EXPECT(value) \
if (value) \
{ \
	std::cout << label << " : " << #value << " succeeded." << std::endl; \
} \
else \
{ \
	std::cerr << label << " : " << #value << " failed." << std::endl; \
}

#define EXPECT_EXCEPTION(value) \
try { \
value; \
	std::cerr << label << " : " << #value << " failed." << std::endl; \
} \
catch (const std::runtime_error&) { \
	std::cout << label << " : " << #value << " succeeded." << std::endl; \
}