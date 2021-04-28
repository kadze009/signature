#pragma once

#include <cstdio>
#include <string_view>

namespace debug {

constexpr char const* short_filename(char const* fname) noexcept
{
    std::string_view fname_sv {fname};
    size_t p = fname_sv.rfind('/');
    return (p != std::string_view::npos)
        ? fname_sv.substr(p + 1).data()
        : fname_sv.data();
}

} // namespace debug

#define LM_ARGS(fmt, ...)    "%s:%d " fmt "\n", debug::short_filename(__FILE__), __LINE__ __VA_OPT__(,) __VA_ARGS__
#define LM(fmt, ...)         fprintf(stderr, LM_ARGS(fmt, __VA_ARGS__))
#define LM_SV(sv)            (int)sv.size(), sv.data()
#define _V(ptr)              (void*)ptr
