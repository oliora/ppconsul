//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/types.h"

namespace ppconsul { namespace coordinate {

    using Value = ppconsul::Coordinate;

    struct Node
    {
        std::string node;
        std::string segment;
        Value coord;
    };

    struct Datacenter
    {
        std::string datacenter;
        std::string areaId;
        std::vector<Node> coordinates;
    };

    namespace kw {
        using ppconsul::kw::consistency;
        using ppconsul::kw::block_for;
        using ppconsul::kw::dc;

        KWARGS_KEYWORD(segment, std::string)

        namespace groups {
            KWARGS_KEYWORDS_GROUP(get, (consistency, dc, block_for))
        }
    }

    namespace impl {
        std::vector<Datacenter> parseDatacenters(const std::string& json);
        std::vector<Node> parseNodes(const std::string& json);
        Node parseNode(const std::string& json);
    }

    class Coordinate
    {
    public:
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Coordinate(Consul& consul, const Params&... params)
        : m_consul(consul)
        , m_defaultConsistency(kwargs::get_opt(kw::consistency, Consistency::Default, params...))
        , m_defaultDc(kwargs::get_opt(kw::dc, std::string(), params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::consistency, kw::dc))
        }
        
        std::vector<Datacenter> datacenters() const
        {
            return impl::parseDatacenters(m_consul.get("/v1/coordinate/datacenters"));
        }

        // Result contains both headers and data.
        // Allowed parameters:
        // - segment
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<Node>> nodes(WithHeaders, const Params&... params) const;

        // Result contains data only.
        // Allowed parameters:
        // - segment
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<Node> nodes(const Params&... params) const
        {
            return std::move(nodes(withHeaders, params...).data());
        }

        // Returns empty vector if node does not exist
        // Result contains both headers and data.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<Node>> node(WithHeaders, const std::string& name, const Params&... params) const;

        // Returns empty vector if node does not exist
        // Result contains data only.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<Node> node(const std::string& name, const Params&... params) const
        {
            return std::move(node(withHeaders, name, params...).data());
        }

    private:
        Consul& m_consul;

        Consistency m_defaultConsistency;
        std::string m_defaultDc;
    };

    // Implementation

    template<class... Params, class>
    Response<std::vector<Node>> Coordinate::nodes(WithHeaders, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get, kw::segment));
        auto r = m_consul.get(withHeaders, "/v1/coordinate/nodes",
                              kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              params...);
        return makeResponse(r.headers(), impl::parseNodes(r.data()));
    }

    template<class... Params, class>
    Response<std::vector<Node>> Coordinate::node(WithHeaders, const std::string& name, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::get));
        http::Status s;
        auto r = m_consul.get(s, "/v1/coordinate/node/" + helpers::encodeUrl(name),
                              kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              params...);
        if (s.success())
            return makeResponse(r.headers(), std::move(impl::parseNodes(r.data())));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.data()));
    }
}}
