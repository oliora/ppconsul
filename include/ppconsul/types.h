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
#include <boost/variant.hpp>
#include <boost/optional.hpp>


namespace ppconsul {

    enum class Consistency
    {
        Default,
        Consistent,
        Stale
    };

    using duration = std::chrono::milliseconds; // TODO: use for BlockForValue as well

    typedef std::pair<std::chrono::seconds, uint64_t> BlockForValue;

    typedef std::unordered_set<std::string> Tags;

    typedef std::unordered_map<std::string, std::string> Properties;

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

    struct TtlCheckConfig
    {
        TtlCheckConfig() = default;
        explicit TtlCheckConfig(const duration& ttl)
        : ttl(ttl) {}

        duration ttl;
    };

    struct ScriptCheckConfig
    {
        ScriptCheckConfig() = default;
        ScriptCheckConfig(std::string script, const duration& interval)
        : script(std::move(script)), interval(interval) {}

        std::string script;
        duration interval;
    };

    struct HttpCheckConfig
    {
        HttpCheckConfig() = default;
        HttpCheckConfig(std::string url, const duration& interval, const duration& timeout = duration::zero())
        : url(std::move(url)), interval(interval), timeout(timeout) {}

        std::string url;
        duration interval;
        duration timeout;
    };

    struct TcpCheckConfig
    {
        TcpCheckConfig() = default;
        TcpCheckConfig(std::string address, const duration& interval, const duration& timeout = duration::zero())
        : address(std::move(address)), interval(interval), timeout(timeout) {}

        std::string address;
        duration interval;
        duration timeout;
    };

    struct DockerCheckConfig
    {
        DockerCheckConfig() = default;
        DockerCheckConfig(std::string containerId, std::string script, const duration& interval, std::string shell = "")
        : containerId(std::move(containerId)), script(std::move(script)), interval(interval), shell(std::move(shell)) {}

        std::string containerId;
        std::string script;
        duration interval;
        std::string shell;
    };

    using CheckConfig = boost::variant<TtlCheckConfig, ScriptCheckConfig, HttpCheckConfig, TcpCheckConfig, DockerCheckConfig>;

    struct CheckRegistrationData
    {
        CheckRegistrationData() = default;

        CheckRegistrationData(std::string name, CheckConfig config, std::string notes = "", std::string id = "")
        : id(std::move(id)), name(std::move(name)), config(std::move(config)), notes(std::move(notes))
        {}

        std::string id;
        std::string name;
        CheckConfig config;
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

    struct ServiceRegistrationData
    {
        struct Check
        {
            CheckConfig config;
            std::string notes;
        };

        ServiceRegistrationData() = default;

        ServiceRegistrationData(std::string name, uint16_t port = 0, Tags tags = Tags(), std::string id = "", std::string address = "")
        : id(std::move(id)), name(std::move(name)), address(std::move(address)), port(port), tags(std::move(tags))
        {}

        ServiceRegistrationData(std::string name, CheckConfig checkConfig, uint16_t port = 0, Tags tags = Tags(), std::string id = "", std::string address = "", std::string checkNotes = "")
        : id(std::move(id)), name(std::move(name)), address(std::move(address)), port(port), tags(std::move(tags))
        , check({std::move(checkConfig), std::move(checkNotes)})
        {}

        std::string id;
        std::string name;
        std::string address;
        uint16_t port = 0;
        Tags tags;

        boost::optional<Check> check;
    };

    struct ServiceInfo
    {
        std::string id;
        std::string name;
        std::string address;
        uint16_t port;
        Tags tags;
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
