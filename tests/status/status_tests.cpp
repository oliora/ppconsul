//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>

#include "ppconsul/status.h"
#include "test_consul.h"
#include <thread>
#include <algorithm>
#include <boost/optional/optional_io.hpp>


using namespace ppconsul::status;
using ppconsul::CheckStatus;

TEST_CASE("status.leader", "[consul][status][leader]")
{
    auto consul = create_test_consul();
    Status status(consul);

    auto leader = status.leader();
    CHECK(leader == get_test_leader());

    auto elected = status.isLeaderElected();
    CHECK(elected);

    // TBD: Can this method be tested with consul having no leader elected?
    // Expect leader() to return empty string in that case
}

TEST_CASE("status.peers", "[consul][status][peers]")
{
    auto consul = create_test_consul();
    Status status(consul);

    auto peers = status.peers();
    CHECK(peers == std::vector<std::string>{get_test_leader()});
}
