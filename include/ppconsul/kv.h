//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/types.h"
#include <boost/variant.hpp>
#include <vector>
#include <stdint.h>


namespace ppconsul { namespace kv {

    struct KeyValue
    {
        bool valid() const { return 0 != modifyIndex; }
        explicit operator bool() const { return valid(); }

        uint64_t createIndex = 0;
        uint64_t modifyIndex = 0;
        uint64_t lockIndex = 0;
        uint64_t flags = 0;
        std::string key;
        std::string value;
        std::string session;
    };

    namespace txn_ops {

        struct Set
        {
            std::string key;
            std::string value;
            uint64_t flags;
        };

        struct CompareSet
        {
            std::string key;
            uint64_t expectedIndex;
            std::string value;
            uint64_t flags;
        };

        struct Get
        {
            std::string key;
        };

        struct GetAll
        {
            std::string keyPrefix;
        };

        struct CheckIndex
        {
            std::string key;
            uint64_t expectedIndex;
        };

        struct CheckNotExists
        {
            std::string key;
        };

        struct Erase
        {
            std::string key;
        };

        struct EraseAll
        {
            std::string keyPrefix;
        };

        struct CompareErase
        {
            std::string key;
            uint64_t expectedIndex;
        };

        struct Lock
        {
            std::string key;
            std::string value;
            std::string session;
            uint64_t flags;
        };

        struct Unlock
        {
            std::string key;
            std::string value;
            std::string session;
            uint64_t flags;
        };

        struct CheckSession
        {
            std::string key;
            std::string session;
        };
    }

    using TxnOperation = boost::variant<
        txn_ops::Set,
        txn_ops::CompareSet,
        txn_ops::Get,
        txn_ops::GetAll,
        txn_ops::CheckIndex,
        txn_ops::CheckNotExists,
        txn_ops::Erase,
        txn_ops::EraseAll,
        txn_ops::CompareErase,
        txn_ops::Lock,
        txn_ops::Unlock,
        txn_ops::CheckSession
    >;

    struct TxnError
    {
        int opIndex;
        std::string what;
    };

    class TxnAborted: public ppconsul::Error
    {
    public:
        explicit TxnAborted(std::vector<TxnError> errors)
            : m_errors(std::move(errors))
        {}

        virtual const char *what() const noexcept override
        {
            if (m_what.empty())
                m_what = "Transaction aborted: " + (m_errors.empty() ? "no errors" : m_errors[0].what);
            return m_what.c_str();
        }

        const std::vector<TxnError>& errors() const noexcept { return m_errors; }

    private:
        std::vector<TxnError> m_errors;
        mutable std::string m_what;
    };

    class UpdateError: public ppconsul::Error
    {
    public:
        UpdateError(std::string key)
        : m_key(key)
        {}

        const std::string& key() const noexcept { return m_key; }

        virtual const char *what() const noexcept override;

    private:
        std::string m_key;
        mutable std::string m_what;
    };

    namespace kw {
        using ppconsul::kw::consistency;
        using ppconsul::kw::block_for;
        using ppconsul::kw::dc;
        using ppconsul::kw::token;

        PPCONSUL_KEYWORD(cas, uint64_t)
        PPCONSUL_KEYWORD(flags, uint64_t)
        PPCONSUL_KEYWORD(recurse, bool)
        PPCONSUL_KEYWORD(keys, bool)
        PPCONSUL_KEYWORD(separator, std::string)
        PPCONSUL_KEYWORD(acquire, std::string)
        PPCONSUL_KEYWORD(release, std::string)

        namespace groups {
            KWARGS_KEYWORDS_GROUP(get, (consistency, dc, block_for, token))
            KWARGS_KEYWORDS_GROUP(put, (flags, token, dc))
            KWARGS_KEYWORDS_GROUP(txn, (consistency, token, dc))
        }
    }

    namespace impl {
        StringList parseKeys(const std::string& resp);
        std::vector<KeyValue> parseValues(const std::string& resp);

        std::vector<KeyValue> txnParseValues(const std::string& resp);
        TxnAborted txnParseErrors(const std::string& resp);
        std::string txnBodyJson(const std::vector<TxnOperation> &ops);
    }

