//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <chrono>
#include <stdint.h>
#include <iostream>


namespace ppconsul {

    enum class Consistency
    {
        Default,
        Consistent,
        Stale
    };

    typedef std::pair<std::chrono::seconds, uint64_t> BlockForValue;

    typedef std::unordered_set<std::string> Tags;

    typedef std::unordered_map<std::string, std::string> Properties;

    struct Service
    {
        // Without this ctor provided VS 2013 crashes with internal error on code like
        // `agent.registerService({ "check_name" }, "script_name", std::chrono::seconds(interval))`
        Service(std::string name = "", uint16_t port = 0, Tags tags = Tags(), std::string id = "")
        : name(std::move(name)), port(port), tags(std::move(tags)), id(std::move(id))
        {}

        std::string name;
        uint16_t port = 0;
        Tags tags;
        std::string id;
    };

    struct Node
    {
        std::string node;
        std::string address;

        bool valid() const { return !node.empty() && !address.empty(); }
    };

    enum class CheckStatus
    {
        Unknown, // Not used since Consul 0.4.1
        Passing,
        Warning,
        Failed
    };

    struct Check
    {
        // Without this ctor provided VS 2013 crashes with internal error on code like
        // `agent.registerCheck({ "check_name" }, "script_name", std::chrono::seconds(interval))`
        Check(std::string name = "", std::string notes = "", std::string id = "")
            : name(std::move(name)), notes(std::move(notes)), id(std::move(id))
        {}

        std::string name;
        std::string notes;
        std::string id;
    };

    struct CheckInfo: Check
    {
        std::string node;
        CheckStatus status;
        std::string output;
        std::string serviceId;
        std::string serviceName;
    };

    struct WithHeaders {};
    const WithHeaders withHeaders = WithHeaders();

    inline std::ostream& operator<< (std::ostream& os, const CheckStatus& s)
    {
        switch (s)
        {
        case CheckStatus::Passing:
            os << "Passing";
            break;
        case CheckStatus::Warning:
            os << "Warning";
            break;
        case CheckStatus::Failed:
            os << "Failed";
            break;
        case CheckStatus::Unknown:
            os << "Unknown";
            break;
        default:
            os << "?";
            break;
        }

        return os;
    }

    inline bool operator!= (const Node& l, const Node& r)
    {
        return l.node != r.node || l.address != r.address;
    }

    inline bool operator== (const Node& l, const Node& r)
    {
        return !operator!=(l, r);
    }

}
