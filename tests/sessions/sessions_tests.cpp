//  Copyright (c) 2019 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>
#include "ppconsul/sessions.h"
#include "test_consul.h"

using namespace ppconsul::sessions;

TEST_CASE("sessions.basic-management", "[consul][sessions]")
{
    auto consul = create_test_consul();
    Sessions manager(consul);

    auto session = manager.create();

    manager.renew(session);

    REQUIRE(manager.destroy(session));

    // the destroy operation is idempotent, should return true for non-existing session
    REQUIRE(manager.destroy(session));
}
