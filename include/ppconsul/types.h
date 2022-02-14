//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <chrono>
#include <stdint.h>
#include <iostream>
#include <boost/variant.hpp>
#include <boost/optional.hpp>


namespace ppconsul {

    enum class Consistency
    {
        Default,
        Consistent,
        Stale
    };

    using duration = std::chrono::milliseconds;

    using BlockForValue = std::pair<std::chrono::milliseconds, uint64_t>;

    using Tags = std::set<std::string>;

    using Metadata = std::map<std::string, std::string>;

    using StringList = std::vector<std::string>;

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

    struct ServiceInfo
    {
        std::string id;
        std::string name;
        std::string address;
        uint16_t port;
        Tags tags;
        Metadata meta;
    };

    struct Coordinate
    {
        double adjustment;
        double error;
        double height;
        std::vector<double> vec;
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
