//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/parameters.h"
#include <catch/catch.hpp>


using ppconsul::Parameters;


TEST_CASE("parameters.Parameters ctor", "[parameters]")
{
    CHECK(Parameters({ { "Do it", "Best as you can" } }).query() == "Do it=Best as you can");
    CHECK(Parameters({ "Do it", "Best as you can" }).query() == "Do it=Best as you can");
    
    CHECK(Parameters({ { "keep", std::string("the tempo") } }).query() == "keep=the tempo");
    CHECK(Parameters({ "keep", std::string("the tempo") }).query() == "keep=the tempo");
    
    CHECK(Parameters({ { "char", 'U' } }).query() == "char=U");
    CHECK(Parameters({ "char", 'U' }).query() == "char=U");

    CHECK(Parameters({ { "uchar", static_cast<unsigned char>(1) } }).query() == "uchar=1");
    CHECK(Parameters({ "uchar", static_cast<unsigned char>(1) }).query() == "uchar=1");
    
    CHECK(Parameters({ { "schar", static_cast<signed char>(-1) } }).query() == "schar=-1");
    CHECK(Parameters({ "schar", static_cast<signed char>(-1) }).query() == "schar=-1");

    CHECK(Parameters({ { "short", short(32767) } }).query() == "short=32767");
    CHECK(Parameters({ { "negshort", short(-32768) } }).query() == "negshort=-32768");
    CHECK(Parameters({ { "ushort", static_cast<unsigned short>(65535) } }).query() == "ushort=65535");
    CHECK(Parameters({ "short", short(32767) }).query() == "short=32767");

    CHECK(Parameters({ { "zero", 0 } }).query() == "zero=0");
    CHECK(Parameters({ { "one", 1 } }).query() == "one=1");
    CHECK(Parameters({ { "int", 24689 } }).query() == "int=24689");
    CHECK(Parameters({ { "negint", -54332 } }).query() == "negint=-54332");
    CHECK(Parameters({ "int", 24689 }).query() == "int=24689");

    CHECK(Parameters({ { "uzero", 0u } }).query() == "uzero=0");
    CHECK(Parameters({ { "uint", 12u } }).query() == "uint=12");
    CHECK(Parameters({ "uint", 12u }).query() == "uint=12");

    CHECK(Parameters({ { "long", long(2147483647ll) } }).query() == "long=2147483647");
    CHECK(Parameters({ { "neglong", long(-2147483648ll) } }).query() == "neglong=-2147483648");
    CHECK(Parameters({ { "ulong", 4294967295ul } }).query() == "ulong=4294967295");
    CHECK(Parameters({ "long", long(2147483647ll) }).query() == "long=2147483647");

    CHECK(Parameters({ { "longlong", 9223372036854775807ll } }).query() == "longlong=9223372036854775807");
    CHECK(Parameters({ { "neglonglong", -9223372036854775807ll - 1 } }).query() == "neglonglong=-9223372036854775808");
    CHECK(Parameters({ { "ulonglong", 18446744073709551615ull } }).query() == "ulonglong=18446744073709551615");
    CHECK(Parameters({ "ulonglong", 18446744073709551615ull }).query() == "ulonglong=18446744073709551615");

    CHECK(Parameters({ { "zero", 0.0 } }).query() == "zero=0.000000");
    CHECK(Parameters({ { "double", 123456789.123456 } }).query() == "double=123456789.123456");
    CHECK(Parameters({ { "negdouble", -123456789.123456 } }).query() == "negdouble=-123456789.123456");
    CHECK(Parameters({ "double", 123456789.123456 }).query() == "double=123456789.123456");

    CHECK(Parameters({ { "float", 2.000001f } }).query() == "float=2.000001");
    CHECK(Parameters({ { "negfloat", -5.200001f } }).query() == "negfloat=-5.200001");
    CHECK(Parameters({ "float", 2.000001f }).query() == "float=2.000001");
    
    CHECK(Parameters({ { "longdouble", 1234567891.987654l } }).query() == "longdouble=1234567891.987654");
    CHECK(Parameters({ "longdouble", 1234567891.987654l }).query() == "longdouble=1234567891.987654");
    CHECK(Parameters({ { "neglongdouble", -1234567891.987654l } }).query() == "neglongdouble=-1234567891.987654");

    CHECK(Parameters({ { "true", true } }).query() == "true=1");
    CHECK(Parameters({ "true", true }).query() == "true=1");
    CHECK(Parameters({ { "false", false } }).query() == "false=0");
    CHECK(Parameters({ "false", false }).query() == "false=0");
}

