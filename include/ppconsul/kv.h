//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/json.h"
#include <vector>
#include <stdint.h>


namespace ppconsul { namespace kv {

    struct KeyValue
    {
        // Creates invalid KeyValue
        KeyValue()
        : m_createIndex(0), m_modifyIndex(0), m_lockIndex(0), m_flags(0)
        {}

        bool valid() const { return 0 != m_modifyIndex; }

        uint64_t createIndex() const { return m_createIndex; }
        uint64_t modifyIndex() const { return m_modifyIndex; }
        uint64_t lockIndex() const { return m_lockIndex; }
        uint64_t flags() const { return m_flags; }
        const std::string& key() const { return m_key; }
        const std::string& value() const { return m_value; }
        const std::string& session() const { return m_session; }

        uint64_t m_createIndex;
        uint64_t m_modifyIndex;
        uint64_t m_lockIndex;
        uint64_t m_flags;
        std::string m_key;
        std::string m_value;
        std::string m_session;
    };

    
    class UpdateError: public ppconsul::Error
    {
    public:
        UpdateError(std::string key)
        : m_key(key)
        {}

        const std::string& key() const PPCONSUL_NOEXCEPT { return m_key; }

        virtual const char *what() const PPCONSUL_NOEXCEPT override;

    private:
        std::string m_key;
        mutable std::string m_what;
    };

    class Storage
    {
    public:
        explicit Storage(Consul& consul)
        : m_consul(consul)
        {}

        size_t count(const std::string& key, Consistency cons = Consistency::Default) const
        {
            return item(key, cons).valid() ? 1 : 0;
        }

        size_t countAll(const std::string& keyPrefix = "", Consistency cons = Consistency::Default) const
        {
            return keys(keyPrefix, cons).size();
        }

        size_t size(Consistency cons = Consistency::Default) const
        {
            return countAll("", cons);
        }

        bool empty(Consistency cons = Consistency::Default) const
        {
            return 0 == countAll("", cons);
        }

        void erase(const std::string& key)
        {
            m_consul.del(keyPath(key));
        }

        void eraseAll(const std::string& keyPrefix)
        {
            m_consul.del(keyPath(keyPrefix), { { "recurse", true } });
        }

        void clear()
        {
            eraseAll("");
        }

        // Returns value's value or defaultValue if key does not exist.
        std::string get(const std::string& key, const std::string& defaultValue, Consistency cons = Consistency::Default) const;

        // Returns invalid KeyValue (i.e. !kv.valid()) if key does not exist. Result contains both value and headers.
        Response<KeyValue> item(WithHeaders, const std::string& key, Consistency cons = Consistency::Default) const;

        // Returns invalid KeyValue (i.e. !kv.valid()) if key does not exist. Result contains value only.
        KeyValue item(const std::string& key, Consistency cons = Consistency::Default) const
        {
            return std::move(item(withHeaders, key, cons).value());
        }

        // Get values recursively. Returns empty vector if no keys found. Result contains both value and headers.
        Response<std::vector<KeyValue>> items(WithHeaders, const std::string& keyPrefix = "", Consistency cons = Consistency::Default) const;

        // Get values recursively. Returns empty vector if no keys found. Result contains value only.
        std::vector<KeyValue> items(const std::string& keyPrefix = "", Consistency cons = Consistency::Default) const
        {
            return std::move(items(withHeaders, keyPrefix, cons).value());
        }

        // Get keys up to a separator provided. Returns empty vector if no keys found. Result contains both value and headers.
        Response<std::vector<std::string>> keys(WithHeaders, const std::string& keyPrefix, const std::string& separator, Consistency cons = Consistency::Default) const;

        // Get keys up to a separator provided. Returns empty vector if no keys found. Result contains value only.
        std::vector<std::string> keys(const std::string& keyPrefix, const std::string& separator, Consistency cons = Consistency::Default) const
        {
            return std::move(keys(withHeaders, keyPrefix, separator, cons).value());
        }

        // Get all keys recursively. Returns empty vector if no keys found. Result contains both value and headers.
        Response<std::vector<std::string>> keys(WithHeaders, const std::string& keyPrefix = "", Consistency cons = Consistency::Default) const;

        // Get all keys recursively. Returns empty vector if no keys found. Result contains value only.
        std::vector<std::string> keys(const std::string& keyPrefix = "", Consistency cons = Consistency::Default) const
        {
            return std::move(keys(withHeaders, keyPrefix, cons).value());
        }

