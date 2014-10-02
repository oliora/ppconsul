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

    CHECK(decodeBase64("YmxhLWJs") == "bla-bl");
    CHECK(decodeBase64("YmxhLWJsYQ==") == "bla-bla");
    //decodeBase64("\x0Awow\x10\x15""bla");
    //decodeBase64("wow\x10\x15""bla");
}
