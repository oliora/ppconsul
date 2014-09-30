//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "consul.h"
#include <json11.hpp>
#include <vector>
#include <stdint.h>


namespace ppconsul { namespace kv {

    struct KeyValue
    {
        // Creates invalid KeyValue
        KeyValue()
        : m_createIdx(0), m_modifyIdx(0), m_lockIdx(0), m_flags(0)
        {}

        bool valid() const { return 0 != m_modifyIdx; }

        uint64_t m_createIdx;
        uint64_t m_modifyIdx;
        uint64_t m_lockIdx;
        std::string m_key;
        uint64_t m_flags;
        std::string m_session;
        std::string m_value;
    };


    class UpdateFailed: public ppconsul::Error
    {
    public:
        virtual const char *what() const PPCONSUL_NOEXCEPT override
        {
            return "Update KV storage failed";
        }
    };


    namespace detail {
        inline uint64_t uint64_value(const json11::Json& v)
        {
            // TODO: support full precision of uint64_t
            return static_cast<uint64_t>(v.number_value());
        }

        std::vector<std::string> parseKeys(const std::string& resp);

        std::vector<KeyValue> parseValues(const std::string& resp);

        using ppconsul::detail::throwStatusError;
    }
    

    class Storage
    {
    public:
        explicit Storage(Consul& consul, const std::string& separator = "/")
        : m_consul(consul)
        , m_separator(separator)
        {}

        bool exists(const std::string& key, Consistency cons = Consistency::Default)
        {
            return get(key, cons).valid();
        }

        // Returns invalid KeyValue (i.e. !kv.valid()) if key does not exist
        KeyValue get(const std::string& key, Consistency cons = Consistency::Default)
        {
            http::Status s;
            auto r = m_consul.get(s, keyPath(key));

            if (s.success())
                return detail::parseValues(r).at(0);

            if (NotFoundError::Code != s.code())
                detail::throwStatusError(std::move(s), std::move(r));
            return {};
        }

        // Returns defaultValue if key does not exist
        std::string get(const std::string& key, const std::string& defaultValue, Consistency cons = Consistency::Default)
        {
            const auto kv = get(key, cons);
            if (!kv.valid())
                return defaultValue;
            else
                return kv.m_value;
        }

        // Get values recursively. Returns empty vector if no keys found
        std::vector<KeyValue> getAll(const std::string& keyPrefix, Consistency cons = Consistency::Default)
        {
            http::Status s;
            auto r = m_consul.get(s, keyPath(keyPrefix), { { "recurse", true } });

            if (s.success())
                return detail::parseValues(r);

            if (NotFoundError::Code != s.code())
                detail::throwStatusError(std::move(s), std::move(r));
            return {};
        }

        // Get keys up to a separator provided in ctor. Returns empty vector if no keys found
        std::vector<std::string> getKeys(const std::string& keyPrefix, Consistency cons = Consistency::Default)
        {
            return detail::parseKeys(m_consul.get(keyPath(keyPrefix), { { "keys", true }, { "separator", m_separator } }));
        }

        // Get all keys recursively. Returns empty vector if no keys found
        std::vector<std::string> getAllKeys(const std::string& keyPrefix, Consistency cons = Consistency::Default)
        {
            return detail::parseKeys(m_consul.get(keyPath(keyPrefix), { { "keys", true } }));
        }

        void put(const std::string& key, const std::string& value)
        {
            if ("true" != m_consul.put(keyPath(key), value))
                throw UpdateFailed();
        }

        void put(const std::string& key, const std::string& value, uint64_t flags)
        {
            if ("true" != m_consul.put(keyPath(key), value, { { "flags", flags } }))
                throw UpdateFailed();
        }

        // Returns true if value was successfully set and false otherwise
        bool checkAndSet(const std::string& key, uint64_t cas, const std::string& value)
        {
            return "true" == m_consul.put(keyPath(key), value, { { "cas", cas } });
        }

        // Returns true if value was successfully set and false otherwise
        bool checkAndSet(const std::string& key, uint64_t cas, const std::string& value, uint64_t flags)
        {
            return "true" == m_consul.put(keyPath(key), value, { { "cas", cas }, { "flags", flags } });
        }

        // TODO: acquire/release session

        void erase(const std::string& key)
        {
            m_consul.del(keyPath(key));
        }

        void eraseAll(const std::string& keyPrefix)
        {
            m_consul.del(keyPath(keyPrefix), { { "recurse", true } });
        }

    private:
        std::string keyPath(const std::string& key) const
        {
            return "/v1/kv/" + key;
        }

        Consul& m_consul;
        std::string m_separator;
    };


    // Implementation

    inline std::vector<std::string> detail::parseKeys(const std::string& resp)
    {
        std::string err;
        auto obj = json11::Json::parse(resp, err);

        // TODO check obj.is_null() and throw parsing error

        std::vector<std::string> r;
        r.reserve(obj.array_items().size());

        for (const auto& i: obj.array_items())
            r.push_back(i.string_value());
        return r;
    }

    inline std::vector<KeyValue> detail::parseValues(const std::string& resp)
    {
        std::string err;
        auto obj = json11::Json::parse(resp, err);
        // TODO check obj.is_null() and throw parsing error

        std::vector<KeyValue> r;
        r.reserve(obj.array_items().size());

        for (const auto& i: obj.array_items())
        {
            KeyValue kv;

            const auto& o = i.object_items();

            kv.m_createIdx = uint64_value(o.at("CreateIndex"));
            kv.m_modifyIdx = uint64_value(o.at("ModifyIndex"));
            kv.m_lockIdx   = uint64_value(o.at("LockIndex"));
            kv.m_key       = o.at("Key").string_value();
            kv.m_flags     = uint64_value(o.at("Flags"));
            kv.m_value     = o.at("Value").string_value(); // TODO: decode from base64

            // TODO: generalize
            if (o.count("Session"))
                kv.m_session = o.at("Session").string_value();

            r.push_back(std::move(kv));
        }
        return r;
    }

}}
