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
        using ppconsul::kw::dc;
        using ppconsul::kw::token;

        KWARGS_KEYWORD(name, std::string)
        KWARGS_KEYWORD(node, std::string)
        KWARGS_KEYWORD(lock_delay, std::chrono::seconds)
        KWARGS_KEYWORD(behavior, InvalidationBehavior)
        KWARGS_KEYWORD(ttl, std::chrono::seconds)

        namespace groups {
            KWARGS_KEYWORDS_GROUP(put, (dc, token))
        }
    }

    namespace impl {
        std::string createBodyJson(const std::string &name,
                                   const std::string &node,
                                   std::chrono::seconds lockDelay,
                                   InvalidationBehavior behavior,
                                   std::chrono::seconds ttl);

        std::string parseCreateResponse(const std::string &resp);
    }

    class Sessions
    {
    public:
        // Allowed parameters:
        // - token - default token for all requests
        // - dc - default dc for all requests
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Sessions(Consul& consul, const Params&... params)
        : m_consul(consul)
        , m_defaultToken(kwargs::get_opt(kw::token, std::string(), params...))
        , m_defaultDc(kwargs::get_opt(kw::dc, std::string(), params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::token, kw::dc))
        }

        // Create a new session. Returns the ID of the created session.
        // Allowed parameters:
        // - name - a human-readable name for the session
        // - node - the name of the node (current agent by default)
        // - lock_delay - the duration for the lock delay (15s by default)
        // - behavior - the behavior to take when the session is invalidated (release by default)
        // - ttl - session TTL
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string create(const Params&... params) const;

        // Renew an existing session by its ID. Note that this operation only makes sense if the session has a TTL.
        // Allowed parameters:
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        void renew(const std::string &session, const Params&... params) const
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
        bool destroy(const std::string &session, const Params&... params) const
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::groups::put))

            return helpers::parseJsonBool(m_consul.put(
                "/v1/session/destroy/" + helpers::encodeUrl(session), "",
                 kw::token = m_defaultToken, kw::dc = m_defaultDc, params...));
        }

    private:
        Consul& m_consul;

        std::string m_defaultToken;
        std::string m_defaultDc;
    };


    // Implementation

    template<class... Params, class>
    std::string Sessions::create(const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kw::groups::put, kw::name, kw::node, kw::lock_delay, kw::behavior, kw::ttl))

        auto resp = m_consul.put(
            "/v1/session/create",
            impl::createBodyJson(
                kwargs::get_opt(kw::name, std::string{}, params...),
                kwargs::get_opt(kw::node, std::string{}, params...),
                kwargs::get_opt(kw::lock_delay, std::chrono::seconds{-1}, params...),
                kwargs::get_opt(kw::behavior, InvalidationBehavior::Release, params...),
                kwargs::get_opt(kw::ttl, std::chrono::seconds{-1}, params...)
            ),
            kw::token = kwargs::get_opt(kw::token, m_defaultToken, params...),  // TODO: implement keywords filtering
            kw::dc = kwargs::get_opt(kw::dc, m_defaultDc, params...)            //
        );

        return impl::parseCreateResponse(resp);
    }
}}