TEST_CASE("parameters.Parameters ctor from reference", "[parameters]")
{
    char chararr[] = "Best as you can";
    char *c_str = chararr;
    const char *cc_str = c_str;
    std::string str = cc_str;
    const std::string cstr = cc_str;

    CHECK(Parameters({ { "Do it", chararr } }).query() == "Do it=Best as you can");
    CHECK(Parameters({ "Do it", chararr }).query() == "Do it=Best as you can");

    CHECK(Parameters({ { "Do it", c_str } }).query() == "Do it=Best as you can");
    CHECK(Parameters({ "Do it", c_str }).query() == "Do it=Best as you can");

    CHECK(Parameters({ { "Do it", cc_str } }).query() == "Do it=Best as you can");
    CHECK(Parameters({ "Do it", cc_str }).query() == "Do it=Best as you can");
    
    CHECK(Parameters({ { "Do it", str } }).query() == "Do it=Best as you can");
    CHECK(Parameters({ "Do it", str }).query() == "Do it=Best as you can");

    CHECK(Parameters({ { "Do it", cstr } }).query() == "Do it=Best as you can");
    CHECK(Parameters({ "Do it", cstr }).query() == "Do it=Best as you can");

    char c = 'f';
    const char cc = c;

    CHECK(Parameters({ { "char", c } }).query() == "char=f");
    CHECK(Parameters({ "char", c }).query() == "char=f");
    
    CHECK(Parameters({ { "char", cc } }).query() == "char=f");
    CHECK(Parameters({ "char", cc }).query() == "char=f");

    int i = 99;
    const int ci = i;

    CHECK(Parameters({ { "int", i } }).query() == "int=99");
    CHECK(Parameters({ "int", i }).query() == "int=99");
    CHECK(Parameters({ { "int", ci } }).query() == "int=99");
    CHECK(Parameters({ "int", ci }).query() == "int=99");

    double d = 4.1;
    const double cd = d;

    CHECK(Parameters({ { "double", d } }).query() == "double=4.100000");
    CHECK(Parameters({ "double", d }).query() == "double=4.100000");
    CHECK(Parameters({ { "double", cd } }).query() == "double=4.100000");
    CHECK(Parameters({ "double", cd }).query() == "double=4.100000");

    float f = 4.2f;
    const float cf = f;

    CHECK(Parameters({ { "float", f } }).query() == "float=4.200000");
    CHECK(Parameters({ "float", f }).query() == "float=4.200000");
    CHECK(Parameters({ { "float", cf } }).query() == "float=4.200000");
    CHECK(Parameters({ "float", cf }).query() == "float=4.200000");

    long double ld = -0.1;
    const long double cld = ld;

    CHECK(Parameters({ { "long-double", ld } }).query() == "long-double=-0.100000");
    CHECK(Parameters({ "long-double", ld }).query() == "long-double=-0.100000");
    CHECK(Parameters({ { "long-double", cld } }).query() == "long-double=-0.100000");
    CHECK(Parameters({ "long-double", cld }).query() == "long-double=-0.100000");

}

TEST_CASE("parameters.Multiple parameters ctor", "[parameters]")
{
    auto p = Parameters({ { "intparam1", 1 }, { "strparam2", "value2" }, { "doubleparam3", 0.2 } });
    CHECK(p.query() == "doubleparam3=0.200000&intparam1=1&strparam2=value2");
}

TEST_CASE("parameters.Default constructed parameters", "[parameters]")
{
    auto p = Parameters();

    CHECK(p.empty());
    CHECK(p.query() == "");
}

TEST_CASE("parameters.Update parameters", "[parameters]")
{
    auto p = Parameters();

    p.update({ { "intparam1", 1 }, { "strparam2", "value2" } });
    REQUIRE(p.query() == "intparam1=1&strparam2=value2");
    REQUIRE(!p.empty());

    p.update({ { "doubleparam3", 0.2 }, { "strparam4", "value4" } });
    REQUIRE(p.query() == "doubleparam3=0.200000&intparam1=1&strparam2=value2&strparam4=value4");
    REQUIRE(!p.empty());

    p.update({ { "boolparam5", true } });
    REQUIRE(p.query() == "boolparam5=1&doubleparam3=0.200000&intparam1=1&strparam2=value2&strparam4=value4");
    REQUIRE(!p.empty());

    p.update("p4", "bla-bla");
    REQUIRE(p.query() == "boolparam5=1&doubleparam3=0.200000&intparam1=1&p4=bla-bla&strparam2=value2&strparam4=value4");
    REQUIRE(!p.empty());
}

