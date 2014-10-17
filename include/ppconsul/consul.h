//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include "ppconsul/error.h"
#include "ppconsul/parameters.h"
#include "ppconsul/http_status.h"
#include "ppconsul/response.h"
#include <chrono>
#include <string>
#include <tuple>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <limits>


namespace ppconsul {

    namespace impl {
        // Forward declaraion to use with PIMPL
        class HttpClient;
    }

    enum class Consistency
    {
        Default,
        Consistent,
        Stale
    };

    typedef std::pair<std::chrono::seconds, uint64_t> BlockForValue;

    namespace params {
        PPCONSUL_PARAM(dc, std::string)
        PPCONSUL_PARAM(token, std::string)
        PPCONSUL_PARAM_NO_NAME(consistency, Consistency)
        PPCONSUL_PARAM_NO_NAME(block_for, BlockForValue)

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
            os << "wait=" << v.first.count() << "s&index=" << v.second;
        }
    }


    struct WithHeaders {};
    const WithHeaders withHeaders{};

    const char Default_Server_Address[] = "localhost:8500";

    class Consul
    {
    public:
        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all client requests (can be overloaded in every specific request)
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(const std::string& addr, const Params&... params)
        : Consul(kwargs::get(params::dc, std::string(), params...),
                kwargs::get(params::token, std::string(), params...),
                addr)
        {
            KWARGS_CHECK_IN_LIST(Params, (params::dc, params::token))
        }

        // Same as Consul(Default_Server_Address, ...)
        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all requests
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(const Params&... params)
            : Consul(Default_Server_Address, params...)
        {}

        ~Consul();

        Consul(Consul &&op);
        Consul& operator= (Consul &&op);

        Consul(const Consul &op) = delete;
        Consul& operator= (const Consul &op) = delete;

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::string> get(http::Status& status, const std::string& path, const Params&... params)
        {
            return get_impl(status, makeUrl(path, params...));
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(http::Status& status, const std::string& path, const std::string& data, const Params&... params)
        {
            return put_impl(status, makeUrl(path, params...), data);
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(const std::string& path, const std::string& data, const Params&... params);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(http::Status& status, const std::string& path, const Params&... params)
        {
            return del_impl(status, makeUrl(path, params...));
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(const std::string& path, const Params&... params);

    private:
        Consul(const std::string& defaultToken, const std::string& dataCenter, const std::string& addr);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string makeUrl(const std::string& path, const Params&... params) const
        {
            using namespace params;
            return parameters::makeUrl(m_addr, path, dc = m_dataCenter, token = m_defaultToken, params...);
        }

        Response<std::string> get_impl(http::Status& status, const std::string& url);
        std::string put_impl(http::Status& status, const std::string& url, const std::string& data);
        std::string del_impl(http::Status& status, const std::string& url);

        std::unique_ptr<impl::HttpClient> m_client; // PIMPL
        std::string m_addr;
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
    std::string Consul::put(const std::string& path, const std::string& data, const Params&... params)
    {
        http::Status s;
        auto r = put(s, path, data, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

    template<class... Params, class>
    std::string Consul::del(const std::string& path, const Params&... params)
    {
        http::Status s;
        auto r = del(s, path, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }
}
