#pragma once

#include <chrono>
#include <iostream>
#include <iomanip>
#include <type_traits>


namespace chrono_io_detail
{
    template<class Period> struct native_period: std::false_type {};

    template<> struct native_period<std::chrono::nanoseconds::period>: std::true_type {};
    template<> struct native_period<std::chrono::microseconds::period>: std::true_type {};
    template<> struct native_period<std::chrono::milliseconds::period>: std::true_type {};
    template<> struct native_period<std::chrono::seconds::period>: std::true_type {};
    template<> struct native_period<std::chrono::minutes::period>: std::true_type {};
    template<> struct native_period<std::chrono::hours::period>: std::true_type {};
    

    template<class Period>
    struct period_name;

    template<> struct period_name<std::chrono::nanoseconds::period>
    {
        template<class CharT> static const CharT* name()
        {
            static CharT s[] = {'n', 's' , 0};
            return s;
        }
    };

    template<> struct period_name<std::chrono::microseconds::period>
    {
        template<class CharT> static const CharT* name()
        {
            static CharT s[] = { 'u', 's', 0 };
            return s;
        }
    };

    template<> struct period_name<std::chrono::milliseconds::period>
    {
        template<class CharT> static const CharT* name()
        {
            static CharT s[] = { 'm', 's', 0 };
            return s;
        }
    };

    template<> struct period_name<std::chrono::seconds::period>
    {
        template<class CharT> static const CharT* name()
        {
            static CharT s[] = { 's', 0 };
            return s;
        }
    };

    template<> struct period_name<std::chrono::minutes>
    {
        template<class CharT> static const CharT* name()
        {
            static CharT s[] = { 'm', 'i', 'n', 0 };
            return s;
        }
    };

    template<> struct period_name<std::chrono::hours>
    {
        template<class CharT> static const CharT* name()
        {
            static CharT s[] = { 'h', 0 };
            return s;
        }
    };
}

namespace std { namespace chrono {

    template<class CharT, class Traits, class Rep, class Period>
    typename std::enable_if<
        ::chrono_io_detail::native_period<Period>::value,
        std::basic_ostream<CharT, Traits>&
    >::type operator<< (std::basic_ostream<CharT, Traits>& os, const std::chrono::duration<Rep, Period>& t)
    {
        os << t.count() << ::chrono_io_detail::period_name<Period>::template name<CharT>();
        return os;
    }

    template<class CharT, class Traits, class Rep, class Period>
    typename std::enable_if<
        !::chrono_io_detail::native_period<Period>::value,
        std::basic_ostream<CharT, Traits>&
    >::type operator<< (std::basic_ostream<CharT, Traits>& os, const std::chrono::duration<Rep, Period>& t)
    {
        auto prev_prec = os.precision();

        return os
            << std::setprecision(std::numeric_limits<double>::digits10 + 1)
            << std::chrono::duration<double, std::chrono::seconds::period>(t)
            << std::setprecision(prev_prec);
    }
}}
