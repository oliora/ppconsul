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

    struct ServiceRegistrationData
    {
        // Without this ctor provided VS 2013 crashes with internal error on code like
        // `agent.registerService({ "check_name" }, "script_name", std::chrono::seconds(interval))`
        ServiceRegistrationData(std::string name = "", uint16_t port = 0, Tags tags = Tags(), std::string id = "", std::string address = "")
        : id(std::move(id)), name(std::move(name)), address(std::move(address)), port(port), tags(std::move(tags))
        {}

        std::string id;
        std::string name;
        std::string address;
        uint16_t port = 0;
        Tags tags;
    };

    struct ServiceInfo
    {
        std::string id;
        std::string name;
        std::string address;
        uint16_t port;
        Tags tags;
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
        Critical
    };

    struct CheckRegistrationData
    {
        // Without this ctor provided VS 2013 crashes with internal error on code like
        // `agent.registerCheck({ "check_name" }, "script_name", std::chrono::seconds(interval))`
        CheckRegistrationData(std::string name = "", std::string notes = "", std::string id = "")
        : id(std::move(id)), name(std::move(name)), notes(std::move(notes))
        {}

        std::string id;
        std::string name;
        std::string notes;
    };

    struct CheckInfo
    {
        std::string id;
        std::string name;
        std::string notes;
        std::string serviceId;
        std::string serviceName;
        std::string node;
        CheckStatus status;
        std::string output;
    };

    struct WithHeaders {};
    const WithHeaders withHeaders = WithHeaders();

    inline std::ostream& operator<< (std::ostream& os, const CheckStatus& s)
    {
        switch (s)
        {
        case CheckStatus::Passing:
            os << "passing";
            break;
        case CheckStatus::Warning:
            os << "warning";
            break;
        case CheckStatus::Critical:
            os << "critical";
            break;
        case CheckStatus::Unknown:
            os << "unknown";
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
