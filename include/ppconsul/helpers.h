#pragma once

#include <cstdio>

#if defined _WIN32 && ! defined PPCONSUL_SNPRINTF_DEFINED && ! defined snprintf
    #define PPCONSUL_SNPRINTF_DEFINED
    #define snprintf _snprintf
#endif


namespace ppconsul { namespace helpers {

    template<class... T>
    std::string format(const char *fmt, const T&...t)
    {
        const auto len = snprintf(nullptr, 0, fmt, t...);
        std::string r;
        r.resize(static_cast<size_t>(len) + 1);
        snprintf(&r.front(), len + 1, fmt, t...);  // Bad boy
        r.resize(static_cast<size_t>(len));

        return r;
    }

}}
