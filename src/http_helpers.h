//  Copyright (c) 2014-2016 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdlib>
#include <cstring>


namespace ppconsul { namespace http { namespace impl {
    const std::string Index_Header_Name("X-Consul-Index");
    const std::string KnownLeader_Header_Name("X-Consul-Knownleader");
    const std::string LastContact_Headers_Name("X-Consul-Lastcontact");

    inline uint64_t uint64_headerValue(const char *v)
    {
        return std::strtoull(v, nullptr, 10);
    }

    inline bool bool_headerValue(const char *v)
    {
        return 0 == strcmp(v, "true");
    }
}}}
