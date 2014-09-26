#pragma once

#include "ppconsul/ppconsul.h"
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <stdint.h>
#include <numeric>
#include <type_traits>


namespace ppconsul { namespace http {

    namespace detail {

//        inline std::string to_string(std::string v)
//        {
//            return std::move(v);
//        }
//
//        inline std::string to_string(const char *v)
//        {
//            return {v};
//        }

        inline std::string to_string(char v)
        {
            return {1, v};
        }

        template<class T,
            class Enabler = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        inline std::string to_string(T t)
        {
            return std::to_string(t);
        }


        class Parameter
        {
        public:
            Parameter(std::string name, std::string value)
            : m_name(std::move(name))
            , m_value(std::move(value))
            {}

            template<class V,
                class Disabler = typename std::enable_if<!std::is_constructible<std::string, V>::value>::type>
            Parameter(std::string name, V value)
            : m_name(std::move(name))
            , m_value(to_string(value))
            {}

            std::string m_name, m_value;
        };

    }

    class Parameters
    {
    public:
        using Parameter = detail::Parameter;

        Parameters()
        {}

        /*template<class V>
        Parameters(std::string name, V value)
        {
            m_values.emplace_back(std::move(name), std::move(value));
        }*/

        Parameters(std::initializer_list<Parameter> values)
        : m_values(values)
        {}

        void append(std::initializer_list<Parameter> values)
        {
            m_values.insert(m_values.end(), values.begin(), values.end());
        }

        /*template<class V>
        void append(std::string name, V value)
        {
            m_values.emplace_back(std::move(name), std::move(value));
        }*/

        bool empty() const { return m_values.empty(); }
        void clear() { m_values.clear(); }

        std::string query() const;

    private:
        std::vector<Parameter> m_values;
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

        // Returns success()
        explicit operator bool () const PPCONSUL_NOEXCEPT { return success(); }

        // Returns true if code() is 2xx (i.e. success) and false otherwise
        bool success() const PPCONSUL_NOEXCEPT { return 2 == m_code / 100; }

        int code() const PPCONSUL_NOEXCEPT { return m_code; }
        const std::string& message() const PPCONSUL_NOEXCEPT { return m_message; }

    private:
        int m_code;
        std::string m_message;
    };


    class BadStatus: public std::exception
    {
    public:
        BadStatus(Status status)
        : m_status(std::move(status))
        {}

        int code() const PPCONSUL_NOEXCEPT { return m_status.code(); }

        const Status& status() const PPCONSUL_NOEXCEPT { return m_status; }

        virtual const char *what() const PPCONSUL_NOEXCEPT override;

    private:
        Status m_status;
        mutable std::string m_what;
    };


    class Client
    {
    public:
        explicit Client(const char *host = "http://localhost:8500");
        ~Client();

        std::string get(const char *path, const Parameters& parameters = Parameters());
        std::string get(Status& status, const char *path, const Parameters& parameters = Parameters());

        void put(const char *path, const std::string& body, const Parameters& parameters = Parameters());
        void put(Status& status, const char *path, const std::string& body, const Parameters& parameters = Parameters());

        void  del(const char *path, const Parameters& parameters = Parameters());
        void  del(Status& status, const char *path, const Parameters& parameters = Parameters());

    private:
        class Impl;

        std::string createUri(const char *path, const Parameters& parameters = Parameters()) const;

        std::unique_ptr<Impl> m_impl;
        std::string m_host;
    };


    // Implementation

    inline std::string Client::get(const char *path, const Parameters& parameters)
    {
        Status s;
        auto r = get(s, path, parameters);
        if (!s)
            throw BadStatus(std::move(s));
        return r;
    }

    inline void Client::put(const char *path, const std::string& body, const Parameters& parameters)
    {
        Status s;
        put(s, path, body, parameters);
        if (!s)
            throw BadStatus(std::move(s));
    }

    inline void Client::del(const char *path, const Parameters& parameters)
    {
        Status s;
        del(s, path, parameters);
        if (!s)
            throw BadStatus(std::move(s));
    }

    inline std::string Client::createUri(const char *path, const Parameters& parameters) const
    {
        auto query = parameters.query();

        std::string r;
        r.reserve(m_host.size() + strlen(path) + query.size() + 1); // 1 is for query prefix '?'

        r += m_host;
        r += path;
        r += '?';
        r += query;

        return r;
    }

}}

