#pragma once

// copied from the web

#include <iostream>

// 1. Detect the compiler and find the correct debugger break instruction
#if defined(_MSC_VER)
    // Microsoft Visual Studio
#define DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    // Clang / GCC
#define DEBUG_BREAK() __builtin_trap() // Note: See GCC warning below
#else
#define DEBUG_BREAK() ((void)0)
#endif

// 2. Define the soft assert macro
#ifndef NDEBUG
#define SOFT_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cerr << "Assertion Failed: " << message \
                          << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
                DEBUG_BREAK(); \
            } \
        } while (false)
#else
    // Completely vanishes in Release builds
#define SOFT_ASSERT(condition, message) ((void)0)
#endif