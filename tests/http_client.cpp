#include "ppconsul/http_client.h"
#include "catch.hpp"


TEST_CASE( "HTTP parameters", "[http, parameters]" )
{
    using ppconsul::http::Parameters;

    SECTION("Types support")
    {
        REQUIRE(Parameters({ { "Do it", "Best as you can" } }).query() == "Do it=Best as you can");
        
        REQUIRE(Parameters({ { "keep", std::string("the tempo") } }).query() == "keep=the tempo");
        
        REQUIRE(Parameters({ { "char", 'U' } }).query() == "char=U");

        REQUIRE(Parameters({ { "uchar", static_cast<unsigned char>(1) } }).query() == "uchar=1");
        REQUIRE(Parameters({ { "schar", static_cast<signed char>(-1) } }).query() == "schar=-1");

        REQUIRE(Parameters({ { "short", short(32767) } }).query() == "short=32767");
        REQUIRE(Parameters({ { "negshort", short(-32768) } }).query() == "negshort=-32768");
        REQUIRE(Parameters({ { "ushort", static_cast<unsigned short>(65535) } }).query() == "ushort=65535");

        REQUIRE(Parameters({ { "zero", 0 } }).query() == "zero=0");
        REQUIRE(Parameters({ { "one", 1 } }).query() == "one=1");
        REQUIRE(Parameters({ { "int", 24689 } }).query() == "int=24689");
        REQUIRE(Parameters({ { "negint", -54332 } }).query() == "negint=-54332");

        REQUIRE(Parameters({ { "uzero", 0u } }).query() == "uzero=0");
        REQUIRE(Parameters({ { "uint", 12u } }).query() == "uint=12");

        REQUIRE(Parameters({ { "long", long(2147483647ll) } }).query() == "long=2147483647");
        REQUIRE(Parameters({ { "neglong", long(-2147483648ll) } }).query() == "neglong=-2147483648");
        REQUIRE(Parameters({ { "ulong", 4294967295ul } }).query() == "ulong=4294967295");

        REQUIRE(Parameters({ { "longlong", 9223372036854775807ll } }).query() == "longlong=9223372036854775807");
        REQUIRE(Parameters({ { "neglonglong", -9223372036854775808ll } }).query() == "neglonglong=-9223372036854775808");
        REQUIRE(Parameters({ { "ulonglong", 18446744073709551615ull } }).query() == "ulonglong=18446744073709551615");

        REQUIRE(Parameters({ { "zero", 0.0 } }).query() == "zero=0.000000");
        REQUIRE(Parameters({ { "double", 123456789.123456 } }).query() == "double=123456789.123456");
        REQUIRE(Parameters({ { "negdouble", -123456789.123456 } }).query() == "negdouble=-123456789.123456");

        REQUIRE(Parameters({ { "float", 2.000001f } }).query() == "float=2.000001");
        REQUIRE(Parameters({ { "negfloat", -5.200001f } }).query() == "negfloat=-5.200001");
        
        REQUIRE(Parameters({ { "longdouble", 1234567891.987654l } }).query() == "longdouble=1234567891.987654");
        REQUIRE(Parameters({ { "neglongdouble", -1234567891.987654l } }).query() == "neglongdouble=-1234567891.987654");

        REQUIRE(Parameters({ { "true", true } }).query() == "true=1");
        REQUIRE(Parameters({ { "false", false } }).query() == "false=0");
    }
    
    SECTION("References support")
    {
        char chararr[] = "Best as you can";
        char *c_str = chararr;
        const char *cc_str = c_str;
        std::string str = cc_str;
        const std::string cstr = cc_str;

        REQUIRE(Parameters({ { "Do it", chararr } }).query() == "Do it=Best as you can");
        REQUIRE(Parameters({ { "Do it", c_str } }).query() == "Do it=Best as you can");
        REQUIRE(Parameters({ { "Do it", cc_str } }).query() == "Do it=Best as you can");
        REQUIRE(Parameters({ { "Do it", str } }).query() == "Do it=Best as you can");
        REQUIRE(Parameters({ { "Do it", cstr } }).query() == "Do it=Best as you can");

        char c = 'f';
        const char cc = c;

        REQUIRE(Parameters({ { "char", c } }).query() == "char=f");
        REQUIRE(Parameters({ { "char", cc } }).query() == "char=f");

        int i = 99;
        const int ci = i;

        REQUIRE(Parameters({ { "int", i } }).query() == "int=99");
        REQUIRE(Parameters({ { "int", ci } }).query() == "int=99");
    }

    SECTION("Multiple parameters")
    {
        auto p = Parameters({ { "intparam1", 1 }, { "strparam2", "value2" }, { "doubleparam3", 0.2 } });
        REQUIRE(p.query() == "intparam1=1&strparam2=value2&doubleparam3=0.200000");
    }

    SECTION("Default constructed parameters")
    {
        auto p = Parameters();

        REQUIRE(p.empty());
        REQUIRE(p.query() == "");
    }

    SECTION("Append")
    {
        auto p = Parameters();

        p.append({ { "intparam1", 1 }, { "strparam2", "value2" } });
        REQUIRE(p.query() == "intparam1=1&strparam2=value2");
        REQUIRE(!p.empty());

        p.append({ { "doubleparam3", 0.2 }, { "strparam4", "value4" } });
        REQUIRE(p.query() == "intparam1=1&strparam2=value2&doubleparam3=0.200000&strparam4=value4");
        REQUIRE(!p.empty());

        p.append({ { "boolparam5", true } });
        REQUIRE(p.query() == "intparam1=1&strparam2=value2&doubleparam3=0.200000&strparam4=value4&boolparam5=1");
        REQUIRE(!p.empty());
    }
    
    SECTION("Construct then append")
    {
        auto p = Parameters({ { "intparam1", 1 }, { "strparam2", "value2" } });
        REQUIRE(p.query() == "intparam1=1&strparam2=value2");
        REQUIRE(!p.empty());

        p.append({ { "doubleparam3", 0.2 }, { "strparam4", "value4" } });
        REQUIRE(p.query() == "intparam1=1&strparam2=value2&doubleparam3=0.200000&strparam4=value4");
        REQUIRE(!p.empty());

        p.append({ { "boolparam5", false } });
        REQUIRE(p.query() == "intparam1=1&strparam2=value2&doubleparam3=0.200000&strparam4=value4&boolparam5=0");
        REQUIRE(!p.empty());
    }

    SECTION("Clear and empty")
    {
        auto p = Parameters({ { "param1", 1 }, { "param2", "value2" } });
        REQUIRE(!p.empty());
        REQUIRE(p.query() == "param1=1&param2=value2");

        p.clear();
        REQUIRE(p.empty());
        REQUIRE(p.query() == "");

        p.append({ { "intparam1", 1 }, { "strparam2", "value2" } });
        REQUIRE(p.query() == "intparam1=1&strparam2=value2");
        REQUIRE(!p.empty());

        p.clear();
        REQUIRE(p.empty());
        REQUIRE(p.query() == "");

        p.append({ { "boolparam5", false } });
        REQUIRE(p.query() == "boolparam5=0");
        REQUIRE(!p.empty());
    }

    SECTION("Copy")
    {
        auto p = Parameters({ { "param1", 1 }, { "param2", "value2" } });
        REQUIRE(!p.empty());
        REQUIRE(p.query() == "param1=1&param2=value2");

        Parameters p2 = p;

        REQUIRE(!p2.empty());
        REQUIRE(p2.query() == "param1=1&param2=value2");
        REQUIRE(p2.query() == p.query());
    }

    SECTION("Move")
    {
        auto p = Parameters({ { "param1", 1 }, { "param2", "value2" } });
        REQUIRE(!p.empty());
        REQUIRE(p.query() == "param1=1&param2=value2");

        Parameters p2 = std::move(p);

        REQUIRE(!p2.empty());
        REQUIRE(p2.query() == "param1=1&param2=value2");
    }
}

TEST_CASE( "Create URL", "[http, url]" )
{
    using ppconsul::http::createUrl;

    REQUIRE(createUrl("http://www.example.com", "/something/interesting", { { "1stparam", "urgent" }, { "2ndparam", 2 } }) ==
        "http://www.example.com/something/interesting?1stparam=urgent&2ndparam=2");
}
