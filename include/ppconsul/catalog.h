//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/types.h"


namespace ppconsul { namespace catalog {

    typedef std::pair<Node, std::unordered_map<std::string, ServiceInfo>> NodeServices;

    typedef std::pair<Node, ServiceInfo> NodeService;

    namespace kw {
        using ppconsul::kw::consistency;
        using ppconsul::kw::block_for;
        using ppconsul::kw::dc;
        using ppconsul::kw::tag;

        namespace groups {
            KWARGS_KEYWORDS_GROUP(get, (consistency, dc, block_for))
        }
    }

    namespace impl {
        std::vector<std::string> parseDatacenters(const std::string& json);
        std::vector<Node> parseNodes(const std::string& json);
        NodeServices parseNode(const std::string& json);
        std::unordered_map<std::string, Tags> parseServices(const std::string& json);
        std::vector<NodeService> parseService(const std::string& json);
    }

    class Catalog
    {
    public:
        // Allowed parameters:
        // - consistency - default consistency for requests that support it
        // - dc - default dc for requests that support it
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Catalog(Consul& consul, const Params&... params)
        : m_consul(consul)
        , m_defaultConsistency(kwargs::get_opt(kw::consistency, Consistency::Default, params...))
        , m_defaultDc(kwargs::get_opt(kw::dc, std::string(), params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::consistency, kw::dc))
        }

        std::vector<std::string> datacenters() const
        {
            return impl::parseDatacenters(m_consul.get("/v1/catalog/datacenters"));
        }

        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<Node>> nodes(WithHeaders, const Params&... params) const;

        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<Node> nodes(const Params&... params) const
        {
            return std::move(nodes(withHeaders, params...).data());
        }

        // If node does not exist, function returns invalid node with empty serivces map
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<NodeServices> node(WithHeaders, const std::string& name, const Params&... params) const;

        // If node does not exist, function returns invalid node with empty serivces map
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        NodeServices node(const std::string& name, const Params&... params) const
        {
            return std::move(node(withHeaders, name, params...).data());
        }

        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::unordered_map<std::string, Tags>> services(WithHeaders, const Params&... params) const;

        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::unordered_map<std::string, Tags> services(const Params&... params) const
        {
            return std::move(services(withHeaders, params...).data());
        }

        // Result contains both headers and data.
        // Allowed parameters:
        // - tag
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<NodeService>> service(WithHeaders, const std::string& name, const Params&... params) const;

        // Result contains data only.
        // Allowed parameters:
        // - tag
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<NodeService> service(const std::string& name, const Params&... params) const
        {
            return std::move(service(withHeaders, name, params...).data());
        }

    private:
        Consul& m_consul;

        Consistency m_defaultConsistency;
        std::string m_defaultDc;
    };


    // Implementation

    template<class... Params, class>
    Response<std::vector<Node>> Catalog::nodes(WithHeaders, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/catalog/nodes",
            kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
            params...);
        return makeResponse(r.headers(), impl::parseNodes(r.data()));
    }

    template<class... Params, class>
    Response<NodeServices> Catalog::node(WithHeaders, const std::string& name, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/catalog/node/" + helpers::encodeUrl(name),
            kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
            params...);
        return makeResponse(r.headers(), impl::parseNode(r.data()));
    }

    template<class... Params, class>
    Response<std::unordered_map<std::string, Tags>> Catalog::services(WithHeaders, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get));
        auto r = m_consul.get(withHeaders, "/v1/catalog/services",
            kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
            params...);
        return makeResponse(r.headers(), impl::parseServices(r.data()));
    }

    template<class... Params, class>
    Response<std::vector<NodeService>> Catalog::service(WithHeaders, const std::string& name, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get, kw::tag));
        auto r = m_consul.get(withHeaders, "/v1/catalog/service/" + helpers::encodeUrl(name),
            kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
            params...);
        return makeResponse(r.headers(), impl::parseService(r.data()));
    }

}}
