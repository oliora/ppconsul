//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
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

    using ppconsul::Service;

    namespace params {
        using ppconsul::params::consistency;
        using ppconsul::params::block_for;
        using ppconsul::params::dc;

        PPCONSUL_PARAM(passing, bool);

        namespace groups {
            PPCONSUL_PARAMS_GROUP(get, (consistency, dc, block_for));
        }
    }

    typedef std::tuple<Node, Service, std::vector<CheckInfo>> NodeServiceChecks;

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
        , m_defaultConsistency(kwargs::get(params::consistency, Consistency::Default, params...))
        , m_defaultDc(kwargs::get(params::dc, "", params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (params::consistency, params::dc))
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
        // - params::tag
        // - params::passing
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<NodeServiceChecks> service(WithHeaders, const std::string& serviceName, const Params&... params) const;

        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        // - params::tag
        // - params::passing
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
        KWARGS_CHECK_IN_LIST(Params, (params::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/health/node/" + helpers::encodeUrl(name),
                              params::consistency = m_defaultConsistency, params::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseCheckInfos(r.data()));
    }

    template<class... Params, class>
    Response<std::vector<CheckInfo>> Health::checks(WithHeaders, const std::string& serviceName, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (params::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/health/checks/" + helpers::encodeUrl(serviceName),
                              params::consistency = m_defaultConsistency, params::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseCheckInfos(r.data()));
    }

    template<class... Params, class>
    Response<NodeServiceChecks> Health::service(WithHeaders, const std::string& serviceName, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (params::groups::get, params::tag, params::passing));
        auto r = m_consul.get(withHeaders, "/v1/health/service/" + helpers::encodeUrl(serviceName),
                              params::consistency = m_defaultConsistency, params::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseService(r.data()));
    }

    template<class... Params, class>
    Response<std::vector<CheckInfo>> Health::state_impl(const std::string& state, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (params::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/health/state/" + state,
                              params::consistency = m_defaultConsistency, params::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseCheckInfos(r.data()));
    }

}}
