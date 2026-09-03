#pragma once

#include <stdexcept>
#include <functional>
#include <iostream>

#include "Framework/TestFactory.h"
#include "Framework/TestRegistrar.h"

class Test
{
public:
	virtual void run() {}
};

#define DECLARE_TEST(inGroup, inName) \
\
class inGroup##_##inName##_Test : public Test \
{ \
public: \
std::string group = #inGroup; \
std::string name = #inName; \
inGroup##_##inName##_Test() = default; \
virtual void run() override; \
}; \
\
namespace \
{ \
	struct AutoRegister_##inGroup##_##inName##  \
	{ \
		AutoRegister_##inGroup##_##inName##() \
		{ \
			TestRegistrar::get().registerFactory(#inGroup, new TestFactory<##inGroup##_##inName##_Test>()); \
		} \
	}; \
	static AutoRegister_##inGroup##_##inName AutoRegister_##inGroup##_##inName##_instance; \
} \
void inGroup##_##inName##_Test::run()


#define EXPECT(value) \
if (!(value)) \
{ \
	std::cerr << "\t" << name << " : " << #value << " failed." << std::endl; \
}

#define EXPECT_EXCEPTION(value) \
try { \
	value; \
	std::cerr << "\t" << name << " : " << #value << " failed." << std::endl; \
} \
catch (const std::runtime_error&) {}