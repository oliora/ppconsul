//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/types.h"


namespace ppconsul { namespace catalog {

    using ppconsul::Service;

    struct Node
    {
        std::string name;
        std::string address;
    };

    typedef std::pair<Node, std::map<std::string, Service>> NodeServices;

    typedef std::pair<Service, Node> ServiceAndNode;

    namespace params {
        using ppconsul::params::consistency;
        using ppconsul::params::block_for;

        PPCONSUL_PARAM(tag, std::string)

        namespace groups {
            PPCONSUL_PARAMS_GROUP(get, (consistency, block_for))
        }
    }

    namespace impl {
        std::vector<std::string> parseDatacenters(const std::string& json);
        std::vector<Node> parseNodes(const std::string& json);
        NodeServices parseNode(const std::string& json);
        std::map<std::string, Tags> parseServices(const std::string& json);
        std::vector<ServiceAndNode> parseService(const std::string& json);
    }

    class Catalog
    {
    public:
        // Allowed parameters:
        // - consistency - default consistency for requests that support it
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Catalog(Consul& consul, const Params&... params)
        : m_consul(consul)
        , m_defaultConsistency(kwargs::get(params::consistency, Consistency::Default, params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (params::consistency))
        }

        std::vector<std::string> datacenters() const
        {
            return impl::parseDatacenters(m_consul.get("/v1/catalog/datacenters"));
        }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<Node> nodes(const Params&... params) const
        {
            KWARGS_CHECK_IN_GROUP(Params, params::groups::get)
            return impl::parseNodes(m_consul.get("/v1/catalog/nodes", params::consistency = m_defaultConsistency, params...));
        }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        NodeServices node(const std::string& name, const Params&... params) const
        {
            KWARGS_CHECK_IN_GROUP(Params, params::groups::get)
            return impl::parseNode(m_consul.get("/v1/catalog/node/" + helpers::encodeUrl(name),
                params::consistency = m_defaultConsistency,
                params...));
        }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::map<std::string, Tags> services(const Params&... params) const
        {
            KWARGS_CHECK_IN_GROUP(Params, params::groups::get)
            return impl::parseServices(m_consul.get("/v1/catalog/services", params::consistency = m_defaultConsistency, params...));
        }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<ServiceAndNode> service(const std::string& name, const Params&... params) const
        {
            KWARGS_CHECK_IN_GROUP(Params, params::groups::get)
            return impl::parseService(m_consul.get("/v1/catalog/service/" + helpers::encodeUrl(name),
                params::consistency = m_defaultConsistency,
                params...));
        }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<ServiceAndNode> service(const std::string& name, const std::string& tag, const Params&... params) const
        {
            KWARGS_CHECK_IN_GROUP(Params, params::groups::get)
            return impl::parseService(m_consul.get("/v1/catalog/service/" + helpers::encodeUrl(name),
                params::consistency = m_defaultConsistency,
                params::tag = helpers::encodeUrl(tag),
                params...));
        }

    private:
        Consul& m_consul;

        Consistency m_defaultConsistency;
    };
}}
