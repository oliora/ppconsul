//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/types.h"


namespace ppconsul { namespace agent {

    enum class Pool
    {
        Lan,
        Wan
    };

    struct Member
    {
        std::string name;
        std::string address;
        uint16_t port = 0;
        Properties tags;
        int status = 0;
        int protocolMin = 0;
        int protocolMax = 0;
        int protocolCur = 0;
        int delegateMin = 0;
        int delegateMax = 0;
        int delegateCur = 0;
    };

    struct Config {}; // TODO: use boost::ptree

    namespace params {
        PPCONSUL_PARAM(pool, Pool)
        PPCONSUL_PARAM(note, std::string)

        inline void printParameter(std::ostream& os, Pool v, KWARGS_KW_TAG(pool))
        {
            if (Pool::Wan == v)
                os << "wan=1";
        }
    }

    inline std::string serviceCheckId(const std::string& serviceId)
    {
        return "service:" + serviceId;
    }
    
    namespace impl {
        std::vector<Member> parseMembers(const std::string& json);
        std::pair<Config, Member> parseSelf(const std::string& json);
        std::unordered_map<std::string, CheckInfo> parseChecks(const std::string& json);
        std::unordered_map<std::string, Service> parseServices(const std::string& json);

        std::string checkRegistrationJson(const Check& check, const std::chrono::seconds& ttl);
        std::string checkRegistrationJson(const Check& check, const std::string& script, const std::chrono::seconds& interval);
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

        std::vector<Member> members(Pool pool = Pool::Lan) const
        {
            return impl::parseMembers(m_consul.get("/v1/agent/members", params::pool = pool));
        }
        
        std::pair<Config, Member> self() const
        {
            return impl::parseSelf(m_consul.get("/v1/agent/self"));
        }

        void join(const std::string& addr, Pool pool = Pool::Lan)
        {
            m_consul.get("/v1/agent/join/" + helpers::encodeUrl(addr), params::pool = pool);
        }

        void forceLeave(const std::string& node)
        {
            m_consul.get("/v1/agent/force-leave/" + helpers::encodeUrl(node));
        }

        std::unordered_map<std::string, CheckInfo> checks() const
        {
            return impl::parseChecks(m_consul.get("/v1/agent/checks"));
        }

        void registerCheck(const Check& check, const std::chrono::seconds& ttl)
        {
            m_consul.put("/v1/agent/check/register", impl::checkRegistrationJson(check, ttl));
        }
        
        void registerCheck(const Check& check, const std::string& script, const std::chrono::seconds& interval)
        {
            m_consul.put("/v1/agent/check/register", impl::checkRegistrationJson(check, script, interval));
        }
        
        void deregisterCheck(const std::string& id)
        {
            m_consul.get("/v1/agent/check/deregister/" + helpers::encodeUrl(id));
        }

        void pass(const std::string& checkId, const std::string& note = "")
        {
            updateCheck(checkId, CheckStatus::Passing, note);
        }

        void warn(const std::string& checkId, const std::string& note = "")
        {
            updateCheck(checkId, CheckStatus::Warning, note);
        }

        void fail(const std::string& checkId, const std::string& note = "")
        {
            updateCheck(checkId, CheckStatus::Critical, note);
        }

        // Same as pass(serviceCheckId(serviceId), note))
        void servicePass(const std::string& serviceId, const std::string& note = "")
        {
            updateServiceCheck(serviceId, CheckStatus::Passing, note);
        }

        // Same as warn(serviceCheckId(serviceId), note))
        void serviceWarn(const std::string& serviceId, const std::string& note = "")
        {
            updateServiceCheck(serviceId, CheckStatus::Warning, note);
        }

        // Same as fail(serviceCheckId(serviceId), note))
        void serviceFail(const std::string& serviceId, const std::string& note = "")
        {
            updateServiceCheck(serviceId, CheckStatus::Critical, note);
        }

        std::unordered_map<std::string, Service> services() const
        {
            return impl::parseServices(m_consul.get("/v1/agent/services"));
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
        void updateCheck(const std::string& checkId, CheckStatus newStatus, const std::string& note = "")
        {
            m_consul.get(impl::updateCheckUrl(newStatus) + helpers::encodeUrl(checkId), params::note = note);
        }

        // Same as `updateCheck(serviceCheckId(serviceId), newStatus, note)`
        void updateServiceCheck(const std::string& serviceId, CheckStatus newStatus, const std::string& note = "")
        {
            updateCheck(serviceCheckId(serviceId), newStatus, note);
        }

        Consul& m_consul;
    };


    // Implementation

    inline std::string impl::updateCheckUrl(CheckStatus newStatus)
    {
        switch (newStatus)
        {
        case CheckStatus::Passing:
            return "/v1/agent/check/pass/";
        case CheckStatus::Warning:
            return "/v1/agent/check/warn/";
        case CheckStatus::Critical:
            return "/v1/agent/check/fail/";
        default:
            throw std::logic_error("Wrong CheckStatus value");
        }
    }

}}
