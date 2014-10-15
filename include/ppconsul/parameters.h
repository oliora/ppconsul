//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "kwargs.h"
#include <type_traits>
#include <string>
#include <sstream>
#include <boost/fusion/include/for_each.hpp>


#define PPCONSUL_PARAM_NO_NAME KWARGS_KEYWORD
#define PPCONSUL_PARAMS_GROUP KWARGS_KEYWORDS_GROUP

#define PPCONSUL_PARAM_NAMED(keyword, type, name_)   \
    KWARGS_KEYWORD(keyword, type)                    \
    inline const char *parameter_name(KWARGS_KW_TAG(keyword)) { return name_; }

#define PPCONSUL_PARAM(keyword, type) PPCONSUL_PARAM_NAMED(keyword, type, BOOST_PP_STRINGIZE(keyword))


namespace ppconsul { namespace parameters {

    namespace detail {
        template<class Keyword, class Value>
        void printParameter(std::ostream&os, const Value& v, Keyword k)
        {
            os << parameter_name(k) << "=" << v;
        }

        template<class Keyword>
        void printParameter(std::ostream&os, const std::string& v, Keyword k)
        {
            if (!v.empty())
                os << parameter_name(k) << "=" << v;
        }

        struct ParameterPrinter
        {
            ParameterPrinter(std::ostream& os): m_os(os) {}

            template<class T>
            void operator() (const T& t) const
            {
                auto pos = m_os.tellp();
                printParameter(m_os, kwargs::get_value(t), kwargs::get_keyword(t));
                if (m_os.tellp() != pos)
                    m_os << '&';
            }

        private:
            std::ostream& m_os;
        };
    }

    inline std::string makeQuery()
    {
        return{};
    }

    template<class... Parameters>
    typename std::enable_if<sizeof...(Parameters) != 0, std::string>::type
        makeQuery(const Parameters&... params)
    {
        std::ostringstream os;

        const auto p = kwargs::unique_args(params...);
        boost::fusion::for_each(p, detail::ParameterPrinter(os));

        // Remove last '&' if exists
        auto r = os.str();
        if (!r.empty() && r.back() == '&')
            r.resize(r.size() - 1);
        return r;
    }

    template<class... Parameters>
    std::string makeUrl(const std::string& addr, const std::string& path, const Parameters&... params)
    {
        auto query = makeQuery(params...);

        std::string r;
        r.reserve(addr.size() + path.size() + query.size() + 1); // 1 is for query prefix '?'

        r += addr;
        r += path;
        if (!query.empty())
        {
            r += '?';
            r += query;
        }

        return r;
    }
}}
