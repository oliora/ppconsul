//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include "ppconsul/kv.h"
#include "test_consul.h"


using ppconsul::kv::Storage;
using ppconsul::kv::KeyValue;

namespace 
{
    auto const Non_Existing_Key = "6DD1E923-71E6-4448-A0B7-57B5F32690E7";
}

TEST_CASE("kv.invalid KeyValue", "[consul][kv]")
{
    KeyValue v;

    CHECK(!v.createIndex());
    CHECK(!v.modifyIndex());
    CHECK(!v.lockIndex());
    CHECK(!v.flags());
    CHECK(v.key() == "");
    CHECK(v.value() == "");
    CHECK(v.session() == "");
    CHECK(!v.valid());
}

TEST_CASE("kv.valid KeyValue", "[consul][kv]")
{
    KeyValue v;
    REQUIRE(!v.valid());
    
    v.m_modifyIndex = 42;
    CHECK(v.valid());
    CHECK(v.modifyIndex() == 42);

    v.m_createIndex = 43;
    v.m_lockIndex = 44;
    v.m_flags = 0xFF123456789;
    v.m_key = "some key";
    v.m_value = "some value";
    v.m_session = "some session";

    CHECK(v.createIndex() == 43);
    CHECK(v.modifyIndex() == 42);
    CHECK(v.lockIndex() == 44);
    CHECK(v.flags() == 0xFF123456789);
    CHECK(v.key() == "some key");
    CHECK(v.value() == "some value");
    CHECK(v.session() == "some session");
}
    
TEST_CASE("kv.erase and count", "[consul][kv]")
{
    auto consul = create_test_consul();
    Storage kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());
    REQUIRE(kv.size() == 0);
    REQUIRE(kv.countAll() == 0);

    SECTION("clear")
    {
        kv.put("key1", "value1");
        kv.put("key2", "value2");

        REQUIRE(!kv.empty());
        REQUIRE(kv.size() == 2);
        REQUIRE(kv.countAll() == 2);

        kv.clear();

        REQUIRE(kv.empty());
        REQUIRE(!kv.size());
        REQUIRE(!kv.countAll());
    }

    SECTION("erase one")
    {
        kv.put("key1", "value1");
        REQUIRE(kv.count("key1"));

        kv.erase("key1");
        REQUIRE(!kv.count("key1"));
    }

    SECTION("erase all")
    {
        kv.put("key1", "value1");
        kv.put("key2", "value2");
        kv.put("key3", "value3");
        kv.put("otherkey1", "othervalue3");
        REQUIRE(kv.count("key1"));
        REQUIRE(kv.count("key2"));
        REQUIRE(kv.count("key3"));
        CHECK(kv.countAll("key") == 3);

        kv.eraseAll("key");
        CHECK(!kv.count("key1"));
        CHECK(!kv.count("key2"));
        CHECK(!kv.count("key3"));
        CHECK(!kv.countAll("key"));
        CHECK(kv.count("otherkey1"));
    }
}

TEST_CASE("kv.get", "[consul][kv]")
{
    auto consul = create_test_consul();
    Storage kv(consul);
    
    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    kv.put("key1", "value1");
    kv.put("key2", "value2");
    kv.put("key3", "value3");
    kv.put("other/Key1", "other/Value1");
    kv.put("other/Key2", "other/Value2");

    REQUIRE(kv.size() == 5);

    SECTION("valid")
    {
        CHECK(kv.get("key1").valid());
        CHECK(kv.get("key2").valid());
        CHECK(kv.get("key3").valid());
        CHECK(kv.get("other/Key1").valid());
        CHECK(kv.get("other/Key2").valid());
    }

    SECTION("get nonexisting")
    {
        REQUIRE(!kv.count(Non_Existing_Key));

        KeyValue v = kv.get(Non_Existing_Key);
        
        CHECK(!v.valid());
        CHECK(!v.createIndex());
        CHECK(!v.modifyIndex());
        CHECK(!v.lockIndex());
        CHECK(!v.flags());
        CHECK(v.value() == "");

        CHECK(kv.get(Non_Existing_Key, "some default value") == "some default value");
    }

    SECTION("get")
    {
        KeyValue v = kv.get("key1");

        REQUIRE(v.valid());

        CHECK(v.createIndex());
        CHECK(v.modifyIndex());
        CHECK(!v.lockIndex());
        CHECK(!v.flags());
        CHECK(v.key() == "key1");
        CHECK(v.value() == "value1");
        CHECK(v.session() == "");

        CHECK(kv.get("key1", "some default value") == "value1");
    }

    SECTION("get with headers")
    {
        ppconsul::Response<KeyValue> v = kv.get(ppconsul::withHeaders, "key1");

        REQUIRE(v.value().valid());

        CHECK(v.value().createIndex());
        CHECK(v.value().modifyIndex());
        CHECK(!v.value().lockIndex());
        CHECK(!v.value().flags());
        CHECK(v.value().key() == "key1");
        CHECK(v.value().value() == "value1");
        CHECK(v.value().session() == "");

        CHECK(v.headers().valid());
        CHECK(v.headers().index());
        CHECK(v.headers().knownLeader());
        CHECK(v.headers().lastContact() == std::chrono::milliseconds(0));
    }

    SECTION("getAll")
    {
        CHECK(kv.getAll(Non_Existing_Key).size() == 0);
        CHECK(kv.getAll().size() == 5);
        CHECK(kv.getAll("other/Key").size() == 2);

        std::vector<KeyValue> v = kv.getAll("key");

        REQUIRE(3 == v.size());

        CHECK(v[0].createIndex());
        CHECK(v[0].modifyIndex());
        CHECK(!v[0].lockIndex());
        CHECK(!v[0].flags());
        CHECK(v[0].key() == "key1");
        CHECK(v[0].value() == "value1");
        CHECK(v[0].session() == "");

        CHECK(v[1].createIndex());
        CHECK(v[1].modifyIndex());
        CHECK(!v[1].lockIndex());
        CHECK(!v[1].flags());
        CHECK(v[1].key() == "key2");
        CHECK(v[1].value() == "value2");
        CHECK(v[1].session() == "");

        CHECK(v[2].createIndex());
        CHECK(v[2].modifyIndex());
        CHECK(!v[2].lockIndex());
        CHECK(!v[2].flags());
        CHECK(v[2].key() == "key3");
        CHECK(v[2].value() == "value3");
        CHECK(v[2].session() == "");
    }

    // TODO: getAll with headers and getAllKeys with headers

    SECTION("getAllKeys")
    {
        CHECK(kv.getAllKeys(Non_Existing_Key) == std::vector<std::string>());
        CHECK(kv.getAllKeys("key") == std::vector<std::string>({"key1", "key2", "key3"}));
        CHECK(kv.getAllKeys("other/Key") == std::vector<std::string>({ "other/Key1", "other/Key2" }));
        CHECK(kv.getSubKeys("", "/") == std::vector<std::string>({ "key1", "key2", "key3", "other/" }));
        CHECK(kv.getSubKeys("", "e") == std::vector<std::string>({ "ke", "othe" }));
    }
}

