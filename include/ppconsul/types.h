//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <chrono>
#include <stdint.h>


namespace ppconsul {

    enum class Consistency
    {
        Default,
        Consistent,
        Stale
    };

    typedef std::pair<std::chrono::seconds, uint64_t> BlockForValue;

    typedef std::vector<std::string> Tags;

    typedef std::map<std::string, std::string> Properties;

    struct Service
    {
        // Without this ctor provided VS 2013 crashes with internal error on code like
        // `registerService({ "check_name" }, "script_name", std::chrono::seconds(interval))`
        Service(std::string name = "", uint16_t port = 0, Tags tags = Tags(), std::string id = "")
        : name(std::move(name)), port(port), tags(std::move(tags)), id(std::move(id))
        {}

        std::string name;
        uint16_t port = 0;
        Tags tags;
        std::string id;
    };

    struct WithHeaders {};
    const WithHeaders withHeaders = WithHeaders();

}
