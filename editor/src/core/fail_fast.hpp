#pragma once

#include <cstdio>
#include <exception>
#include <source_location>

namespace editor
{

[[noreturn]] inline void fail_fast(const char *message,
                                   const std::source_location location = std::source_location::current())
{
    std::fprintf(stderr, "editor invariant failed at %s:%u in %s: %s\\n", location.file_name(), location.line(),
                 location.function_name(), message);
    std::terminate();
}

} // namespace editor

#define EDITOR_CHECK(condition, message)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            ::editor::fail_fast((message), std::source_location::current());                                           \
        }                                                                                                              \
    } while (false)
