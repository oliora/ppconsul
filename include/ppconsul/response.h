//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include <stdint.h>
#include <chrono>


namespace ppconsul {

    struct ResponseHeaders
    {
        // Creates non-valid headers (i.e. !valid())
        ResponseHeaders()
        : ResponseHeaders(0, false, std::chrono::milliseconds::zero())
        {}

        ResponseHeaders(uint64_t index, bool knownLeader, const std::chrono::milliseconds& lastContact)
        : m_index(index)
        , m_knownLeader(knownLeader)
        , m_lastContact(lastContact)
        {}

        bool valid() const { return 0 != m_index; }
        explicit operator bool () const { return valid(); }

        uint64_t index() const { return m_index; }
        bool knownLeader() const { return m_knownLeader; }
        const std::chrono::milliseconds& lastContact() const { return m_lastContact; }

        uint64_t m_index;
        bool m_knownLeader;
        std::chrono::milliseconds m_lastContact;
    };

    template<class Data>
    struct Response
    {
    public:
        typedef Data DataType;

        Response()
        {}

        Response(const ResponseHeaders& headers)
        : m_headers(headers)
        {}

        Response(const ResponseHeaders& headers, const DataType& data)
        : m_value(data)
        , m_headers(headers)
        {}

        Response(const ResponseHeaders& headers, DataType&& data)
        : m_value(std::move(data))
        , m_headers(headers)
        {}

        ResponseHeaders& headers() { return m_headers; }
        const ResponseHeaders& headers() const { return m_headers; }

        void headers(const ResponseHeaders& headers) { m_headers = headers; }

        DataType& data() { return m_value; }
        const DataType& data() const { return m_value; }

        void data(const DataType& data) { m_value = data; }
        void data(DataType&& data) { m_value = std::move(data); }

    private:
        DataType m_value;
        ResponseHeaders m_headers;
    };

    template<class DataType>
    Response<DataType> makeResponse(const ResponseHeaders& headers, DataType&& data)
    {
        return Response<DataType>(headers, std::forward<DataType>(data));
    }
}
