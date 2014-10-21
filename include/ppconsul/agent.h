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

    enum class CheckStatus
    {
        Unknown, // Not used since Consul 0.4.1
        Passing,
        Warning,
        Failed
    };

    typedef std::vector<std::string> Tags;
    typedef std::map<std::string, std::string> Properties;

    enum class Pool
    {
        Lan,
        Wan
    };


    struct Check
    {
        // Otherwise MSVC crashes with internal error
        // on code like `registerCheck({ "check_name" }, "script_name", std::chrono::seconds(interval))`
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

    struct Service
    {
        // Otherwise MSVC crashes with internal error
        // on code like `registerService({ "check_name" }, "script_name", std::chrono::seconds(interval))`
        Service(std::string name = "", uint16_t port = 0, Tags tags = Tags(), std::string id = "")
        : name(std::move(name)), port(port), tags(std::move(tags)), id(std::move(id))
        {}

        std::string name;
        uint16_t port = 0;
        Tags tags;
        std::string id;
    };

    struct ServiceInfo: Service
    {};

    struct Member
    {
        std::string name;
        std::string addr;
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

    std::ostream& operator<< (std::ostream& os, const CheckStatus& s);

    inline std::string serviceCheckId(const std::string& serviceId)
    {
        return "service:" + serviceId;
    }
    
    namespace impl {
        std::vector<Member> parseMembers(const std::string& json);
        std::pair<Config, Member> parseSelf(const std::string& json);
        std::map<std::string, CheckInfo> parseChecks(const std::string& json);
        std::map<std::string, ServiceInfo> parseServices(const std::string& json);

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

        std::map<std::string, CheckInfo> checks() const
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

        void updateCheck(const std::string& checkId, CheckStatus newStatus, const std::string& note = "")
        {
            m_consul.get(impl::updateCheckUrl(newStatus) + helpers::encodeUrl(checkId), params::note = helpers::encodeUrl(note));
        }

        void updateServiceCheck(const std::string& serviceId, CheckStatus newStatus, const std::string& note = "")
        {
            updateCheck(serviceCheckId(serviceId), newStatus, note);
        }

        std::map<std::string, ServiceInfo> services() const
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
        Consul& m_consul;
    };


    // Implementation

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
