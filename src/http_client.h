#pragma once

#include "ppconsul/ppconsul.h"
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <stdint.h>


namespace ppconsul { namespace http {

    class Options
    {
    public:
        Options& operator() ();

    private:
        typedef std::pair<std::string, std::string> Value;
        std::vector<Value> m_values;
    };

    class Status
    {
    public:
        // Creates status with standard success code 200 and empty message
        Status()
        : m_code(200)
        {}

        explicit Status(int code, std::string message = "")
        : m_code(code)
        , m_message(std::move(message))
        {}

        // returns success()
        explicit operator bool () const PPCONSUL_NOEXCEPT { return success(); }

        // Returns true if code() is 2xx (i.e. success) and false otherwise
        bool success() const PPCONSUL_NOEXCEPT { return 2 == m_code / 100; }

        int code() const PPCONSUL_NOEXCEPT { return m_code; }
        const std::string& message() const PPCONSUL_NOEXCEPT { return m_message; }

    private:
        int m_code;
        std::string m_message;
    };

    class StatusError: public std::exception
    {
    public:
        StatusError(Status status)
        : m_status(std::move(status))
        {}

        int code() const PPCONSUL_NOEXCEPT { return m_status.code(); }

        const Status& status() const PPCONSUL_NOEXCEPT { return m_status; }

        // TODO: return string "<code> <message>"
        virtual const char *what() const PPCONSUL_NOEXCEPT override { return m_status.message().c_str(); }

    private:
        Status m_status;
    };

    class Client
    {
    public:
        explicit Client(const char *host = "http://localhost:8500");

        // Throws StatusError on error
        std::string get(const char *path, const Options& options = Options());
        void put(const char *path, const std::string& body, const Options& options = Options());
        void del(const char *path, const Options& options = Options());

        // Returns non-success status thru parameter
        std::string get(Status& status, const char *path, const Options& options = Options());
        void put(Status& status, const char *path, const std::string& body, const Options& options = Options());
        void del(Status& status, const char *path, const Options& options = Options());

    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };


    // Implementation

    inline std::string Client::get(const char *path, const Options& options)
    {
        Status s;
        auto r = get(s, path, options);
        if (!s)
            throw StatusError(std::move(s));
        return r;
    }

    inline void Client::put(const char *path, const std::string& body, const Options& options)
    {
        Status s;
        put(s, path, body, options);
        if (!s)
            throw StatusError(std::move(s));
    }

    inline void Client::del(const char *path, const Options& options)
    {
        Status s;
        del(s, path, options);
        if (!s)
            throw StatusError(std::move(s));
    }

}}