        // Throws UpdateError if value can not be set.
        void put(const std::string& key, const std::string& value)
        {
            if ("true" != m_consul.put(keyPath(key), value))
                throw UpdateError(key);
        }

        // Throws UpdateError if value can not be set.
        void put(const std::string& key, const std::string& value, uint64_t flags)
        {
            if ("true" != m_consul.put(keyPath(key), value, { { "flags", flags } }))
                throw UpdateError(key);
        }

        // Compare and set. Returns true if value was successfully set and false otherwise.
        bool cas(const std::string& key, uint64_t cas, const std::string& value)
        {
            return "true" == m_consul.put(keyPath(key), value, { { "cas", cas } });
        }

        // Compare and set. Returns true if value was successfully set and false otherwise.
        bool cas(const std::string& key, uint64_t cas, const std::string& value, uint64_t flags)
        {
            return "true" == m_consul.put(keyPath(key), value, { { "cas", cas }, { "flags", flags } });
        }

        // TODO: acquire/release session

    private:
        std::string keyPath(const std::string& key) const
        {
            return "/v1/kv/" + helpers::encodeUrl(key);
        }

        Consul& m_consul;
    };


    // Implementation

    namespace impl {
        inline std::vector<std::string> parseKeys(const std::string& resp)
        {
            auto obj = json::parse_json(resp);

            std::vector<std::string> r;
            r.reserve(obj.array_items().size());

            for (const auto& i : obj.array_items())
                r.push_back(i.string_value());
            return r;
        }

        inline std::vector<KeyValue> parseValues(const std::string& resp)
        {
            using namespace json;

            auto obj = parse_json(resp);

            std::vector<KeyValue> r;
            r.reserve(obj.array_items().size());

            for (const auto& i : obj.array_items())
            {
                const auto& o = i.object_items();

                KeyValue kv;
                kv.m_createIndex = uint64_value(o.at("CreateIndex"));
                kv.m_modifyIndex = uint64_value(o.at("ModifyIndex"));
                kv.m_lockIndex = uint64_value(o.at("LockIndex"));
                kv.m_key = o.at("Key").string_value();
                kv.m_flags = uint64_value(o.at("Flags"));
                kv.m_value = helpers::decodeBase64(o.at("Value").string_value());
                kv.m_session = i["Session"].string_value(); // generalize getting of optional values

                r.push_back(std::move(kv));
            }
            return r;
        }
    }

    inline const char *UpdateError::what() const PPCONSUL_NOEXCEPT
    {
        if (m_what.empty())
            m_what = helpers::format("Update of key '%s' failed", m_key.c_str());

        return m_what.c_str();
    }

    inline Response<KeyValue> Storage::item(WithHeaders, const std::string& key, Consistency cons) const
    {
        http::Status s;
        auto r = m_consul.get(s, keyPath(key));

        if (s.success())
            return makeResponse(r.headers(), std::move(impl::parseValues(r.value()).at(0)));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.value()));
    }

    inline std::string Storage::get(const std::string& key, const std::string& defaultValue, Consistency cons) const
    {
        const auto kv = item(withHeaders, key, cons);
        if (!kv.value().valid())
            return defaultValue;
        else
            return std::move(kv.value().value());
    }

    inline Response<std::vector<KeyValue>> Storage::items(WithHeaders, const std::string& keyPrefix, Consistency cons) const
    {
        http::Status s;
        auto r = m_consul.get(s, keyPath(keyPrefix), { { "recurse", true } });

        if (s.success())
            return makeResponse(r.headers(), impl::parseValues(r.value()));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.value()));
    }

    inline Response<std::vector<std::string>> Storage::keys(WithHeaders, const std::string& keyPrefix, const std::string& separator, Consistency cons) const
    {
        http::Status s;
        auto r = m_consul.get(s, keyPath(keyPrefix), { { "keys", true }, { "separator", helpers::encodeUrl(separator) } });
        if (s.success())
            return makeResponse(r.headers(), impl::parseKeys(r.value()));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.value()));
    }

    inline Response<std::vector<std::string>> Storage::keys(WithHeaders, const std::string& keyPrefix, Consistency cons) const
    {
        http::Status s;
        auto r = m_consul.get(s, keyPath(keyPrefix), { { "keys", true } });
        if (s.success())
            return makeResponse(r.headers(), impl::parseKeys(r.value()));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.value()));
    }
}}
