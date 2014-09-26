#pragma once

#include "client.h"


namespace ppconsul { namespace kv {

    struct KeyValue
    {
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


    class Storage
    {
    public:
        explicit Storage(Client& client, const std::string& separator = "/")
        : m_client(client)
        , m_separator(separator)
        {}

        bool exists(const std::string& key, Consistency cons = Consistency::Default)
        {
            return get(key, cons).valid();
        }

        // Returns invalid KeyValue (i.e. !kv.valid()) if key does not exist
        KeyValue get(const std::string& key, Consistency cons = Consistency::Default);

        // Returns defaultValue if key does not exist
        std::string get(const std::string& key, const std::string& defaultValue, Consistency cons = Consistency::Default)
        {
            const auto kv = get(key, cons);
            if (!kv.valid())
                return defaultValue;
            else
                return kv.m_value;
        }

        // Returns empty vector if no keys found
        std::vector<KeyValue> getAll(const std::string& keyPrefix, Consistency cons = Consistency::Default);

        // Returns empty vector if no keys found
        std::vector<std::string> getKeys(const std::string& keyPrefix, Consistency cons = Consistency::Default);

        // Returns empty vector if no keys found
        std::vector<std::string> getAllKeys(const std::string& keyPrefix, Consistency cons = Consistency::Default);

        void put(const std::string& key, const std::string& value, uint64_t flags = 0);

        // Returns true if value was successfully set and false otherwise
        bool checkAndSet(const std::string& key, uint64_t cas, const std::string& value, uint64_t flags = 0);

        // TODO: acquire/release session

        void erase(const std::string& key);
        void eraseAll(const std::string& keyPrefix);

    private:
        Client& m_client;
        std::string m_separator;
    };

}}
