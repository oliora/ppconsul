//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/parameters.h"
#include "ppconsul/consul.h"
#include <catch/catch.hpp>


namespace test_params {
    
}


TEST_CASE("makeQuery", "[parameters]")
{
    using ppconsul::parameters::makeQuery;

    CHECK(makeQuery() == "");

}
