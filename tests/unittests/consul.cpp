//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/consul.h"
#include <catch/catch.hpp>


TEST_CASE( "consul.BadStatus", "[http][consul][status][error]" )
{
    using ppconsul::BadStatus;
    using ppconsul::http::Status;

    CHECK(BadStatus(Status(500, "Internal Server Error"), "No path to datacenter").what() == std::string("No path to datacenter [500 Internal Server Error]"));
    CHECK(BadStatus(Status(404, "Not Found")).what() == std::string("404 Not Found"));
    CHECK(BadStatus(Status(0, "Nothing")).what() == std::string("000 Nothing"));
    CHECK(BadStatus(Status(9999, "Wrong Long Code")).what() == std::string("9999 Wrong Long Code"));
    CHECK(std::string(BadStatus(Status(1)).what()).find("001") == 0);
}

TEST_CASE( "consul.throwStatusError", "[http][consul][status][error]" )
{
    using namespace ppconsul;

    CHECK_THROWS_AS(throwStatusError(http::Status(500, "Internal Server Error"), "No path to datacenter"), BadStatus);
    CHECK_THROWS_AS(throwStatusError(http::Status(500, "Internal Server Error")), BadStatus);
    CHECK_THROWS_AS(throwStatusError(http::Status(500)), BadStatus);
    CHECK_THROWS_AS(throwStatusError(http::Status(404, "Not Found"), "Something"), NotFoundError);
    CHECK_THROWS_AS(throwStatusError(http::Status(404, "Not Found")), NotFoundError);
    CHECK_THROWS_AS(throwStatusError(http::Status(404)), NotFoundError);
}
