#pragma once

#include "ppconsul/consul.h"
#include <cstdlib>


inline std::string get_test_datacenter()
{
    auto datacenter = std::getenv("PPCONSUL_TEST_DC");
    return datacenter ? datacenter : "ppconsul_test";
}

inline ppconsul::Consul create_test_consul()
{
    auto addr = std::getenv("PPCONSUL_TEST_ADDR");

    return ppconsul::Consul(addr ? addr : ppconsul::Default_Server_Address,
        ppconsul::params::dc = get_test_datacenter());
}

namespace test_detail
{
    template<class Period>
    struct DurationPeriodName;

    template<>
    struct DurationPeriodName<std::chrono::seconds::period>
    {
        template<class CharT>
        static const CharT* name()
        {
            static CharT s[] = {'s' , 0};
            return s;
        }
    };

    template<>
    struct DurationPeriodName<std::chrono::nanoseconds::period>
    {
        template<class CharT>
        static const CharT* name()
        {
            static CharT s[] = {'n', 's' , 0};
            return s;
        }
    };

    template<>
    struct DurationPeriodName<std::chrono::microseconds::period>
    {
        template<class CharT>
        static const CharT* name()
        {
            static CharT s[] = {'u', 's' , 0};
            return s;
        }
    };

    template<>
    struct DurationPeriodName<std::chrono::milliseconds::period>
    {
        template<class CharT>
        static const CharT* name()
        {
            static CharT s[] = {'m', 's' , 0};
            return s;
        }
    };

    template<>
    struct DurationPeriodName<std::chrono::minutes>
    {
        template<class CharT>
        static const CharT* name()
        {
            static CharT s[] = {'m', 'i' ,'n', 0};
            return s;
        }
    };

    template<>
    struct DurationPeriodName<std::chrono::hours>
    {
        template<class CharT>
        static const CharT* name()
        {
            static CharT s[] = {'h', 0};
            return s;
        }
    };
}

namespace std { namespace chrono {

    template<class CharT, class Traits, class Rep, class Period>
    std::basic_ostream<CharT, Traits>& operator<< (std::basic_ostream<CharT, Traits>& os, const std::chrono::duration<Rep, Period>& t)
    {
        os << t.count() << test_detail::DurationPeriodName<Period>::template name<CharT>();
        return os;
    }

}}
