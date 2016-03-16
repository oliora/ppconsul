//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "kwargs.h"
#include "helpers.h"
#include <type_traits>
#include <string>
#include <sstream>
#include <boost/fusion/include/for_each.hpp>


#define PPCONSUL_KEYWORD_NAMED_(keyword, type, name_)   \
    KWARGS_KEYWORD(keyword, type)                    \
    inline const char *parameter_name(KWARGS_KW_TAG(keyword)) { return name_; }

#define PPCONSUL_KEYWORD(keyword, type) PPCONSUL_KEYWORD_NAMED_(keyword, type, BOOST_PP_STRINGIZE(keyword))


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
                os << parameter_name(k) << "=" << helpers::encodeUrl(v);
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

    template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
    std::string makeQuery(const Params&... params)
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

    template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
    std::string makeUrl(const std::string& path, const Params&... params)
    {
        auto query = makeQuery(params...);

        std::string r;
        r.reserve(path.size() + query.size() + 1); // 1 is for query prefix '?'

        r += path;
        if (!query.empty())
        {
            r += '?';
            r += query;
        }

        return r;
    }
}}
