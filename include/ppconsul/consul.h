//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include "ppconsul/error.h"
#include "ppconsul/types.h"
#include "ppconsul/parameters.h"
#include "ppconsul/http_status.h"
#include "ppconsul/response.h"
#include "ppconsul/http_client.h"
#include <chrono>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <limits>
#include <iostream>


namespace ppconsul {

    namespace kw {
        PPCONSUL_KEYWORD(dc, std::string)
        PPCONSUL_KEYWORD(token, std::string)
        PPCONSUL_KEYWORD(tag, std::string)
        KWARGS_KEYWORD(consistency, Consistency)
        KWARGS_KEYWORD(block_for, BlockForValue)

        inline void printParameter(std::ostream& os, const Consistency& v, KWARGS_KW_TAG(consistency))
        {
            switch (v)
            {
                case Consistency::Stale: os << "stale"; break;
                case Consistency::Consistent: os << "consistent"; break;
                default: break;
            }
        }

        inline void printParameter(std::ostream& os, const BlockForValue& v, KWARGS_KW_TAG(block_for))
        {
            os << "wait=" << v.first.count() << "ms&index=" << v.second;
        }
    }

    const char Default_Server_Address[] = "127.0.0.1:8500";

    class Consul
    {
    public:
        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all client requests (can be overloaded in every specific request)
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(const std::string& addr, const Params&... params)
        : Consul(kwargs::get_opt(kw::token, std::string(), params...),
                kwargs::get_opt(kw::dc, std::string(), params...),
                addr)
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::dc, kw::token))
        }

        // Same as Consul(Default_Server_Address, ...)
        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all requests
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(const Params&... params)
            : Consul(Default_Server_Address, params...)
        {}

        Consul(Consul &&op) PPCONSUL_NOEXCEPT
        : m_client(std::move(op.m_client))
        , m_dataCenter(std::move(op.m_dataCenter))
        , m_defaultToken(std::move(op.m_defaultToken))
        {}

        Consul& operator= (Consul &&op) PPCONSUL_NOEXCEPT
        {
            m_client = std::move(op.m_client);
            m_dataCenter = std::move(op.m_dataCenter);
            m_defaultToken = std::move(op.m_defaultToken);

            return *this;
        }

        Consul(const Consul &op) = delete;
        Consul& operator= (const Consul &op) = delete;

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::string> get(WithHeaders, const std::string& path, const Params&... params);

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string get(const std::string& path, const Params&... params)
        {
            return std::move(get(withHeaders, path, params...).data());
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::string> get(http::Status& status, const std::string& path, const Params&... params)
        {
            return get_impl(status, path, makeQuery(params...));
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(http::Status& status, const std::string& path, const std::string& data, const Params&... params)
        {
            return put_impl(status, path, makeQuery(params...), data);
        }

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(const std::string& path, const std::string& data, const Params&... params);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(http::Status& status, const std::string& path, const Params&... params)
        {
            return del_impl(status, path, makeQuery(params...));
        }

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(const std::string& path, const Params&... params);

    private:
        Consul(const std::string& defaultToken, const std::string& dataCenter, const std::string& addr);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string makeQuery(const Params&... params) const
        {
            return parameters::makeQuery(kw::dc = m_dataCenter, kw::token = m_defaultToken, params...);
        }

        // TODO: make impl funcs inline
        Response<std::string> get_impl(http::Status& status, const std::string& paty, const std::string& query);
        std::string put_impl(http::Status& status, const std::string& path, const std::string& query, const std::string& data);
        std::string del_impl(http::Status& status, const std::string& path, const std::string& query);

        std::unique_ptr<http::impl::Client> m_client;
        std::string m_dataCenter;

        std::string m_defaultToken;
    };

    // Implementation

    inline void throwStatusError(http::Status status, std::string data)
    {
        if (NotFoundError::Code == status.code())
        {
            throw NotFoundError{};
            //throw NotFoundError(std::move(status), std::move(data));
        }
        else
        {
            throw BadStatus(std::move(status), std::move(data));
        }
    }

    template<class... Params, class>
    inline Response<std::string> Consul::get(WithHeaders, const std::string& path, const Params&... params)
    {
        http::Status s;
        auto r = get(s, path, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r.data()));
        return r;
    }

    template<class... Params, class>
    inline std::string Consul::put(const std::string& path, const std::string& data, const Params&... params)
    {
        http::Status s;
        auto r = put(s, path, data, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

    template<class... Params, class>
    inline std::string Consul::del(const std::string& path, const Params&... params)
    {
        http::Status s;
        auto r = del(s, path, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

    inline Response<std::string> Consul::get_impl(http::Status& status, const std::string& path, const std::string& query)
    {
        Response<std::string> r;
        std::tie(status, r.headers(), r.data()) = m_client->get(path, query);
        return r;
    }

    inline std::string Consul::put_impl(http::Status& status, const std::string& path, const std::string& query, const std::string& data)
    {
        std::string r;
        std::tie(status, r) = m_client->put(path, query, data);
        return r;
    }

    inline std::string Consul::del_impl(http::Status& status, const std::string& path, const std::string& query)
    {
        std::string r;
        std::tie(status, r) = m_client->del(path, query);
        return r;
    }

}
