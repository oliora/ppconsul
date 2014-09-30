//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/helpers.h"
#include <boost/network/utils/base64/encode.hpp>


namespace ppconsul { namespace helpers {

    std::string decodeBase64(const std::string& s)
    {
        return boost::network::utils::base64::encode<char>(s);
    }

}}
