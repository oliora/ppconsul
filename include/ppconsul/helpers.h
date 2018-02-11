//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include "ppconsul/error.h"
#include <cstdio>
#include <string>


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

    inline bool urlHasScheme(const std::string& url)
    {
        return url.find("://") != std::string::npos;
    }

    // Ensures that URL has scheme. Adds `defaultScheme` if no scheme is present yet.
    inline std::string ensureScheme(const std::string& url, const std::string& defaultScheme = "http")
    {
        if (urlHasScheme(url))
            return url;

        return defaultScheme + "://" + url;
    }

    // Note that implementation used is liberal to the input: it allows characters outside the alphabete and
    // incorrect padding.
    std::string decodeBase64(const std::string& s);

    // Encode string to be safely used as *part* of URL.
    std::string encodeUrl(const std::string&s);

    bool parseJsonBool(const std::string& s);
}}
