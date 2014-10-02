//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/helpers.h"
#include <catch/catch.hpp>


TEST_CASE("helpers.base64decode", "[parameters]")
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
