//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
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

    struct TtlCheck
    {
        TtlCheck() = default;

        explicit TtlCheck(const duration& ttl)
        : ttl(ttl) {}

        duration ttl;
    };

    struct ScriptCheck
    {
        ScriptCheck() = default;

        ScriptCheck(std::string script, const duration& interval)
        : script(std::move(script)), interval(interval) {}

        std::string script;
        duration interval;
    };

    struct HttpCheck
    {
        HttpCheck() = default;

        HttpCheck(std::string url, const duration& interval, const duration& timeout = duration::zero())
        : url(std::move(url)), interval(interval), timeout(timeout) {}

        std::string url;
        duration interval;
        duration timeout;
    };

    namespace impl {
        inline std::string makeTcpAddress(const char *host, uint16_t port)
        {
            return helpers::format("%s:%u", host, static_cast<unsigned>(port));
        }
    }

    struct TcpCheck
    {
        TcpCheck() = default;

        TcpCheck(std::string address, const duration& interval, const duration& timeout = duration::zero())
        : address(std::move(address)), interval(interval), timeout(timeout) {}

        TcpCheck(const std::string& host, uint16_t port, const duration& interval, const duration& timeout = duration::zero())
        : TcpCheck(impl::makeTcpAddress(host.c_str(), port), interval, timeout) {}

        std::string address;
        duration interval;
        duration timeout;
    };

    struct DockerCheck
    {
        DockerCheck() = default;

        DockerCheck(std::string containerId, std::string script, const duration& interval, std::string shell = "")
        : containerId(std::move(containerId)), script(std::move(script)), interval(interval), shell(std::move(shell)) {}

        std::string containerId;
        std::string script;
        duration interval;
        std::string shell;
    };

    using CheckParams = boost::variant<TtlCheck, ScriptCheck, HttpCheck, TcpCheck, DockerCheck>;

    namespace kw {
        KWARGS_KEYWORD(name, std::string)
        KWARGS_KEYWORD(notes, std::string)
        KWARGS_KEYWORD(id, std::string)
        KWARGS_KEYWORD(check, CheckParams)
        KWARGS_KEYWORD(port, uint16_t)
        KWARGS_KEYWORD(address, std::string)
        KWARGS_KEYWORD(tags, Tags)

        PPCONSUL_KEYWORD(pool, Pool)
        PPCONSUL_KEYWORD(note, std::string)

        inline void printParameter(std::ostream& os, Pool v, KWARGS_KW_TAG(pool))
        {
            if (Pool::Wan == v)
                os << "wan=1";
        }
    }


    namespace impl {
        struct CheckRegistrationData;
        struct ServiceRegistrationData;

        std::vector<Member> parseMembers(const std::string& json);
        std::pair<Config, Member> parseSelf(const std::string& json);
        std::unordered_map<std::string, CheckInfo> parseChecks(const std::string& json);
        std::unordered_map<std::string, ServiceInfo> parseServices(const std::string& json);

        std::string checkRegistrationJson(const CheckRegistrationData& check);
        std::string serviceRegistrationJson(const ServiceRegistrationData& service);

        std::string updateCheckUrl(CheckStatus newStatus);
    }


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

        std::vector<Member> members(Pool pool = Pool::Lan) const
        {
            return impl::parseMembers(m_consul.get("/v1/agent/members", kw::pool = pool));
        }
        
        std::pair<Config, Member> self() const
        {
            return impl::parseSelf(m_consul.get("/v1/agent/self"));
        }

        void join(const std::string& addr, Pool pool = Pool::Lan)
        {
            m_consul.get("/v1/agent/join/" + helpers::encodeUrl(addr), kw::pool = pool);
        }

        void forceLeave(const std::string& node)
        {
            m_consul.get("/v1/agent/force-leave/" + helpers::encodeUrl(node));
        }

        std::unordered_map<std::string, CheckInfo> checks() const
        {
            return impl::parseChecks(m_consul.get("/v1/agent/checks"));
        }

        // Allowed parameters:
        // - name - the check's name (required)
        // - check - parameters of the check (required)
        // - id - the check's id, set to the check's name by default
        // - notes - the check's notes, empty by default
        template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
        void registerCheck(Keywords&&... params);

        // Allowed parameters:
        // - id - the check's id, set to the check's name by default
        // - notes - the check's notes, empty by default
        template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
        void registerCheck(std::string name, CheckParams check, Keywords&&... params)
        {
            registerCheck(
                kw::name = std::move(name),
                kw::check = std::move(check),
                std::forward<Keywords>(params)...
            );
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

        std::unordered_map<std::string, ServiceInfo> services() const
        {
            return impl::parseServices(m_consul.get("/v1/agent/services"));
        }

        // Allowed parameters:
        // - name - the service's name (required)
        // - id - the service's id, set to the service's name by default
        // - tags - the service's tags, empty by default
        // - address - the service's address, empty by default (Note that Consul docs say one is set to the agent's address but this is not true)
        // - port - the service's port, set to 0 by default
        // - check - parameters for a check associated with the service, no check is associated if omitted
        // - notes - notes for a check associated with the service, empty by default
        template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
        void registerService(Keywords&&... params);

        // Allowed parameters:
        // - id - the service's id, set to the service's name by default
        // - tags - the service's tags, empty by default
        // - address - the service's address, empty by default (Note that Consul docs say one is set to the agent's address but this is not true)
        // - port - the service's port, set to 0 by default
        // - check - parameters for a check associated with the service, no check is associated if omitted
        // - notes - notes for a check associated with the service, empty by default
        template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
        void registerService(std::string name, Keywords&&... params)
        {
            registerService(
                kw::name = std::move(name),
                std::forward<Keywords>(params)...
            );
        }

        // Allowed parameters:
        // - id - the service's id, set to the service's name by default
        // - tags - the service's tags, empty by default
        // - address - the service's address, empty by default (Note that Consul docs say one is set to the agent's address but this is not true)
        // - port - the service's port, set to 0 by default
        // - notes - notes for a check associated with the service, empty by default
        template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
        void registerService(std::string name, CheckParams check, Keywords&&... params)
        {
            registerService(
                kw::name = std::move(name),
                kw::check = std::move(check),
                std::forward<Keywords>(params)...
            );
        }

        void deregisterService(const std::string& id)
        {
            m_consul.get("/v1/agent/service/deregister/" + helpers::encodeUrl(id));
        }

    private:
        void updateCheck(const std::string& checkId, CheckStatus newStatus, const std::string& note = "")
        {
            m_consul.get(impl::updateCheckUrl(newStatus) + helpers::encodeUrl(checkId), kw::note = note);
        }

        // Same as `updateCheck(serviceCheckId(serviceId), newStatus, note)`
        void updateServiceCheck(const std::string& serviceId, CheckStatus newStatus, const std::string& note = "")
        {
            updateCheck(serviceCheckId(serviceId), newStatus, note);
        }

        Consul& m_consul;
    };


    // Implementation

    namespace impl {
        struct CheckRegistrationData
        {
            template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
            CheckRegistrationData(Keywords&&... params)
            : id(kwargs::get_opt(kw::id, std::string(), std::forward<Keywords>(params)...))
            , name(kwargs::get(kw::name, std::forward<Keywords>(params)...))
            , params(kwargs::get(kw::check, std::forward<Keywords>(params)...))
            , notes(kwargs::get_opt(kw::notes, std::string(), std::forward<Keywords>(params)...))
            {
                KWARGS_CHECK_IN_LIST(Keywords, (
                    kw::id, kw::name, kw::check, kw::notes))
            }

            std::string id;
            std::string name;
            CheckParams params;
            std::string notes;
        };

        struct ServiceRegistrationData
        {
            struct Check
            {
                CheckParams params;
                std::string notes;
            };

            template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
            ServiceRegistrationData(std::true_type, Keywords&&... params)
            : id(kwargs::get_opt(kw::id, std::string(), std::forward<Keywords>(params)...))
            , name(kwargs::get(kw::name, std::forward<Keywords>(params)...))
            , address(kwargs::get_opt(kw::address, std::string(), std::forward<Keywords>(params)...))
            , port(kwargs::get_opt(kw::port, 0, std::forward<Keywords>(params)...))
            , tags(kwargs::get_opt(kw::tags, Tags(), std::forward<Keywords>(params)...))
            , check({
                kwargs::get(kw::check, std::forward<Keywords>(params)...),
                kwargs::get_opt(kw::notes, std::string(), std::forward<Keywords>(params)...)})
            {
                KWARGS_CHECK_IN_LIST(Keywords, (
                    kw::id, kw::name, kw::address, kw::port, kw::tags,
                        kw::check, kw::notes))
            }

            template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
            ServiceRegistrationData(std::false_type, Keywords&&... params)
            : id(kwargs::get_opt(kw::id, std::string(), std::forward<Keywords>(params)...))
            , name(kwargs::get(kw::name, std::forward<Keywords>(params)...))
            , address(kwargs::get_opt(kw::address, std::string(), std::forward<Keywords>(params)...))
            , port(kwargs::get_opt(kw::port, 0, std::forward<Keywords>(params)...))
            , tags(kwargs::get_opt(kw::tags, Tags(), std::forward<Keywords>(params)...))
            {
                KWARGS_CHECK_IN_LIST(Keywords, (
                    kw::id, kw::name, kw::address, kw::port, kw::tags))
            }

            template<class... Keywords, class = kwargs::enable_if_kwargs_t<Keywords...>>
            ServiceRegistrationData(Keywords&&... params)
            : ServiceRegistrationData(
                kwargs::has_keyword<decltype(kw::check), Keywords...>{},
                std::forward<Keywords>(params)...)
            {}

            std::string id;
            std::string name;
            std::string address;
            uint16_t port = 0;
            Tags tags;

            boost::optional<Check> check;
        };

        inline std::string updateCheckUrl(CheckStatus newStatus)
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
    }

    template<class... Keywords, class>
    inline void Agent::registerCheck(Keywords&&... params)
    {
        m_consul.put("/v1/agent/check/register",
                     impl::checkRegistrationJson({std::forward<Keywords>(params)...}));
    }

    template<class... Keywords, class>
    inline void Agent::registerService(Keywords&&... params)
    {
        m_consul.put("/v1/agent/service/register",
                     impl::serviceRegistrationJson({std::forward<Keywords>(params)...}));
    }

}}
