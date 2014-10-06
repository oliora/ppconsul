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

        // Declared in header to be available for tests
        std::string makeUrl(const std::string& addr, const std::string& path, const Parameters& parameters = Parameters());
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

    struct WithHeaders {};
    const WithHeaders withHeaders;

    const char Default_Server_Address[] = "localhost:8500";

    class Consul
    {
    public:
        explicit Consul(const std::string& dataCenter = "", const std::string& addr = Default_Server_Address);
        ~Consul();

        Consul(Consul &&op);
        Consul& operator= (Consul &&op);

        Consul(const Consul &op) = delete;
        Consul& operator= (const Consul &op) = delete;

        //Response<std::string> get(const std::string& path, const Parameters& params = Parameters());
        Response<std::string> get(http::Status& status, const std::string& path, const Parameters& params = Parameters());

        std::string put(const std::string& path, const std::string& body, const Parameters& params = Parameters());
        std::string put(http::Status& status, const std::string& path, const std::string& body, const Parameters& params = Parameters());

        std::string del(const std::string& path, const Parameters& params = Parameters());
        std::string del(http::Status& status, const std::string& path, const Parameters& params = Parameters());

    private:
        std::string makeUrl(const std::string& path, const Parameters& params = Parameters()) const;

        Parameters m_defaultParams;
        std::string m_addr;
        std::unique_ptr<impl::HttpClient> m_client; // PIMPL
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

    /*inline Response<std::string> Consul::get(const std::string& path, const Parameters& parameters)
    {
        http::Status s;
        auto r = get(s, path, parameters);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r.first));
        return r;
    }*/

    inline std::string Consul::put(const std::string& path, const std::string& body, const Parameters& parameters)
    {
        http::Status s;
        auto r = put(s, path, body, parameters);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

    inline std::string Consul::del(const std::string& path, const Parameters& parameters)
    {
        http::Status s;
        auto r = del(s, path, parameters);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

}