TEST_CASE("kv.put", "[consul][kv]")
{
    auto consul = create_test_consul();
    Storage kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    SECTION("put")
    {
        kv.put("key42", "value31");
        KeyValue v = kv.get("key42");
        REQUIRE(v.valid());
        CHECK(v.createIndex());
        CHECK(v.modifyIndex());
        CHECK(!v.lockIndex());
        CHECK(v.flags() == 0);
        CHECK(v.key() == "key42");
        CHECK(v.value() == "value31");
        CHECK(v.session() == "");
    }

    SECTION("put flags")
    {
        kv.put("key24", "value13", 0x12345678);

        {
            KeyValue v = kv.get("key24");
            REQUIRE(v.valid());
            CHECK(v.createIndex());
            CHECK(v.modifyIndex());
            CHECK(!v.lockIndex());
            CHECK(v.flags() == 0x12345678);
            CHECK(v.value() == "value13");
            CHECK(v.key() == "key24");
            CHECK(v.session() == "");
        }

        kv.put("key24", "value14");
        CHECK(kv.get("key24").value() == "value14");
        // Feature or bug of Consul: the flags are reseted after put without "?flags" specified
        //CHECK(kv.get("key24").flags() == 0x12345678);
    }
}

TEST_CASE("kv.checkAndSet", "[consul][kv]")
{
    auto consul = create_test_consul();
    Storage kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    SECTION("checkAndSet")
    {
        SECTION("change nonexisting")
        {
            REQUIRE(!kv.checkAndSet("key2", 1, "value2"));
            CHECK(!kv.count("key2"));
            CHECK(!kv.get("key2").valid());
        }

        SECTION("init without cas")
        {
            kv.put("key1", "value1");
            
            {
                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value1");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }

            REQUIRE(kv.get("key1").valid());
            REQUIRE(kv.get("key1").value() == "value1");
            REQUIRE(kv.get("key1").flags() == 0);

            SECTION("change with cas wrong")
            {
                REQUIRE(!kv.checkAndSet("key1", 0, "value2"));
                CHECK(kv.get("key1").valid());
                CHECK(kv.get("key1").value() == "value1");
                CHECK(kv.get("key1").flags() == 0);
            }

            SECTION("change with cas right")
            {
                auto cas = kv.get("key1").modifyIndex();

                REQUIRE(kv.checkAndSet("key1", cas, "value2"));

                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value2");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }
        }

        SECTION("init with cas")
        {
            REQUIRE(kv.checkAndSet("key1", 0, "value1"));
            CHECK(kv.get("key1").value() == "value1");
            CHECK(kv.get("key1").valid());
            CHECK(kv.get("key1").flags() == 0);

            SECTION("change with put")
            {
                kv.put("key1", "value2");

                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value2");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }

            SECTION("change with cas")
            {
                auto cas = kv.get("key1").modifyIndex();

                REQUIRE(kv.checkAndSet("key1", cas, "value2"));

                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value2");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }
        }
    }

    SECTION("checkAndSet flags")
    {
        SECTION("change nonexisting")
        {
            REQUIRE(!kv.checkAndSet("key2", 1, "value2", 0x87654321));
            CHECK(!kv.count("key2"));
            CHECK(!kv.get("key2").valid());
        }

        SECTION("init without cas")
        {
            kv.put("key1", "value1", 0x87654321);

            {
                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value1");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0x87654321);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }

            SECTION("change with cas wrong")
            {
                REQUIRE(!kv.checkAndSet("key1", 0, "value2", 0xFC12DE56));
                CHECK(kv.get("key1").valid());
                CHECK(kv.get("key1").value() == "value1");
                CHECK(kv.get("key1").flags() == 0x87654321);
            }

            SECTION("change with cas right")
            {
                auto cas = kv.get("key1").modifyIndex();

                REQUIRE(kv.checkAndSet("key1", cas, "value2", 0xFC12DE56));

                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value2");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0xFC12DE56);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }
        }

        SECTION("init with cas")
        {
            REQUIRE(kv.checkAndSet("key1", 0, "value1", 0x87654321));
            
            {
                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value1");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0x87654321);
                CHECK(v.key() == "key1");
                CHECK(v.session() == ""); 
            }

            SECTION("change with put")
            {
                kv.put("key1", "value2", 0xFC12DE56);
                
                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value2");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0xFC12DE56);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }

            SECTION("change with put value only")
            {
                kv.put("key1", "value2");

                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value2");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                // Feature or bug of Consul: the flags are reseted after put without "?flags" specified
                // CHECK(v.flags() == 0x87654321);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }

            SECTION("change with cas")
            {
                auto cas = kv.get("key1").modifyIndex();

                REQUIRE(kv.checkAndSet("key1", cas, "value2", 0xFC12DE56));

                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value2");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                CHECK(v.flags() == 0xFC12DE56);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }

            SECTION("change with cas value only")
            {
                auto cas = kv.get("key1").modifyIndex();

                REQUIRE(kv.checkAndSet("key1", cas, "value2"));

                KeyValue v = kv.get("key1");
                REQUIRE(v.valid());
                CHECK(v.value() == "value2");
                CHECK(v.createIndex());
                CHECK(v.modifyIndex());
                CHECK(!v.lockIndex());
                // Feature or bug of Consul: the flags are reseted after put without "?flags" specified
                // CHECK(v.flags() == 0x87654321);
                CHECK(v.key() == "key1");
                CHECK(v.session() == "");
            }
        }
    }
}

TEST_CASE("kv.special chars", "[consul][kv][special chars]")
{
    auto consul = create_test_consul();
    Storage kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    SECTION("get1")
    {
        kv.put("key{1}/&23\x03", "value1");
        KeyValue v = kv.get("key{1}/&23\x03");
        REQUIRE(v.valid());
        CHECK(v.key() == "key{1}/&23\x03");
        CHECK(v.value() == "value1");
    }
    
    SECTION("get2")
    {
        const auto key = std::string("key\x0-1-\x0", 8);
        kv.put(key, "value2");
        KeyValue v = kv.get(key);
        REQUIRE(v.valid());
        CHECK(v.key() == key);
        CHECK(v.value() == "value2");
    }

    SECTION("getSubKeys1")
    {
        kv.put("key{1}\x2-1\x2&2-\x03", "value1");
        kv.put("key{1}\x2-2\x2&2-\x04", "value2");
        kv.put("key{1}\x2-3\x2&2-\x05", "value3");
        kv.put("key{1}\x2-3\x2&2-\x06", "value4");
        REQUIRE(kv.size() == 4);

        auto keys = kv.getSubKeys("key{1}\x2", "\x2");
        REQUIRE(keys.size() == 3);
        CHECK(keys[0] == "key{1}\x2-1\x2");
        CHECK(keys[1] == "key{1}\x2-2\x2");
        CHECK(keys[2] == "key{1}\x2-3\x2");
    }

    SECTION("getSubKeys2")
    {
        const auto keyPart = std::string("key{1}\r\n\t\x0", 10);
        const auto null = std::string("\x0", 1);

        kv.put(keyPart + "-1\t" + null + "&2-\x03", "value1");
        kv.put(keyPart + "-2" + null + "&2-\x04", "value2");
        kv.put(keyPart + "-3" + null + "&2-\x05", "value3");
        kv.put(keyPart + "-3" + null + "&2-\x06", "value4");
        REQUIRE(kv.size() == 4);

        auto keys = kv.getSubKeys(keyPart, null);
        REQUIRE(keys.size() == 3);
        CHECK(keys[0] == keyPart + "-1\t" + null);
        CHECK(keys[1] == keyPart + "-2" + null);
        CHECK(keys[2] == keyPart + "-3" + null);
    }

    SECTION("value1")
    {
        const auto value = std::string("\x2-1\x2&2-\x03", 8);

        kv.put("key1", value);

        auto v = kv.get("key1");
        CHECK(v.valid());
        CHECK(v.value() == value);
    }

    SECTION("value2")
    {
        const auto value = std::string("&=\r\n\t 1\x0 -2\x0", 12);

        kv.put("key2", value);

        auto v = kv.get("key2");
        CHECK(v.valid());
        CHECK(v.value() == value);
    }
}

