//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/json.h"
#include <vector>
#include <map>
#include <chrono>
#include <stdint.h>


namespace ppconsul { namespace agent {

    namespace detail {
        PPCONSUL_PARAM(wan, bool)
        PPCONSUL_PARAM(note, std::string)
    }

    enum class CheckStatus
    {
        Pass,
        Warn,
        Fail
    };

    typedef std::vector<std::string> Tags;
    typedef std::map<std::string, std::string> Properties;

    struct Check
    {
        std::string m_id;
        std::string m_node;
        std::string m_name;
        CheckStatus m_status;
        std::string m_notes;
        std::string m_output;
        std::string m_serviceId;
        std::string m_serviceName;
    };

    struct Service
    {
        Service() {}

        explicit Service(std::string name, uint16_t port = 0, Tags tags = Tags(), std::string id = "")
        : m_id(std::move(id)), m_name(std::move(name)), m_port(port), m_tags(std::move(tags))
        {}

        std::string m_id;
        std::string m_name;
        uint16_t m_port = 0;
        Tags m_tags;
    };

    struct Member
    {
        std::string m_name;
        std::string m_addr;
        uint16_t m_port;
        Properties m_tags;
        int m_status;
        int m_protocolMin, m_protocolMax, m_protocolCur;
        int m_delegateMin, m_delegateMax, m_delegateCur;
    };

    struct Config {}; // use boost::ptree or expose json11 object

    inline std::string serviceCheckId(const std::string& serviceId)
    {
        return "service:" + serviceId;
    }
    
    class Agent
    {
    public:
        explicit Agent(Consul& consul)
        : m_consul(consul)
        {}

        std::vector<Member> members(bool wan = false) const;
        std::pair<Config, Member> self() const;

        void join(const std::string& addr, bool wan = false);
        void forceLeave(const std::string& node);

        std::map<std::string, Check> checks() const;

        void registerCheck(const std::string& name, const std::chrono::seconds& ttl, const std::string& notes = "", const std::string& id = "");
        void registerCheck(const std::string& name, const std::string& script, const std::chrono::seconds& interval, const std::string& notes = "", const std::string& id = "");
        void deregisterCheck(const std::string& id);

        void updateCheck(const std::string& id, CheckStatus newStatus);
        void updateCheck(const std::string& id, CheckStatus newStatus, const std::string& notes);

        std::map<std::string, Service> services() const;
        
        void registerService(const Service& service);
        void registerService(const Service& service, const std::chrono::seconds& ttl);
        void registerService(const Service& service, const std::string& script, const std::chrono::seconds& interval);
        void deregisterService(const std::string& id);

    private:
        Consul& m_consul;
    };

}}
