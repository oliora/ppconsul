//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include "ppconsul/http_status.h"
#include <stdexcept>


namespace ppconsul {

    class Error: public std::exception {};

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

    class BadStatus: public Error
    {
    public:
        explicit BadStatus(http::Status status, std::string message = "")
        : m_status(std::move(status))
        , m_message(std::move(message))
        {}

        int code() const PPCONSUL_NOEXCEPT{ return m_status.code(); }

        const http::Status& status() const PPCONSUL_NOEXCEPT{ return m_status; }
        const std::string& message() const PPCONSUL_NOEXCEPT{ return m_message; }

        virtual const char *what() const PPCONSUL_NOEXCEPT override;

    private:
        http::Status m_status;
        std::string m_message;
        mutable std::string m_what;
    };

    class NotFoundError: public BadStatus
    {
    public:
        enum { Code = 404 };

        /*explicit NotFoundError(http::Status status, std::string message = "")
        : BadStatus(std::move(status), std::move(message))
        {}*/

        NotFoundError()
        : BadStatus(http::Status(Code, "Not Found"))
        {}
    };

    void throwStatusError(http::Status status, std::string data = "");

}
