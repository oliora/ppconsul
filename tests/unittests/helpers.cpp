//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/helpers.h"
#include <catch/catch.hpp>


TEST_CASE("helpers.base64decode", "[base64]")
{
    using ppconsul::helpers::decodeBase64;

    CHECK("" == decodeBase64(""));
    CHECK("sure." == decodeBase64("c3VyZS4="));
    CHECK("sure." == decodeBase64("c3VyZS4"));
    CHECK("sure" == decodeBase64("c3VyZQ=="));
    CHECK("sure" == decodeBase64("c3VyZQ"));
    CHECK("sur" == decodeBase64("c3Vy"));
    CHECK("su" == decodeBase64("c3U="));
    CHECK("s" == decodeBase64("cw=="));
    CHECK("bla-bl" == decodeBase64("YmxhLWJs"));
    CHECK("bla-bla" == decodeBase64("YmxhLWJsYQ=="));
    CHECK("bla-bla" == decodeBase64("\x10YmxhLWJsYQ=="));

    CHECK("bla-bla" == decodeBase64("YmxhLWJ\x10\x15sYQ=="));
    CHECK_NOTHROW(decodeBase64("Ym\x15xhL\x0WJsYQ==")); // Not a real test, just be sure that it's not fail
}

TEST_CASE("helpers.url encode", "[url]")
{
    using ppconsul::helpers::encodeUrl;

    CHECK(encodeUrl("") == "");
    CHECK(encodeUrl("\x01\x13 bla /* {tag}%") == "%01%13%20bla%20%2F%2A%20%7Btag%7D%25");
    CHECK(encodeUrl("\x7F\x80\x81.-_~") == "%7F%80%81.-_~");
    CHECK(encodeUrl({"\x0?&", 3}) == "%00%3F%26");
}
