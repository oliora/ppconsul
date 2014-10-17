//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include <vector>
#include <map>
#include <chrono>
#include <stdint.h>


namespace ppconsul { namespace agent {

    namespace params {
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
        std::string id;
        std::string node;
        std::string name;
        CheckStatus status;
        std::string notes;
        std::string output;
        std::string serviceId;
        std::string serviceName;
    };

    struct Service
    {
        std::string name;
        uint16_t port = 0;
        Tags tags;
        std::string id;
    };

    struct Member
    {
        std::string name;
        std::string addr;
        uint16_t port;
        Properties tags;
        int status;
        int protocolMin;
        int protocolMax;
        int protocolCur;
        int delegateMin;
        int delegateMax;
        int delegateCur;
    };

    struct Config {}; // use boost::ptree

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

        void updateCheck(const std::string& id, CheckStatus newStatus, const std::string& note = "");

        std::map<std::string, Service> services() const;
        
        void registerService(const Service& service);
        void registerService(const Service& service, const std::chrono::seconds& ttl);
        void registerService(const Service& service, const std::string& script, const std::chrono::seconds& interval);
        void deregisterService(const std::string& id);

    private:
        Consul& m_consul;
    };

    // Implementation

    namespace impl {
        void parseJson(std::vector<Member>& out, const std::string& json);
        void parseJson(std::pair<Config, Member>& out, const std::string& json);
        void parseJson(std::map<std::string, Check>& out, const std::string& json);
        void parseJson(std::map<std::string, Service>& out, const std::string& json);

        std::string checkRegistrationJson(const std::string& name, const std::chrono::seconds& ttl, const std::string& notes, const std::string& id);
        std::string checkRegistrationJson(const std::string& name, const std::string& script, const std::chrono::seconds& interval, const std::string& notes, const std::string& id);
        std::string serviceRegistrationJson(const Service& service);
        std::string serviceRegistrationJson(const Service& service, const std::chrono::seconds& ttl);
        std::string serviceRegistrationJson(const Service& service, const std::string& script, const std::chrono::seconds& interval);

        inline std::string updateCheckUrl(CheckStatus newStatus)
        {
            switch (newStatus)
            {
            case CheckStatus::Pass:
                return "/v1/agent/check/pass/";
            case CheckStatus::Warn:
                return "/v1/agent/check/warn/";
            case CheckStatus::Fail:
                return "/v1/agent/check/fail/";
            default:
                throw std::logic_error("Wrong CheckStatus value");
            }
        }
    }

    inline std::vector<Member> Agent::members(bool wan) const
    {
        std::vector<Member> res;
        impl::parseJson(res, m_consul.get("/v1/agent/members", params::wan = wan));
        return res;
    }

    inline std::pair<Config, Member> Agent::self() const
    {
        std::pair<Config, Member> res;
        impl::parseJson(res, m_consul.get("/v1/agent/self"));
        return res;
    }

    inline void Agent::join(const std::string& addr, bool wan)
    {
        m_consul.get("/v1/agent/join/" + helpers::encodeUrl(addr), params::wan = wan);
    }

    inline void Agent::forceLeave(const std::string& node)
    {
        m_consul.get("/v1/agent/force-leave/" + helpers::encodeUrl(node));
    }

    inline std::map<std::string, Check> Agent::checks() const
    {
        std::map<std::string, Check> res;
        impl::parseJson(res, m_consul.get("/v1/agent/checks"));
        return res;
    }

    inline void Agent::registerCheck(const std::string& name, const std::chrono::seconds& ttl, const std::string& notes, const std::string& id)
    {
        m_consul.put("/v1/agent/check/register", impl::checkRegistrationJson(name, ttl, notes, id));
    }

    inline void Agent::registerCheck(const std::string& name, const std::string& script, const std::chrono::seconds& interval, const std::string& notes, const std::string& id)
    {
        m_consul.put("/v1/agent/check/register", impl::checkRegistrationJson(name, script, interval, notes, id));
    }

    inline void Agent::deregisterCheck(const std::string& id)
    {
        m_consul.get("/v1/agent/check/deregister/" + helpers::encodeUrl(id));
    }

    inline void Agent::updateCheck(const std::string& id, CheckStatus newStatus, const std::string& note)
    {
        m_consul.get(impl::updateCheckUrl(newStatus) + helpers::encodeUrl(id), params::note = helpers::encodeUrl(note));
    }

    inline std::map<std::string, Service> Agent::services() const
    {
        std::map<std::string, Service> res;
        impl::parseJson(res, m_consul.get("/v1/agent/services"));
        return res;
    }

    inline void Agent::registerService(const Service& service)
    {
        m_consul.put("/v1/agent/service/register", impl::serviceRegistrationJson(service));
    }

    inline void Agent::registerService(const Service& service, const std::chrono::seconds& ttl)
    {
        m_consul.put("/v1/agent/service/register", impl::serviceRegistrationJson(service, ttl));
    }

    inline void Agent::registerService(const Service& service, const std::string& script, const std::chrono::seconds& interval)
    {
        m_consul.put("/v1/agent/service/register", impl::serviceRegistrationJson(service, script, interval));
    }

    inline void Agent::deregisterService(const std::string& id)
    {
        m_consul.get("/v1/agent/service/deregister/" + helpers::encodeUrl(id));
    }
}}