    class Kv
    {
    public:
        // Allowed parameters:
        // - consistency - default consistency for all "get" requests
        // - token - default token for all requests
        // - dc - default dc for all requests
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Kv(Consul& consul, const Params&... params)
        : m_consul(consul)
        , m_defaultToken(kwargs::get_opt(kw::token, std::string(), params...))
        , m_defaultConsistency(kwargs::get_opt(kw::consistency, Consistency::Default, params...))
        , m_defaultDc(kwargs::get_opt(kw::dc, std::string(), params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::consistency, kw::token, kw::dc))
        }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        size_t count(const std::string& key, const Params&... params) const { return item(key, params...).valid() ? 1 : 0; }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        size_t countAll(const std::string& keyPrefix, const Params&... params) const { return keys(keyPrefix, params...).size(); }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        size_t countAll(const Params&... params) const { return countAll(std::string(), params...); }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        size_t size(const Params&... params) const { return countAll(std::string(), params...); }

        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        bool empty(const Params&... params) const
        {
            return 0 == countAll(std::string(), params...);
        }

        void erase(const std::string& key) const
        {
            m_consul.del(keyPath(key),
                kw::token = m_defaultToken, kw::dc = m_defaultDc);
        }

        void eraseAll(const std::string& keyPrefix) const
        {
            m_consul.del(keyPath(keyPrefix),
                         kw::token = m_defaultToken, kw::dc = m_defaultDc,
                         kw::recurse = true);
        }

        void clear() const
        {
            eraseAll(std::string());
        }

        // Returns value's value or defaultValue if key does not exist.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string get(const std::string& key, const std::string& defaultValue, const Params&... params) const;

        // Returns invalid KeyValue (i.e. !kv.valid()) if key does not exist.
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<KeyValue> item(WithHeaders, const std::string& key, const Params&... params) const;

        // Returns invalid KeyValue (i.e. !kv.valid()) if key does not exist.
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        KeyValue item(const std::string& key, const Params&... params) const
        {
            return std::move(item(withHeaders, key, params...).data());
        }

        // Recursively get values which key starting with specified prefix. Returns empty vector if no keys found.
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<KeyValue>> items(WithHeaders, const std::string& keyPrefix, const Params&... params) const;

        // Recursively get values which key starting with specified prefix. Returns empty vector if no keys found.
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<KeyValue> items(const std::string& keyPrefix, const Params&... params) const
        {
            return std::move(items(withHeaders, keyPrefix, params...).data());
        }

        // Get all values. Returns empty vector if no keys found. Same as items(withHeader, "")
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::vector<KeyValue>> items(WithHeaders, const Params&... params) const
        {
            return items(withHeaders, std::string(), params...);
        }

        // Get all values. Returns empty vector if no keys found. Same as items("")
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<KeyValue> items(const Params&... params) const
        {
            return items(std::string(), params...);
        }

