//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/types.h"
#include "s11n.h"


namespace ppconsul {

    inline void load(const s11n::Json& src, Service& dst)
    {
        using s11n::load;

        load(src, dst.name, "Service");
        load(src, dst.port, "Port");
        load(src, dst.tags, "Tags");
        load(src, dst.id, "ID");
    }

}
