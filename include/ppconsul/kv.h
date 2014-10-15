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

    namespace params {
        using ppconsul::params::consistency;

        PPCONSUL_PARAM(cas, uint64_t)
        PPCONSUL_PARAM(flags, uint64_t)
        PPCONSUL_PARAM(recurse, bool)
        PPCONSUL_PARAM(keys, bool)
        PPCONSUL_PARAM(separator, std::string)

        namespace groups {
            PPCONSUL_PARAMS_GROUP(get, (consistency))
            PPCONSUL_PARAMS_GROUP(put, (flags))
        }
    }

    class Storage
    {
    public:
        explicit Storage(Consul& consul)
        : m_consul(consul)
        {}

        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        size_t count(const std::string& key, const Params&... params) const { return item(key, params...).valid() ? 1 : 0; }

        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        size_t countAll(const std::string& keyPrefix, const Params&... params) const { return keys(keyPrefix, params...).size(); }

        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        size_t countAll(const Params&... params) const { return countAll(std::string(), params...); }

        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        size_t size(const Params&... params) const { return countAll(std::string(), params...); }

        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        bool empty(const Params&... params) const
        {
            return 0 == countAll(std::string(), params...);
        }

        void erase(const std::string& key)
        {
            m_consul.del(keyPath(key));
        }

        void eraseAll(const std::string& keyPrefix)
        {
            m_consul.del(keyPath(keyPrefix), params::recurse = true);
        }

        void clear()
        {
            eraseAll(std::string());
        }

        // Returns value's value or defaultValue if key does not exist.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string get(const std::string& key, const std::string& defaultValue, const Params&... params) const;

        // Returns invalid KeyValue (i.e. !kv.valid()) if key does not exist. Result contains both value and headers.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<KeyValue> item(WithHeaders, const std::string& key, const Params&... params) const;

        // Returns invalid KeyValue (i.e. !kv.valid()) if key does not exist. Result contains value only.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        KeyValue item(const std::string& key, const Params&... params) const
        {
            return std::move(item(withHeaders, key, params...).value());
        }

        // Recursively get values which key starting with specified prefix. Returns empty vector if no keys found.
        // Result contains both value and headers.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<KeyValue>> items(WithHeaders, const std::string& keyPrefix, const Params&... params) const;

        // Recursively get values which key starting with specified prefix. Returns empty vector if no keys found.
        // Result contains value only.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<KeyValue> items(const std::string& keyPrefix, const Params&... params) const
        {
            return std::move(items(withHeaders, keyPrefix, params...).value());
        }

        // Get all values. Returns empty vector if no keys found. Same as items(withHeader, "")
        // Result contains both value and headers.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<KeyValue>> items(WithHeaders, const Params&... params) const
        {
            return items(withHeaders, std::string(), params...);
        }

        // Get all values. Returns empty vector if no keys found. Same as items("")
        // Result contains value only.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<KeyValue> items(const Params&... params) const
        {
            return items(std::string(), params...);
        }

        // Get keys up to a separator provided. Returns empty vector if no keys found. Result contains both value and headers.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<std::string>> keys(WithHeaders, const std::string& keyPrefix, const std::string& separator, const Params&... params) const
        {
            KWARGS_CHECK_IN_GROUP(Params, kv::params::groups::get)
            return get_keys_impl(keyPrefix, kv::params::separator = helpers::encodeUrl(separator), params...);
        }

        // Get keys up to a separator provided. Returns empty vector if no keys found. Result contains value only.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<std::string> keys(const std::string& keyPrefix, const std::string& separator, const Params&... params) const
        {
            return std::move(keys(withHeaders, keyPrefix, separator, params...).value());
        }

        // Get all keys strarting with specified prefix. Returns empty vector if no keys found. Result contains both value and headers.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<std::string>> keys(WithHeaders, const std::string& keyPrefix, const Params&... params) const
        {
            KWARGS_CHECK_IN_GROUP(Params, kv::params::groups::get)
            return get_keys_impl(keyPrefix, params...);
        }

        // Get all keys. Returns empty vector if no keys found. Result contains both value and headers.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<std::string>> keys(WithHeaders, const Params&... params) const
        {
            return keys(withHeaders, std::string(), params...);
        }

        // Get all keys strarting with specified prefix. Returns empty vector if no keys found. Result contains value only.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<std::string> keys(const std::string& prefix, const Params&... params) const
        {
            return std::move(keys(withHeaders, prefix, params...).value());
        }

        // Get all keys. Returns empty vector if no keys found. Result contains value only.
        // Allowed parameters:
        // - group::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<std::string> keys(const Params&... params) const
        {
            return keys(std::string(), params...);
        }

        // Throws UpdateError if value can not be set.
        // Allowed parameters:
        // - group::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        void put(const std::string& key, const std::string& value, const Params&... params)
        {
            KWARGS_CHECK_IN_GROUP(Params, kv::params::groups::put)
            if ("true" != m_consul.put(keyPath(key), value, params...))
                throw UpdateError(key);
        }

        // Compare and set. Returns true if value was successfully set and false otherwise.
        // Allowed parameters:
        // - group::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        bool cas(const std::string& key, uint64_t cas, const std::string& value, const Params&... params)
        {
            KWARGS_CHECK_IN_GROUP(Params, kv::params::groups::put)
            return "true" == m_consul.put(keyPath(key), value, params::cas = cas, params...);
        }

        // TODO: acquire/release session

    private:
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<std::string>> get_keys_impl(const std::string& keyPrefix, const Params&... params) const;

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

    template<class... Params, class>
    Response<KeyValue> Storage::item(WithHeaders, const std::string& key, const Params&... params) const
    {
        KWARGS_CHECK_IN_GROUP(Params, kv::params::groups::get)
        http::Status s;
        auto r = m_consul.get(s, keyPath(key), params...);

        if (s.success())
            return makeResponse(r.headers(), std::move(impl::parseValues(r.value()).at(0)));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.value()));
    }

    template<class... Params, class>
    std::string Storage::get(const std::string& key, const std::string& defaultValue, const Params&... params) const
    {
        const auto kv = item(withHeaders, key, params...);
        if (!kv.value().valid())
            return defaultValue;
        else
            return std::move(kv.value().value());
    }

    template<class... Params, class>
    Response<std::vector<KeyValue>> Storage::items(WithHeaders, const std::string& keyPrefix, const Params&... params) const
    {
        KWARGS_CHECK_IN_GROUP(Params, kv::params::groups::get)
        http::Status s;
        auto r = m_consul.get(s, keyPath(keyPrefix), kv::params::recurse = true, params...);

        if (s.success())
            return makeResponse(r.headers(), impl::parseValues(r.value()));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.value()));
    }

    template<class... Params, class>
    Response<std::vector<std::string>> Storage::get_keys_impl(const std::string& keyPrefix, const Params&... params) const
    {
        http::Status s;
        auto r = m_consul.get(s, keyPath(keyPrefix), kv::params::keys = true, params...);
        if (s.success())
            return makeResponse(r.headers(), impl::parseKeys(r.value()));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.value()));
    }
}}
