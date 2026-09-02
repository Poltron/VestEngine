#pragma once

#include <functional>
#include <iostream>

#define DECLARE_TEST(name) \
class name \
{ \
public: \
std::string label = #name; \
name(); \
void test(); \
}; \
static name testInstance_##name##; \
name::name() { TestRegistrar::get().addTest(std::bind(&name::test, testInstance_##name##)); } \
void name::test()

#define TEST(value) \
if (value) \
{ \
	std::cout << label << " : " << #value << " succeeded." << std::endl; \
} \
else \
{ \
	std::cerr << label << " : " << #value << " failed." << std::endl; \
}