TEST_CASE("parameters.Update parameters from reference", "[parameters]")
{
    auto p1 = Parameters();

    char chararr[] = "charrr";
    char *c_str = chararr;
    const char *cc_str = c_str;
    std::string str = cc_str;
    const std::string cstr = cc_str;

    p1.update("p1", chararr);
    p1.update("p2", c_str);
    p1.update("p3", cc_str);
    p1.update("p4", str);
    p1.update("p5", cstr);

    CHECK(p1.query() == "p1=charrr&p2=charrr&p3=charrr&p4=charrr&p5=charrr");

    auto p2 = Parameters();

    char c = 'f';
    const char cc = c;

    p2.update("p1", c);
    p2.update("p2", cc);

    CHECK(p2.query() == "p1=f&p2=f");

    auto p3 = Parameters();

    int i = 99;
    const int ci = i;

    p3.update("p1", i);
    p3.update("p2", ci);

    CHECK(p3.query() == "p1=99&p2=99");

    auto p4 = Parameters();

    double d = 4.1;
    const double cd = d;

    p4.update("p1", d);
    p4.update("p2", cd);

    CHECK(p4.query() == "p1=4.100000&p2=4.100000");

    auto p5 = Parameters();

    float f = 4.2f;
    const float cf = f;

    p5.update("p1", f);
    p5.update("p2", cf);

    CHECK(p5.query() == "p1=4.200000&p2=4.200000");

    auto p6 = Parameters();

    long double ld = -0.1;
    const long double cld = ld;

    p6.update("p1", ld);
    p6.update("p2", cld);

    CHECK(p6.query() == "p1=-0.100000&p2=-0.100000");
}

TEST_CASE("parameters.Overwrite parameters", "[parameters]")
{
    auto p = Parameters({ { "p1", 1 }, { "p2", "value2" } });
    REQUIRE(p.query() == "p1=1&p2=value2");

    p.update({ { "p1", 2 }, { "p2", 2.5 } });
    REQUIRE(p.query() == "p1=2&p2=2.500000");
    REQUIRE(!p.empty());

    p.update({ { "p1", 2 }, { "p3", "value3" } });
    REQUIRE(p.query() == "p1=2&p2=2.500000&p3=value3");
    REQUIRE(!p.empty());

    p.update("p1", 3);
    REQUIRE(p.query() == "p1=3&p2=2.500000&p3=value3");
    REQUIRE(!p.empty());
}

TEST_CASE("parameters.Ctor then update parameters", "[parameters]")
{
    auto p = Parameters({ { "intparam1", 1 }, { "strparam2", "value2" } });
    REQUIRE(p.query() == "intparam1=1&strparam2=value2");
    REQUIRE(!p.empty());

    p.update({ { "doubleparam3", 0.2 }, { "strparam4", "value4" } });
    REQUIRE(p.query() == "doubleparam3=0.200000&intparam1=1&strparam2=value2&strparam4=value4");
    REQUIRE(!p.empty());

    p.update({ { "boolparam5", false } });
    REQUIRE(p.query() == "boolparam5=0&doubleparam3=0.200000&intparam1=1&strparam2=value2&strparam4=value4");
    REQUIRE(!p.empty());
}

TEST_CASE("parameters.Clear and empty parameters", "[parameters]")
{
    auto p = Parameters({ { "param1", 1 }, { "param2", "value2" } });
    REQUIRE(!p.empty());
    REQUIRE(p.query() == "param1=1&param2=value2");

    p.clear();
    REQUIRE(p.empty());
    REQUIRE(p.query() == "");

    p.update({ { "intparam1", 1 }, { "strparam2", "value2" } });
    REQUIRE(p.query() == "intparam1=1&strparam2=value2");
    REQUIRE(!p.empty());

    p.clear();
    REQUIRE(p.empty());
    REQUIRE(p.query() == "");

    p.update({ { "boolparam5", false } });
    REQUIRE(p.query() == "boolparam5=0");
    REQUIRE(!p.empty());
}

TEST_CASE("parameters.Copy parameters", "[parameters]")
{
    auto p = Parameters({ { "param1", 1 }, { "param2", "value2" } });
    REQUIRE(!p.empty());
    REQUIRE(p.query() == "param1=1&param2=value2");

    Parameters p2 = p;

    CHECK(!p2.empty());
    CHECK(p2.query() == "param1=1&param2=value2");
    CHECK(p2.query() == p.query());
}

TEST_CASE("parameters.Move parameters", "[parameters]")
{
    auto p = Parameters({ { "param1", 1 }, { "param2", "value2" } });
    REQUIRE(!p.empty());
    REQUIRE(p.query() == "param1=1&param2=value2");

    Parameters p2 = std::move(p);

    CHECK(!p2.empty());
    CHECK(p2.query() == "param1=1&param2=value2");
}
