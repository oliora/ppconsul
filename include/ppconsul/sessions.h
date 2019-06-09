//  Copyright (c) 2019 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/types.h"
#include <chrono>

namespace ppconsul { namespace sessions {

    enum class InvalidationBehavior
    {
        Release,
        Delete,
    };

    namespace kw {
        using ppconsul::kw::consistency;
        using ppconsul::kw::block_for;
        using ppconsul::kw::dc;
        using ppconsul::kw::token;

        namespace groups {
            KWARGS_KEYWORDS_GROUP(get, (consistency, dc, block_for, token))
            KWARGS_KEYWORDS_GROUP(put, (dc, token))
        }
    }

    namespace impl {
        std::string createBodyJson(const std::string &node, std::chrono::seconds lockDelay,
                                   InvalidationBehavior behavior, std::chrono::seconds ttl);
        std::string parseCreateResponse(const std::string &resp);
    }

    class Sessions
    {
    public:
        // Allowed parameters:
        // - consistency - default consistency for all requests
        // - token - default token for all requests
        // - dc - default dc for all requests
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Sessions(Consul& consul, const Params&... params)
        : m_consul(consul)
        , m_defaultToken(kwargs::get_opt(kw::token, std::string(), params...))
        , m_defaultConsistency(kwargs::get_opt(kw::consistency, Consistency::Default, params...))
        , m_defaultDc(kwargs::get_opt(kw::dc, std::string(), params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::consistency, kw::token, kw::dc))
        }

        // Create a new session. Returns the ID of the created session.
        // Allowed parameters:
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string create(const std::string &node = "",
                           std::chrono::seconds lockDelay = std::chrono::seconds{15},
                           InvalidationBehavior behavior = InvalidationBehavior::Release,
                           std::chrono::seconds ttl = std::chrono::seconds{0},
                           const Params&... params)
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::groups::put))

            return impl::parseCreateResponse(m_consul.put(
                "/v1/session/create",
                impl::createBodyJson(node, lockDelay, behavior, ttl),
                kw::token = m_defaultToken, kw::dc = m_defaultDc, params...));
        }

        // Renew an existing session by its ID. Note that this operation only makes sense if the session has a TTL.
        // Allowed parameters:
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        void renew(const std::string &session, const Params&... params)
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::groups::put))

            m_consul.put(
                "/v1/session/renew/" + helpers::encodeUrl(session), "",
                kw::token = m_defaultToken, kw::dc = m_defaultDc, params...);
        }

        // Destroy a session by its ID. Returns true on success and false on failure.
        // Allowed parameters:
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        bool destroy(const std::string &session, const Params&... params)
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::groups::put))

            return helpers::parseJsonBool(m_consul.put(
                "/v1/session/destroy/" + helpers::encodeUrl(session), "",
                 kw::token = m_defaultToken, kw::dc = m_defaultDc, params...));
        }

    private:
        Consul& m_consul;

        std::string m_defaultToken;
        Consistency m_defaultConsistency;
        std::string m_defaultDc;
    };

}}
