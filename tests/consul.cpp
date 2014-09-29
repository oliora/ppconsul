#include "ppconsul/consul.h"
#include "catch.hpp"

TEST_CASE( "Make URL", "[http, url, consul]" )
{
    using ppconsul::impl::makeUrl;

    REQUIRE(makeUrl("http://www.example.com", "/something/interesting", { { "1stparam", "urgent" }, { "2ndparam", 2 } }) ==
        "http://www.example.com/something/interesting?1stparam=urgent&2ndparam=2");

    REQUIRE(makeUrl("https://127.0.0.1:8090", "/something/interesting") ==
        "https://127.0.0.1:8090/something/interesting");

    REQUIRE(makeUrl("http://127.0.0.1:8090", "/something/interesting", {"p1", 42}) ==
        "http://127.0.0.1:8090/something/interesting?p1=42");

    REQUIRE(makeUrl("http://127.0.0.1:8090", "/something/interesting?p1=42") ==
        "http://127.0.0.1:8090/something/interesting?p1=42");
}
