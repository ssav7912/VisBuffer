#pragma once
#include <format>
#include <print>
//adapted from Microsoft MiniEngine. 

#if defined _DEBUG

#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)
#define ASSERT( isFalse, ... ) \
        if (!(bool)(isFalse)) { \
            std::print("\nAssertion failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            std::print("\'" #isFalse "\' is false"); \
            std::println(STRINGIFY_BUILTIN(__VA_ARGS__)); \
            __debugbreak(); \
        }

#define ASSERT_SUCCEEDED( hr, ... ) \
        if (FAILED(hr)) { \
            std::print("\nHRESULT failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            std::print(std::format("hr = {:08X}", hr)); \
            std::println(STRINGIFY_BUILTIN(__VA_ARGS__)); \
            __debugbreak(); \
        }


#define WARN_ONCE_IF( isTrue, ... ) \
    { \
        static bool s_TriggeredWarning = false; \
        if ((bool)(isTrue) && !s_TriggeredWarning) { \
            s_TriggeredWarning = true; \
            std::print("\nWarning issued in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            std::print("\'" #isTrue "\' is true"); \
            std::println((__VA_ARGS__)); \
        } \
    }

#define WARN_ONCE_IF_NOT( isTrue, ... ) WARN_ONCE_IF(!(isTrue), __VA_ARGS__)

#define ERROR( ... ) \
        std::print("\nError reported in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
        std::print((__VA_ARGS__)); \
        std::print("\n");

#define DEBUGPRINT( msg, ... ) \
    std::printf( msg "\n", ##__VA_ARGS__ );
#else

#define ASSERT( isTrue, ... ) (void)(isTrue)
#define WARN_ONCE_IF( isTrue, ... ) (void)(isTrue)
#define WARN_ONCE_IF_NOT( isTrue, ... ) (void)(isTrue)
#define ERROR( msg, ... )
#define DEBUGPRINT( msg, ... ) do {} while(0)
#define ASSERT_SUCCEEDED( hr, ... ) (void)(hr)
#endif

#define BreakIfFailed( hr ) if (FAILED(hr)) __debugbreak()