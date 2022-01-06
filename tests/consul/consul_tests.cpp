//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>

#include "ppconsul/consul.h"
#include "ppconsul/kv.h"
#include "test_consul.h"
#include <chrono>
#include <thread>


using namespace ppconsul;


TEST_CASE("consul.request_timeout", "[consul][http]")
{
    auto consul = create_test_consul(kw::request_timeout = std::chrono::milliseconds{10});
    kv::Kv kv(consul);

    kv.set("key1", "value1");
    auto index1 = kv.item(ppconsul::withHeaders, "key1").headers().index();

    auto t1 = std::chrono::steady_clock::now();
    REQUIRE_THROWS_AS(kv.item(ppconsul::withHeaders, "key1", kw::block_for = {std::chrono::milliseconds(500), index1}), RequestTimedOut);
    auto time = std::chrono::steady_clock::now() - t1;
    CHECK(time >= std::chrono::milliseconds{10});
    CHECK(time < std::chrono::milliseconds{100});  // Timeout mechanism is not guaranteed to be very precise
}

TEST_CASE("consul.connect_timeout", "[consul][http]")
{
    // This test may fail if libCURL is not compiled with asynchronous DNS resolve library like c-ares

    Consul consul{"http://10.255.255.1:81", kw::connect_timeout = std::chrono::milliseconds{2}};

    auto t1 = std::chrono::steady_clock::now();
    REQUIRE_THROWS_AS(consul.get("/"), RequestTimedOut);
    auto time = std::chrono::steady_clock::now() - t1;
    CHECK(time >= std::chrono::milliseconds{2});
    CHECK(time < std::chrono::milliseconds{100});  // Timeout mechanism is not guaranteed to be very precise
}
