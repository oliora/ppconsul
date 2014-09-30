//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include <json11.hpp>


namespace ppconsul { namespace json {

    using namespace json11;

    class FormatError: public ppconsul::Error
    {
    public:
        FormatError(std::string message)
        : m_message(std::move(message))
        {}

        virtual const char *what() const PPCONSUL_NOEXCEPT override { return m_message.c_str(); }

    private:
        std::string m_message;
    };

    inline uint64_t uint64_value(const Json& v)
    {
        // TODO: support full precision of uint64_t in json11
        return static_cast<uint64_t>(v.number_value());
    }

    inline Json parse_json(const std::string &s)
    {
        std::string err;
        auto obj = Json::parse(s, err);
        if (!err.empty())
            throw FormatError(std::move(err));
        return obj;
    }

}}