        // Get keys starting with specified prefix up to a separator provided. Returns empty vector if no keys found.
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<StringList> subKeys(WithHeaders, const std::string& keyPrefix, const std::string& separator, const Params&... params) const
        {
            KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::get))
            return get_keys_impl(keyPrefix, kv::kw::separator = separator, params...);
        }

        // Get keys starting with specified prefix up to a separator provided. Returns empty vector if no keys found.
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        StringList subKeys(const std::string& keyPrefix, const std::string& separator, const Params&... params) const
        {
            return std::move(subKeys(withHeaders, keyPrefix, separator, params...).data());
        }

        // Get all keys strarting with specified prefix. Returns empty vector if no keys found.
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<StringList> keys(WithHeaders, const std::string& keyPrefix, const Params&... params) const
        {
            KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::get))
            return get_keys_impl(keyPrefix, params...);
        }

        // Get all keys strarting with specified prefix. Returns empty vector if no keys found.
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        StringList keys(const std::string& prefix, const Params&... params) const
        {
            return std::move(keys(withHeaders, prefix, params...).data());
        }

        // Get all keys. Returns empty vector if no keys found.
        // Result contains both headers and data.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<StringList> keys(WithHeaders, const Params&... params) const
        {
            return keys(withHeaders, std::string(), params...);
        }

        // Get all keys. Returns empty vector if no keys found.
        // Result contains data only.
        // Allowed parameters:
        // - groups::get
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        StringList keys(const Params&... params) const
        {
            return keys(std::string(), params...);
        }

        // Throws UpdateError if value can not be set.
        // Allowed parameters:
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        void set(const std::string& key, const std::string& value, const Params&... params) const
        {
            KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::put))
            auto r = helpers::parseJsonBool(
               m_consul.put(keyPath(key), value,
                            kw::token = m_defaultToken, kw::dc = m_defaultDc,
                            params...));
            if (!r)
                throw UpdateError(key);
        }

        // Compare and set (CAS operation). Returns true if value was successfully set and false otherwise.
        // Allowed parameters:
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        bool compareSet(const std::string& key, uint64_t expectedIndex, const std::string& value, const Params&... params) const
        {
            KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::put))
            return helpers::parseJsonBool(
                m_consul.put(keyPath(key), value,
                    kw::token = m_defaultToken, kw::dc = m_defaultDc,
                    kw::cas = expectedIndex, params...));
        }

        // Compare and erase (CAS operation).
        bool compareErase(const std::string& key, uint64_t expectedIndex) const
        {
            return helpers::parseJsonBool(
                m_consul.del(keyPath(key),
                    kw::token = m_defaultToken, kw::dc = m_defaultDc, kw::cas = expectedIndex));
        }

        // Acquire the lock. Returns true if the lock is acquired and value is set, returns false otherwise.
        // Allowed parameters:
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        bool lock(const std::string& key, const std::string& session, const std::string& value, const Params&... params) const
        {
            KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::put))
            return helpers::parseJsonBool(
                m_consul.put(keyPath(key), value,
                    kw::token = m_defaultToken, kw::dc = m_defaultDc,
                    kw::acquire = session, params...));
        }

        // Acquire the lock. Returns true if the lock is acquired and value is set, returns false otherwise.
        // Allowed parameters:
        // - groups::put
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        bool unlock(const std::string& key, const std::string& session, const std::string& value, const Params&... params) const
        {
            KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::put))
            return helpers::parseJsonBool(
                m_consul.put(keyPath(key), value,
                    kw::token = m_defaultToken, kw::dc = m_defaultDc,
                    kw::release = session, params...));
        }

        // Perform a series of operations as a transaction.
        // A KeyValue element is appended to the result for each operation except for:
        // - txn_ops::Erase
        // - txn_ops::EraseAll
        // - txn_ops::CompareErase
        // - txn_ops::CheckNotExists
        // If the transaction was rolled back, TxnAborted is thrown.
        //
        // Allowed parameters:
        // - groups::txn
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::vector<KeyValue> commit(const std::vector<TxnOperation> &ops, const Params&... params) const
        {
            KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::txn))

            http::Status status;
            auto data = m_consul.put(
                status,
                "/v1/txn", impl::txnBodyJson(ops),
                kw::token = m_defaultToken, kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                params...);

            switch (status.code()) {
            case 200:
                return impl::txnParseValues(std::move(data));
            case 409:
                throw impl::txnParseErrors(std::move(data));
            default:
                throw BadStatus(std::move(status), std::move(data));
            }
        }

    private:
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<StringList> get_keys_impl(const std::string& keyPrefix, const Params&... params) const;

        std::string keyPath(const std::string& key) const
        {
            return "/v1/kv/" + helpers::encodeUrl(key);
        }

        Consul& m_consul;

        std::string m_defaultToken;
        Consistency m_defaultConsistency;
        std::string m_defaultDc;
    };

    // TODO: add ScopedLock

    // Implementation

    inline const char *UpdateError::what() const noexcept
    {
        if (m_what.empty())
            m_what = helpers::format("Update of key '%s' failed", m_key.c_str());

        return m_what.c_str();
    }

    template<class... Params, class>
    Response<KeyValue> Kv::item(WithHeaders, const std::string& key, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::get))
        http::Status s;
        auto r = m_consul.get(s, keyPath(key),
                              kw::token = m_defaultToken, kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              params...);

        if (s.success())
            return makeResponse(r.headers(), std::move(impl::parseValues(r.data()).at(0)));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.data()));
    }

    template<class... Params, class>
    std::string Kv::get(const std::string& key, const std::string& defaultValue, const Params&... params) const
    {
        const auto kv = item(withHeaders, key, params...);
        if (!kv.data().valid())
            return defaultValue;
        else
            return std::move(kv.data().value);
    }

    template<class... Params, class>
    Response<std::vector<KeyValue>> Kv::items(WithHeaders, const std::string& keyPrefix, const Params&... params) const
    {
        KWARGS_CHECK_IN_LIST(Params, (kv::kw::groups::get))
        http::Status s;
        auto r = m_consul.get(s, keyPath(keyPrefix),
                              kw::token = m_defaultToken, kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              kv::kw::recurse = true, params...);

        if (s.success())
            return makeResponse(r.headers(), impl::parseValues(r.data()));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.data()));
    }

    template<class... Params, class>
    Response<StringList> Kv::get_keys_impl(const std::string& keyPrefix, const Params&... params) const
    {
        http::Status s;
        auto r = m_consul.get(s, keyPath(keyPrefix),
                              kw::token = m_defaultToken, kw::consistency = m_defaultConsistency, kw::dc = m_defaultDc,
                              kv::kw::keys = true, params...);
        if (s.success())
            return makeResponse(r.headers(), impl::parseKeys(r.data()));
        if (NotFoundError::Code == s.code())
            return{ r.headers() };
        throw BadStatus(std::move(s), std::move(r.data()));
    }
}}
