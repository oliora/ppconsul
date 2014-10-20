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
        Unknown,
        Passing,
        Warning,
        Failed
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

    struct Config {}; // TODO: use boost::ptree

    inline std::string serviceCheckId(const std::string& serviceId)
    {
        return "service:" + serviceId;
    }
    
    namespace impl {
        std::vector<Member> parseMembers(const std::string& json);
        std::pair<Config, Member> parseSelf(const std::string& json);
        std::map<std::string, Check> parseChecks(const std::string& json);
        std::map<std::string, Service> parseServices(const std::string& json);

        std::string checkRegistrationJson(const std::string& name, const std::chrono::seconds& ttl, const std::string& notes, const std::string& id);
        std::string checkRegistrationJson(const std::string& name, const std::string& script, const std::chrono::seconds& interval, const std::string& notes, const std::string& id);
        std::string serviceRegistrationJson(const Service& service);
        std::string serviceRegistrationJson(const Service& service, const std::chrono::seconds& ttl);
        std::string serviceRegistrationJson(const Service& service, const std::string& script, const std::chrono::seconds& interval);

        std::string updateCheckUrl(CheckStatus newStatus);
    }

    class Agent
    {
    public:
        explicit Agent(Consul& consul)
        : m_consul(consul)
        {}

        std::vector<Member> members(bool wan = false) const
        {
            return impl::parseMembers(m_consul.get("/v1/agent/members", params::wan = wan));
        }
        
        std::pair<Config, Member> self() const
        {
            return impl::parseSelf(m_consul.get("/v1/agent/self"));
        }

        void join(const std::string& addr, bool wan = false)
        {
            m_consul.get("/v1/agent/join/" + helpers::encodeUrl(addr), params::wan = wan);
        }

        void forceLeave(const std::string& node)
        {
            m_consul.get("/v1/agent/force-leave/" + helpers::encodeUrl(node));
        }

        std::map<std::string, Check> checks() const
        {
            return impl::parseChecks(m_consul.get("/v1/agent/checks"));
        }

        void registerCheck(const std::string& name, const std::chrono::seconds& ttl, const std::string& notes = "", const std::string& id = "")
        {
            m_consul.put("/v1/agent/check/register", impl::checkRegistrationJson(name, ttl, notes, id));
        }
        
        void registerCheck(const std::string& name, const std::string& script, const std::chrono::seconds& interval, const std::string& notes = "", const std::string& id = "")
        {
            m_consul.put("/v1/agent/check/register", impl::checkRegistrationJson(name, script, interval, notes, id));
        }
        
        void deregisterCheck(const std::string& id)
        {
            m_consul.get("/v1/agent/check/deregister/" + helpers::encodeUrl(id));
        }

        void updateCheck(const std::string& id, CheckStatus newStatus, const std::string& note = "")
        {
            m_consul.get(impl::updateCheckUrl(newStatus) + helpers::encodeUrl(id), params::note = helpers::encodeUrl(note));
        }

        std::map<std::string, Service> services() const
        {
            impl::parseServices(m_consul.get("/v1/agent/services"));
        }

        void registerService(const Service& service)
        {
            m_consul.put("/v1/agent/service/register", impl::serviceRegistrationJson(service));
        }
        
        void registerService(const Service& service, const std::chrono::seconds& ttl)
        {
            m_consul.put("/v1/agent/service/register", impl::serviceRegistrationJson(service, ttl));
        }

        void registerService(const Service& service, const std::string& script, const std::chrono::seconds& interval)
        {
            m_consul.put("/v1/agent/service/register", impl::serviceRegistrationJson(service, script, interval));
        }
        
        void deregisterService(const std::string& id)
        {
            m_consul.get("/v1/agent/service/deregister/" + helpers::encodeUrl(id));
        }

    private:
        Consul& m_consul;
    };


    // Implementation

    namespace impl {
        inline std::string updateCheckUrl(CheckStatus newStatus)
        {
            switch (newStatus)
            {
            case CheckStatus::Passing:
                return "/v1/agent/check/pass/";
            case CheckStatus::Warning:
                return "/v1/agent/check/warn/";
            case CheckStatus::Failed:
                return "/v1/agent/check/fail/";
            default:
                throw std::logic_error("Wrong CheckStatus value");
            }
        }
    }

}}
