//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include <string>


namespace ppconsul { namespace http {

    class Status
    {
    public:
        explicit Status(int code = 0) PPCONSUL_NOEXCEPT
        : m_code(code)
        {}

        Status(int code, std::string message)
        : m_code(code)
        , m_message(std::move(message))
        {}

        // Returns true if code() is 2xx (i.e. success) and false otherwise
        bool success() const PPCONSUL_NOEXCEPT{ return 2 == m_code / 100; }

        int code() const PPCONSUL_NOEXCEPT{ return m_code; }
        const std::string& message() const PPCONSUL_NOEXCEPT{ return m_message; }

    private:
        int m_code;
        std::string m_message;
    };

}}
