#pragma once

// Detect compiler and find the correct debugger break instruction
#if defined(_MSC_VER)
    #define DEBUG_BREAK() __debugbreak() // Microsoft Visual Studio
#elif defined(__clang__) || defined(__GNUC__)
    #define DEBUG_BREAK() __builtin_trap() // Clang / GCC
#else
    #define DEBUG_BREAK() ((void)0)
#endif

// note: need this for unit_tests to run at editor start, kind of don't like it but eh
namespace ensure_behavior
{
    extern bool bShouldThrow;
}

#ifndef RELEASE
    #include <iostream>

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
            if (ensure_behavior::bShouldThrow) \
            { \
                throw std::runtime_error(error); \
            } \
            else \
            { \
                std::cerr << "Ensure triggered: " << #condition \
                    << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
                DEBUG_BREAK(); \
            } \
        }
#else
    #define ensure(condition) ((void)0)
#endif