//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>

#include "ppconsul/kv.h"
#include "ppconsul/sessions.h"
#include "test_consul.h"
#include <chrono>
#include <thread>


using namespace ppconsul::kv;
using ppconsul::StringList;


namespace 
{
    auto const Non_Existing_Key = "6DD1E923-71E6-4448-A0B7-57B5F32690E7";
}

TEST_CASE("kv.invalid KeyValue", "[consul][kv]")
{
    KeyValue v;

    CHECK(!v.createIndex);
    CHECK(!v.modifyIndex);
    CHECK(!v.lockIndex);
    CHECK(!v.flags);
    CHECK(v.key == "");
    CHECK(v.value == "");
    CHECK(v.session == "");
    CHECK(!v.valid());
}

TEST_CASE("kv.valid KeyValue", "[consul][kv]")
{
    KeyValue v;
    REQUIRE(!v.valid());
    
    v.modifyIndex = 42;
    CHECK(v.valid());
    CHECK(v.modifyIndex == 42);

    v.createIndex = 43;
    v.lockIndex = 44;
    v.flags = 0xFF123456789;
    v.key = "some key";
    v.value = "some value";
    v.session = "some session";

    CHECK(v.createIndex == 43);
    CHECK(v.modifyIndex == 42);
    CHECK(v.lockIndex == 44);
    CHECK(v.flags == 0xFF123456789);
    CHECK(v.key == "some key");
    CHECK(v.value == "some value");
    CHECK(v.session == "some session");
}
    
