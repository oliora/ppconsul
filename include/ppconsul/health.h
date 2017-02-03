//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/types.h"
#include <tuple>


namespace ppconsul { namespace health {

    namespace kw {
        using ppconsul::kw::consistency;
        using ppconsul::kw::block_for;
        using ppconsul::kw::dc;
        using ppconsul::kw::tag;

        PPCONSUL_KEYWORD(passing, bool);

        namespace groups {
            KWARGS_KEYWORDS_GROUP(get, (consistency, dc, block_for));
        }
    }

    typedef std::tuple<Node, ServiceInfo, std::vector<CheckInfo>> NodeServiceChecks;

    namespace impl {
        std::vector<CheckInfo> parseCheckInfos(const std::string& json);
        NodeServiceChecks parseService(const std::string& json);
        std::string to_string(CheckStatus state);
    }
    

    class Health
    {
    public:
        // Allowed parameters:
        // - consistency - default consistency for requests that support it
        // - dc - default dc for requests that support it
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Health(Consul& consul, const Params&... params)
        : m_consul(consul)
        , m_defaultConsistency(kwargs::get_opt(kw::consistency, Consistency::Default, params...))
        , m_defaultDc(kwargs::get_opt(kw::dc, std::string(), params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::consistency, kw::dc))
        }

        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<CheckInfo>> node(WithHeaders, const std::string& name, const Params&... params) const;

        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<CheckInfo> node(const std::string& name, const Params&... params) const
        {
            return std::move(node(withHeaders, name, params...).data());
        }

        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<CheckInfo>> checks(WithHeaders, const std::string& serviceName, const Params&... params) const;

        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<CheckInfo> checks(const std::string& serviceName, const Params&... params) const
        {
            return std::move(checks(withHeaders, serviceName, params...).data());
        }

        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        // - kw::tag
        // - kw::passing
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<NodeServiceChecks> service(WithHeaders, const std::string& serviceName, const Params&... params) const;

        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        // - kw::tag
        // - kw::passing
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        NodeServiceChecks service(const std::string& serviceName, const Params&... params) const
        {
            return std::move(service(withHeaders, serviceName, params...).data());
        }

        // Returns the checks in specified state
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<CheckInfo>> state(WithHeaders, CheckStatus state, const Params&... params) const
        {
            return state_impl(impl::to_string(state), params...);
        }

        // Returns the checks in specified state
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<CheckInfo> state(CheckStatus state, const Params&... params) const
        {
            return std::move(state(withHeaders, state, params...).data());
        }

        // Returns all the checks
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<CheckInfo>> state(WithHeaders, const Params&... params) const
        {
            return state_impl("any", params...);
        }

        // Returns all the checks
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<CheckInfo> state(const Params&... params) const
        {
            return std::move(state(withHeaders, params...).data());
        }

    private:
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<CheckInfo>> state_impl(const std::string& state, const Params&... params) const;

        Consul& m_consul;

        Consistency m_defaultConsistency;
        std::string m_defaultDc;
    };


    // Implementation

    inline std::string impl::to_string(CheckStatus state)
    {
        switch (state)
        {
            case CheckStatus::Unknown:
                return "unknown";
            case CheckStatus::Passing:
                return "passing";
            case CheckStatus::Warning:
                return "warning";
            case CheckStatus::Critical:
                return "critical";
            default:
                throw std::logic_error("Wrong CheckStatus value");
        }
    }

    template<class... Params, class>
    Response<std::vector<CheckInfo>> Health::node(WithHeaders, const std::string& name, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/health/node/" + helpers::encodeUrl(name),
                              kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseCheckInfos(r.data()));
    }

    template<class... Params, class>
    Response<std::vector<CheckInfo>> Health::checks(WithHeaders, const std::string& serviceName, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/health/checks/" + helpers::encodeUrl(serviceName),
                              kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseCheckInfos(r.data()));
    }

    template<class... Params, class>
    Response<NodeServiceChecks> Health::service(WithHeaders, const std::string& serviceName, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get, kw::tag, kw::passing));
        auto r = m_consul.get(withHeaders, "/v1/health/service/" + helpers::encodeUrl(serviceName),
                              kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseService(r.data()));
    }

    template<class... Params, class>
    Response<std::vector<CheckInfo>> Health::state_impl(const std::string& state, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/health/state/" + state,
                              kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseCheckInfos(r.data()));
    }

}}
