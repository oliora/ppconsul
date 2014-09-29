#pragma once

#include "config.h"
#include "parameters.h"
#include "http_status.h"
#include <string>
#include <memory>
#include <stdexcept>


namespace ppconsul {

    namespace impl {
        class HttpClient; // Forward declaraion to use with PIMPL

        std::string makeUrl(const std::string& addr, const std::string& path, const Parameters& parameters = Parameters());
    }

    class Error: public std::exception {};

    class BadStatus: public Error
    {
    public:
        BadStatus(http::Status status)
        : m_status(std::move(status))
        {}

        int code() const PPCONSUL_NOEXCEPT{ return m_status.code(); }

        const http::Status& status() const PPCONSUL_NOEXCEPT{ return m_status; }

        virtual const char *what() const PPCONSUL_NOEXCEPT override;

    private:
        http::Status m_status;
        mutable std::string m_what;
    };

    enum class Consistency
    {
        Default,
        Consistent,
        Stale
    };

    const char Default_Server_Address[] = "localhost:8500";

    class Consul
    {
    public:
        explicit Consul(const std::string& dataCenter = "", const std::string& addr = Default_Server_Address);
        ~Consul();

        std::string get(const std::string& path, const Parameters& params = Parameters());
        std::string get(http::Status& status, const std::string& path, const Parameters& params = Parameters());

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

    inline std::string Consul::get(const std::string& path, const Parameters& parameters)
    {
        http::Status s;
        auto r = get(s, path, parameters);
        if (!s)
            throw BadStatus(std::move(s));
        return r;
    }

    inline std::string Consul::put(const std::string& path, const std::string& body, const Parameters& parameters)
    {
        http::Status s;
        auto r = put(s, path, body, parameters);
        if (!s)
            throw BadStatus(std::move(s));
        return r;
    }

    inline std::string Consul::del(const std::string& path, const Parameters& parameters)
    {
        http::Status s;
        auto r = del(s, path, parameters);
        if (!s)
            throw BadStatus(std::move(s));
        return r;
    }

}
