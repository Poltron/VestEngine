#pragma once

// Detect compiler and find the correct debugger break instruction
#if defined(_MSC_VER)
    #define DEBUG_BREAK() __debugbreak() // Microsoft Visual Studio
#elif defined(__clang__) || defined(__GNUC__)
    #define DEBUG_BREAK() __builtin_trap() // Clang / GCC
#else
    #define DEBUG_BREAK() ((void)0)
#endif

#ifndef NDEBUG
    #include <iostream>
#if defined(UNIT_TESTING)
    #include <stdexcept>
    #include <string> 

    #define ensure(condition) \
        if (!(condition)) { \
            std::string error = "Ensure triggered:"; \
            error.append(#condition); \
            error.append(" ("); \
            error.append(__FILE__); \
            error.append(":"); \
            error.append(std::to_string(__LINE__)); \
            error.append(")\n"); \
            throw std::runtime_error(error); \
        }
#else
    // Production build: fast crash/abort or zero-overhead assert
    #define ensure(condition) \
        if (!(condition)) { \
            std::cerr   << "Ensure triggered: " << #condition \
                        << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            DEBUG_BREAK(); \
        }
#endif
#else
    #define ensure(condition) ((void)0)
#endif