TEST_CASE("kv.erase and count", "[consul][kv]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());
    REQUIRE(kv.size() == 0);
    REQUIRE(kv.countAll() == 0);

    SECTION("clear")
    {
        kv.set("key1", "value1");
        kv.set("key2", "value2");

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
        kv.set("key1", "value1");
        REQUIRE(kv.count("key1"));

        kv.erase("key1");
        REQUIRE(!kv.count("key1"));
    }

    SECTION("erase all")
    {
        kv.set("key1", "value1");
        kv.set("key2", "value2");
        kv.set("key3", "value3");
        kv.set("otherkey1", "othervalue3");
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
/**/
TEST_CASE("kv.get", "[consul][kv][headers]")
{
    auto consul = create_test_consul();
    Kv kv(consul);
    
    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    kv.set("key1", "value1");
    kv.set("key2", "value2");
    kv.set("key3", "value3");
    kv.set("other/Key1", "other/Value1");
    kv.set("other/Key2", "other/Value2");

    REQUIRE(kv.size() == 5);

    SECTION("valid")
    {
        CHECK(kv.item("key1").valid());
        CHECK(kv.item("key2").valid());
        CHECK(kv.item("key3").valid());
        CHECK(kv.item("other/Key1").valid());
        CHECK(kv.item("other/Key2").valid());
    }

    SECTION("get nonexisting item")
    {
        REQUIRE(!kv.count(Non_Existing_Key));

        KeyValue v = kv.item(Non_Existing_Key);
        
        CHECK(!v.valid());
        CHECK(!v.createIndex);
        CHECK(!v.modifyIndex);
        CHECK(!v.lockIndex);
        CHECK(!v.flags);
        CHECK(v.value == "");

        CHECK(kv.get(Non_Existing_Key, "some default value") == "some default value");
    }

    SECTION("get")
    {
        KeyValue v = kv.item("key1");

        REQUIRE(v.valid());

        CHECK(v.createIndex);
        CHECK(v.modifyIndex);
        CHECK(!v.lockIndex);
        CHECK(!v.flags);
        CHECK(v.key == "key1");
        CHECK(v.value == "value1");
        CHECK(v.session == "");

        CHECK(kv.get("key1", "some default value") == "value1");
    }

    SECTION("get item with headers")
    {
        ppconsul::Response<KeyValue> v0 = kv.item(ppconsul::withHeaders, Non_Existing_Key);

        CHECK(!v0.data().valid());
        CHECK(v0.headers().valid());
        CHECK(v0.headers().index());
        CHECK(v0.headers().knownLeader());
        CHECK(v0.headers().lastContact() == std::chrono::milliseconds(0));

        ppconsul::Response<KeyValue> v1 = kv.item(ppconsul::withHeaders, "key1");

        REQUIRE(v1.data().valid());

        CHECK(v1.data().createIndex);
        CHECK(v1.data().modifyIndex);
        CHECK(!v1.data().lockIndex);
        CHECK(!v1.data().flags);
        CHECK(v1.data().key == "key1");
        CHECK(v1.data().value == "value1");
        CHECK(v1.data().session == "");

        CHECK(v1.headers().valid());
        CHECK(v1.headers().index());
        CHECK(v1.headers().knownLeader());
        CHECK(v1.headers().index() == v1.data().modifyIndex);
        CHECK(v1.headers().lastContact() == std::chrono::milliseconds(0));
    }

    SECTION("get items")
    {
        CHECK(kv.items(Non_Existing_Key).size() == 0);
        CHECK(kv.items().size() == 5);
        CHECK(kv.items("other/Key").size() == 2);

        std::vector<KeyValue> v = kv.items("key");

        REQUIRE(3 == v.size());

        CHECK(v[0].createIndex);
        CHECK(v[0].modifyIndex);
        CHECK(!v[0].lockIndex);
        CHECK(!v[0].flags);
        CHECK(v[0].key == "key1");
        CHECK(v[0].value == "value1");
        CHECK(v[0].session == "");

        CHECK(v[1].createIndex);
        CHECK(v[1].modifyIndex);
        CHECK(!v[1].lockIndex);
        CHECK(!v[1].flags);
        CHECK(v[1].key == "key2");
        CHECK(v[1].value == "value2");
        CHECK(v[1].session == "");

        CHECK(v[2].createIndex);
        CHECK(v[2].modifyIndex);
        CHECK(!v[2].lockIndex);
        CHECK(!v[2].flags);
        CHECK(v[2].key == "key3");
        CHECK(v[2].value == "value3");
        CHECK(v[2].session == "");
    }

    SECTION("get items with headers")
    {
        ppconsul::Response<std::vector<KeyValue>> v0 = kv.items(ppconsul::withHeaders, Non_Existing_Key);
        
        CHECK(!v0.data().size());
        CHECK(v0.headers().valid());
        CHECK(v0.headers().index());
        CHECK(v0.headers().knownLeader());
        CHECK(v0.headers().lastContact() == std::chrono::milliseconds(0));

        ppconsul::Response<std::vector<KeyValue>> v1 = kv.items(ppconsul::withHeaders, "key");

        REQUIRE(3 == v1.data().size());

        const auto maxModifyIndex = std::max_element(v1.data().begin(), v1.data().end(), [](const KeyValue&l, const KeyValue& r) {
            return l.modifyIndex < r.modifyIndex;
        })->modifyIndex;

        CHECK(v1.headers().valid());
        CHECK(v1.headers().index());
        CHECK(v1.headers().knownLeader());
        CHECK(v1.headers().index() == maxModifyIndex);
        CHECK(v1.headers().lastContact() == std::chrono::milliseconds(0));
    }

    SECTION("get keys")
    {
        CHECK(kv.keys(Non_Existing_Key) == StringList());
        CHECK(kv.subKeys(Non_Existing_Key, "/") == StringList());
        CHECK(kv.keys("key") == StringList({"key1", "key2", "key3"}));
        CHECK(kv.keys("other/Key") == StringList({ "other/Key1", "other/Key2" }));
        CHECK(kv.subKeys("", "/") == StringList({ "key1", "key2", "key3", "other/" }));
        CHECK(kv.subKeys("", "e") == StringList({ "ke", "othe" }));
    }

    SECTION("get keys with headers")
    {
        ppconsul::Response<StringList> v0 = kv.keys(ppconsul::withHeaders, Non_Existing_Key);
        CHECK(v0.data() == StringList());
        CHECK(v0.headers().valid());
        CHECK(v0.headers().index());
        CHECK(v0.headers().knownLeader());
        CHECK(v0.headers().lastContact() == std::chrono::milliseconds(0));

        ppconsul::Response<StringList> v1 = kv.keys(ppconsul::withHeaders, "key");
        CHECK(v1.data() == StringList({ "key1", "key2", "key3" }));
        CHECK(v1.headers().valid());
        CHECK(v1.headers().index());
        CHECK(v1.headers().knownLeader());
        CHECK(v1.headers().lastContact() == std::chrono::milliseconds(0));

        ppconsul::Response<StringList> v2 = kv.subKeys(ppconsul::withHeaders, Non_Existing_Key, "/");
        CHECK(v2.data() == StringList());
        CHECK(v2.headers().valid());
        CHECK(v2.headers().index());
        CHECK(v2.headers().knownLeader());
        CHECK(v2.headers().lastContact() == std::chrono::milliseconds(0));

        ppconsul::Response<StringList> v3 = kv.subKeys(ppconsul::withHeaders, "", "/");
        CHECK(v3.data() == StringList({ "key1", "key2", "key3", "other/" }));
        CHECK(v3.headers().valid());
        CHECK(v3.headers().index());
        CHECK(v3.headers().knownLeader());
        CHECK(v3.headers().lastContact() == std::chrono::milliseconds(0));
    }
}

TEST_CASE("kv.set", "[consul][kv]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    SECTION("put")
    {
        kv.set("key42", "value31");
        KeyValue v = kv.item("key42");
        REQUIRE(v.valid());
        CHECK(v.createIndex);
        CHECK(v.modifyIndex);
        CHECK(!v.lockIndex);
        CHECK(v.flags == 0);
        CHECK(v.key == "key42");
        CHECK(v.value == "value31");
        CHECK(v.session == "");
    }

    SECTION("put flags")
    {
        kv.set("key24", "value13", kw::flags=0x12345678);

        {
            KeyValue v = kv.item("key24");
            REQUIRE(v.valid());
            CHECK(v.createIndex);
            CHECK(v.modifyIndex);
            CHECK(!v.lockIndex);
            CHECK(v.flags == 0x12345678);
            CHECK(v.value == "value13");
            CHECK(v.key == "key24");
            CHECK(v.session == "");
        }

        kv.set("key24", "value14");
        CHECK(kv.item("key24").value == "value14");
        // Feature or bug of Consul: the flags are reseted after put without "?flags" specified
        //CHECK(kv.item("key24").flags == 0x12345678);
    }
}

TEST_CASE("kv.compareSet", "[consul][kv]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    SECTION("cas")
    {
        SECTION("change nonexisting")
        {
            REQUIRE(!kv.compareSet("key2", 1, "value2"));
            CHECK(!kv.count("key2"));
            CHECK(!kv.item("key2").valid());
        }

        SECTION("init without cas")
        {
            kv.set("key1", "value1");
            
            {
                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value1");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }

            REQUIRE(kv.item("key1").valid());
            REQUIRE(kv.item("key1").value == "value1");
            REQUIRE(kv.item("key1").flags == 0);

            SECTION("change with cas wrong")
            {
                REQUIRE(!kv.compareSet("key1", 0, "value2"));
                CHECK(kv.item("key1").valid());
                CHECK(kv.item("key1").value == "value1");
                CHECK(kv.item("key1").flags == 0);
            }

            SECTION("change with cas right")
            {
                auto cas = kv.item("key1").modifyIndex;

                REQUIRE(kv.compareSet("key1", cas, "value2"));

                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value2");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }
        }

        SECTION("init with cas")
        {
            REQUIRE(kv.compareSet("key1", 0, "value1"));
            CHECK(kv.item("key1").value == "value1");
            CHECK(kv.item("key1").valid());
            CHECK(kv.item("key1").flags == 0);

            SECTION("change with put")
            {
                kv.set("key1", "value2");

                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value2");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }

            SECTION("change with cas")
            {
                auto cas = kv.item("key1").modifyIndex;

                REQUIRE(kv.compareSet("key1", cas, "value2"));

                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value2");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }
        }
    }

    SECTION("cas flags")
    {
        SECTION("change nonexisting")
        {
            REQUIRE(!kv.compareSet("key2", 1, "value2", kw::flags = 0x87654321));
            CHECK(!kv.count("key2"));
            CHECK(!kv.item("key2").valid());
        }

        SECTION("init without cas")
        {
            kv.set("key1", "value1", kw::flags = 0x87654321);

            {
                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value1");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0x87654321);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }

            SECTION("change with cas wrong")
            {
                REQUIRE(!kv.compareSet("key1", 0, "value2", kw::flags = 0xFC12DE56));
                CHECK(kv.item("key1").valid());
                CHECK(kv.item("key1").value == "value1");
                CHECK(kv.item("key1").flags == 0x87654321);
            }

            SECTION("change with cas right")
            {
                auto cas = kv.item("key1").modifyIndex;

                REQUIRE(kv.compareSet("key1", cas, "value2", kw::flags = 0xFC12DE56));

                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value2");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0xFC12DE56);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }
        }

        SECTION("init with cas")
        {
            REQUIRE(kv.compareSet("key1", 0, "value1", kw::flags = 0x87654321));
            
            {
                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value1");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0x87654321);
                CHECK(v.key == "key1");
                CHECK(v.session == ""); 
            }

            SECTION("change with put")
            {
                kv.set("key1", "value2", kw::flags = 0xFC12DE56);
                
                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value2");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0xFC12DE56);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }

            SECTION("change with put value only")
            {
                kv.set("key1", "value2");

                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value2");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                // Feature or bug of Consul: the flags are reseted after put without "?flags" specified
                // CHECK(v.flags == 0x87654321);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }

            SECTION("change with cas")
            {
                auto cas = kv.item("key1").modifyIndex;

                REQUIRE(kv.compareSet("key1", cas, "value2", kw::flags = 0xFC12DE56));

                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value2");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                CHECK(v.flags == 0xFC12DE56);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }

            SECTION("change with cas value only")
            {
                auto cas = kv.item("key1").modifyIndex;

                REQUIRE(kv.compareSet("key1", cas, "value2"));

                KeyValue v = kv.item("key1");
                REQUIRE(v.valid());
                CHECK(v.value == "value2");
                CHECK(v.createIndex);
                CHECK(v.modifyIndex);
                CHECK(!v.lockIndex);
                // Feature or bug of Consul: the flags are reseted after put without "?flags" specified
                // CHECK(v.flags == 0x87654321);
                CHECK(v.key == "key1");
                CHECK(v.session == "");
            }
        }
    }
}

TEST_CASE("kv.compareErase", "[consul][kv]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    kv.set("key1", "bla");
    KeyValue v = kv.item("key1");
    REQUIRE(v);

    SECTION("zero index does nothing")
    {
        REQUIRE(!kv.compareErase("key1", 0));
        KeyValue v2 = kv.item("key1");
        REQUIRE(v2);
        REQUIRE(v2.modifyIndex == v.modifyIndex);
        REQUIRE(v2.value == "bla");
    }

    SECTION("correct index")
    {
        REQUIRE(kv.compareErase("key1", v.modifyIndex));
        REQUIRE(kv.count("key1") == 0);
    }

    SECTION("incorrect index")
    {
        kv.set("key1", "bla2");
        KeyValue v1 = kv.item("key1");
        REQUIRE(v1);
        REQUIRE(v1.modifyIndex != v.modifyIndex);

        REQUIRE(!kv.compareErase("key1", v.modifyIndex));

        KeyValue v2 = kv.item("key1");
        REQUIRE(v2);
        REQUIRE(v2.modifyIndex == v1.modifyIndex);
        REQUIRE(v2.value == "bla2");
    }
}


TEST_CASE("kv.special chars", "[consul][kv][special chars]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    // Start from blank KV storage
    kv.clear();
    REQUIRE(kv.empty());

    SECTION("value1")
    {
        const auto value = std::string("\x2-1\x2&2-\x03", 8);

        kv.set("key1", value);

        auto v = kv.item("key1");
        CHECK(v.valid());
        CHECK(v.value == value);
    }

    SECTION("value2")
    {
        const auto value = std::string("&=\r\n\t 1\x0 -2\x0", 12);

        kv.set("key2", value);

        auto v = kv.item("key2");
        CHECK(v.valid());
        CHECK(v.value == value);
    }
}

TEST_CASE("kv.index", "[consul][kv][headers]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    kv.erase("key1");
    REQUIRE(!kv.count("key1"));

    // Check that getting an item does not change the item's modify-index
    auto modifyIndex1 = kv.items(ppconsul::withHeaders).headers().index();
    CHECK(kv.items(ppconsul::withHeaders).headers().index() == modifyIndex1);
    
    kv.set("key1", "value1");
    auto modifyIndex2 = kv.items(ppconsul::withHeaders).headers().index();
    CHECK(modifyIndex2 > modifyIndex1);
    
    // Check that erasing of an item changes the root's modify-index (doesn't work before Consul 0.5.0)
    kv.erase("key1");
    CHECK(kv.items(ppconsul::withHeaders).headers().index() > modifyIndex2);
}

TEST_CASE("kv.blocking-query", "[consul][kv][blocking]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    kv.set("key1", "value1");
    auto index1 = kv.item(ppconsul::withHeaders, "key1").headers().index();

    auto t1 = std::chrono::steady_clock::now();
    auto resp1 = kv.item(ppconsul::withHeaders, "key1", kw::block_for = {std::chrono::seconds(5), index1});
    CHECK((std::chrono::steady_clock::now() - t1) >= std::chrono::seconds(5));
    CHECK(index1 == resp1.headers().index());
    CHECK(resp1.data().value == "value1");

    kv.set("key1", "value2");
    auto t2 = std::chrono::steady_clock::now();
    auto resp2 = kv.item(ppconsul::withHeaders, "key1", kw::block_for = {std::chrono::seconds(5), index1});
    CHECK((std::chrono::steady_clock::now() - t2) < std::chrono::seconds(2));
    CHECK(index1 != resp2.headers().index());
    CHECK(resp2.data().value == "value2");
}

TEST_CASE("kv.abort-blocking-query", "[consul][kv][blocking]")
{
    auto consul = create_test_consul(ppconsul::kw::enable_stop = true);
    Kv kv(consul);

    kv.set("key1", "value1");
    auto index1 = kv.item(ppconsul::withHeaders, "key1").headers().index();

    const auto t1 = std::chrono::steady_clock::now();
    std::exception_ptr exception;
    auto monitorThread = std::thread(
        [&]()
        {
            try
            {
                kv.item(ppconsul::withHeaders, "key1", kw::block_for = {std::chrono::seconds(5), index1});
            }
            catch (const std::exception&)
            {
                exception = std::current_exception();
            }
        });

    consul.stop();
    monitorThread.join();
    CHECK((std::chrono::steady_clock::now() - t1) < std::chrono::seconds(1));
    CHECK_THROWS_AS(std::rethrow_exception(exception), ppconsul::OperationAborted);
}

TEST_CASE("kv.quick-block-query", "[consul][kv][blocking]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    kv.set("key1", "value1");
    auto index1 = kv.item(ppconsul::withHeaders, "key1").headers().index();

    auto t1 = std::chrono::steady_clock::now();
    auto resp1 = kv.item(ppconsul::withHeaders, "key1", kw::block_for = {std::chrono::milliseconds(500), index1});
    CHECK((std::chrono::steady_clock::now() - t1) >= std::chrono::milliseconds(500));
    CHECK(index1 == resp1.headers().index());
    CHECK(resp1.data().value == "value1");

    kv.set("key1", "value2");
    auto t2 = std::chrono::steady_clock::now();
    auto resp2 = kv.item(ppconsul::withHeaders, "key1", kw::block_for = {std::chrono::milliseconds(500), index1});
    CHECK((std::chrono::steady_clock::now() - t2) < std::chrono::milliseconds(500));
    CHECK(index1 != resp2.headers().index());
    CHECK(resp2.data().value == "value2");
}

TEST_CASE("kv.lock_unlock", "[consul][kv][session]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    kv.erase("key1");
    REQUIRE(!kv.count("key1"));

    ppconsul::sessions::Sessions sessions(consul);

    auto session1 = sessions.create();
    auto session2 = sessions.create();

    SECTION("successful lock-unlock")
    {
        SECTION("existing value")
        {
            kv.set("key1", "bla");
        }

        SECTION("nonexisting value")
        {
            REQUIRE(!kv.count("key1"));
        }

        REQUIRE(kv.lock("key1", session1, "test1"));

        auto v = kv.item("key1");
        REQUIRE(v);
        REQUIRE(v.session == session1);
        REQUIRE(v.value == "test1");

        REQUIRE(kv.unlock("key1", session1, "test2"));

        v = kv.item("key1");
        REQUIRE(v);
        REQUIRE(v.session == "");
        REQUIRE(v.value == "test2");
    }

    SECTION("lock-unlock already locked")
    {
        REQUIRE(kv.lock("key1", session1, "test1"));
        REQUIRE(!kv.lock("key1", session2, "test2"));

        auto v = kv.item("key1");
        REQUIRE(v);
        REQUIRE(v.session == session1);
        REQUIRE(v.value == "test1");

        REQUIRE(!kv.unlock("key1", session2, "test3"));

        v = kv.item("key1");
        REQUIRE(v);
        REQUIRE(v.session == session1);
        REQUIRE(v.value == "test1");
    }

    // TODO: add more tests
}

TEST_CASE("kv.transactions", "[consul][kv][transactions]")
{
    auto consul = create_test_consul();
    Kv kv(consul);

    SECTION("empty transaction")
    {
        auto v = kv.commit({});
        REQUIRE(v.empty());
    }

    SECTION("simple operations")
    {
        auto v = kv.commit({
            txn_ops::Set{"key1", "one", 42},
            txn_ops::Get{"key1"},
            txn_ops::Erase{"key1"},
            txn_ops::CheckNotExists{"key1"},
        });

        REQUIRE(v.size() == 2);

        CHECK(v[0].key == "key1");
        CHECK(v[0].flags == 42);
        CHECK(v[0].value == "");

        CHECK(v[1].key == "key1");
        CHECK(v[1].flags == 42);
        CHECK(v[1].value == "one");
    }

    SECTION("index-checking operations")
    {
        auto v1 = kv.commit({
            txn_ops::Set{"key1", "ichi", 111},
            txn_ops::Set{"key2", "ni",   222},
            txn_ops::Set{"key3", "san",  333},
        });
        REQUIRE(v1.size() == 3);

        CHECK(v1[0].key == "key1");
        CHECK(v1[0].flags == 111);
        CHECK(v1[0].value == "");

        CHECK(v1[1].key == "key2");
        CHECK(v1[1].flags == 222);
        CHECK(v1[1].value == "");

        CHECK(v1[2].key == "key3");
        CHECK(v1[2].flags == 333);
        CHECK(v1[2].value == "");

        auto v2 = kv.commit({
            txn_ops::CompareSet{"key1", v1[0].modifyIndex, "x", 27},
            txn_ops::CompareErase{"key2", v1[1].modifyIndex},
            txn_ops::CheckIndex{"key3", v1[2].modifyIndex},
        });
        REQUIRE(v2.size() == 2);

        CHECK(v2[0].key == "key1");
        CHECK(v2[0].flags == 27);
        CHECK(v2[0].value == "");

        CHECK(v2[1].key == "key3");
        CHECK(v2[1].flags == 333);
        CHECK(v2[1].value == "");

        kv.erase("key1");
        kv.erase("key3");
    }

    SECTION("tree operations")
    {
        auto v1 = kv.commit({
            txn_ops::Set{"key1", "ichi", 123},
            txn_ops::Set{"key2", "ni",   456},
            txn_ops::Set{"key3", "san",  789},
        });
        REQUIRE(v1.size() == 3);

        auto v2 = kv.commit({
            txn_ops::GetAll{"key"},
            txn_ops::EraseAll{"key"},
        });
        REQUIRE(v2.size() == 3);

        CHECK(v2[0].key == "key1");
        CHECK(v2[0].flags == 123);
        CHECK(v2[0].value == "ichi");

        CHECK(v2[1].key == "key2");
        CHECK(v2[1].flags == 456);
        CHECK(v2[1].value == "ni");

        CHECK(v2[2].key == "key3");
        CHECK(v2[2].flags == 789);
        CHECK(v2[2].value == "san");
    }

    SECTION("transaction errors")
    {
        REQUIRE_THROWS_AS(
            kv.commit({txn_ops::Get{"key1"}}),
            TxnAborted
        );
    }
}
