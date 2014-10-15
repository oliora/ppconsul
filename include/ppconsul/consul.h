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
#include <memory>
#include <stdexcept>
#include <limits>


namespace ppconsul {

    namespace impl {
        // Forward declaraion to use with PIMPL
        class HttpClient;
    }

    class BadStatus: public Error
    {
    public:
        explicit BadStatus(http::Status status, std::string message = "")
        : m_status(std::move(status))
        , m_message(std::move(message))
        {}

        int code() const PPCONSUL_NOEXCEPT{ return m_status.code(); }

        const http::Status& status() const PPCONSUL_NOEXCEPT{ return m_status; }
        const std::string& message() const PPCONSUL_NOEXCEPT{ return m_message; }

        virtual const char *what() const PPCONSUL_NOEXCEPT override;

    private:
        http::Status m_status;
        std::string m_message;
        mutable std::string m_what;
    };

    class NotFoundError: public BadStatus
    {
    public:
        enum { Code = 404 };

        /*explicit NotFoundError(http::Status status, std::string message = "")
        : BadStatus(std::move(status), std::move(message))
        {}*/

        NotFoundError()
        : BadStatus(http::Status(Code, "Not Found"))
        {}
    };

    void throwStatusError(http::Status status, std::string body = "");

    enum class Consistency
    {
        Default,
        Consistent,
        Stale
    };

    namespace params {
        PPCONSUL_PARAM(dc, std::string)
        PPCONSUL_PARAM(token, std::string)
        PPCONSUL_PARAM_NO_NAME(consistency, Consistency)

        inline void printParameter(std::ostream& os, Consistency consistency, KWARGS_KW_TAG(consistency))
        {
            switch (consistency)
            {
            case Consistency::Stale: os << "stale"; break;
            case Consistency::Consistent: os << "consistent"; break;
            default: break;
            }
        }
    }

    struct WithHeaders {};
    const WithHeaders withHeaders{};

    const char Default_Server_Address[] = "localhost:8500";

    class Consul
    {
    public:
        // Allowed parameters:
        // - dc - default dc for all client requests
        // - token - default token for all client requests
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(const std::string& addr, const Params&... params)
            : Consul(kwargs::get(params::dc, "", params...),
                kwargs::get(params::token, "", params...),
                addr)
        {}

        // Same as Consul(Default_Server_Address, ...)
        // Allowed parameters:
        // - dc - default dc for all client requests
        // - token - default token for all client requests
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
        std::string put(http::Status& status, const std::string& path, const std::string& body, const Params&... params)
        {
            return put_impl(status, makeUrl(path, params...), body);
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(const std::string& path, const std::string& body, const Params&... params);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(http::Status& status, const std::string& path, const Params&... params)
        {
            return del_impl(status, makeUrl(path, params...));
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(const std::string& path, const Params&... params);

    private:
        Consul(const std::string& dataCenter, const std::string& token, const std::string& addr);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string makeUrl(const std::string& path, const Params&... params) const
        {
            using namespace params;
            return parameters::makeUrl(m_addr, path, dc = m_dataCenter, token = m_token, params...);
        }

        Response<std::string> get_impl(http::Status& status, const std::string& url);
        std::string put_impl(http::Status& status, const std::string& url, const std::string& body);
        std::string del_impl(http::Status& status, const std::string& url);

        std::string m_addr;
        std::unique_ptr<impl::HttpClient> m_client; // PIMPL

        // Default params
        std::string m_dataCenter;
        std::string m_token;
    };

    // Implementation

    inline void throwStatusError(http::Status status, std::string body)
    {
        if (NotFoundError::Code == status.code())
        {
            throw NotFoundError{};
            //throw NotFoundError(std::move(status), std::move(body));
        }
        else
        {
            throw BadStatus(std::move(status), std::move(body));
        }
    }

    template<class... Params, class>
    std::string Consul::put(const std::string& path, const std::string& body, const Params&... params)
    {
        http::Status s;
        auto r = put(s, path, body, params...);
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
