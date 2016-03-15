//  Copyright (c) 2014-2016 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"

#if defined PPCONSUL_USE_CPPNETLIB
#include "http_client_netlib.h"
#else
#include "http_client_curl.h"
#endif


namespace ppconsul {

    namespace {
        // Creates base URL like "http://<addr>" if 'addr' has no scheme specified
        // or just "<addr>" if 'addr' contains any scheme.
        inline std::string makeAddr(const std::string& addr)
        {
            if (addr.find("://") != std::string::npos)
                return addr;
            else
                return "http://" + addr;
        }
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
    : m_client(new http::impl::HttpClient())
    , m_addr(makeAddr(addr))
    , m_dataCenter(dataCenter)
    , m_defaultToken(defaultToken)
    {}

}
