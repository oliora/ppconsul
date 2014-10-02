//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/helpers.h"
#include <boost/network/protocol/http/message.hpp>

extern "C" {
    #include <b64/cdecode.h>
}


namespace ppconsul { namespace helpers {

    std::string decodeBase64(const std::string& s)
    {
        std::string r;
        r.resize((s.size() + 3) / 4 * 3);

        base64_decodestate state;
        base64_init_decodestate(&state);
        auto len = base64_decode_block(s.data(), s.size(), &r.front(), &state);
        // TODO: check result and throw FormatError
        r.resize(static_cast<size_t>(len));
        return r;
    }

}}
