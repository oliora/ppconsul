//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "all_clients.h"


namespace ppconsul {

    namespace {
    }

    const char *BadStatus::what() const PPCONSUL_NOEXCEPT
    {
        if (m_what.empty())
        {
            if (!m_message.empty())
            {
                m_what = helpers::format("%s [%03d %s]",
                    m_message.c_str(),
                    m_status.code(),
                    m_status.message().c_str());
            }
            else
            {
                m_what = helpers::format("%03d %s",
                    m_status.code(),
                    m_status.message().c_str());
            }
        }

        return m_what.c_str();
    }

    Consul::Consul(const std::string& defaultToken, const std::string& dataCenter, const std::string& addr)
    : m_client(new impl::HttpClient(addr))
    , m_dataCenter(dataCenter)
    , m_defaultToken(defaultToken)
    {}

}
