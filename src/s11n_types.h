//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/types.h"
#include "s11n.h"


namespace ppconsul {

    inline void load(const s11n::Json& src, ServiceInfo& dst)
    {
        using s11n::load;

        load(src, dst.id, "ID");
        load(src, dst.name, "Service");
        load(src, dst.address, "Address");
        load(src, dst.port, "Port");
        load(src, dst.tags, "Tags");
        load(src, dst.meta, "Meta");
    }

    inline void load(const s11n::Json& src, Node& dst)
    {
        using s11n::load;

        load(src, dst.node, "Node");
        load(src, dst.address, "Address");
    }

    inline void load(const s11n::Json& src, CheckStatus& dst)
    {
        const auto& s = src.string_value();

        if (s == "passing")
            dst = CheckStatus::Passing;
        else if (s == "warning")
            dst = CheckStatus::Warning;
        else if (s == "critical")
            dst = CheckStatus::Critical;
        else
            dst = CheckStatus::Unknown;
    }


    inline void load(const s11n::Json& src, CheckInfo& dst)
    {
        using s11n::load;

        load(src, dst.id, "CheckID");
        load(src, dst.node, "Node");
        load(src, dst.name, "Name");
        load(src, dst.status, "Status");
        load(src, dst.notes, "Notes");
        load(src, dst.output, "Output");
        load(src, dst.serviceId, "ServiceID");
        load(src, dst.serviceName, "ServiceName");
    }

    inline void load(const json11::Json& src, Coordinate& dst)
    {
        using s11n::load;

        load(src, dst.adjustment, "Adjustment");
        load(src, dst.error, "Error");
        load(src, dst.height, "Height");
        load(src, dst.vec, "Vec");
    }

}
