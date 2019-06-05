//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/kv.h"
#include "s11n.h"


namespace ppconsul { namespace kv {

    void load(const s11n::Json& src, KeyValue& dst)
    {
        using s11n::load;

        std::string value;

        load(src, dst.createIndex, "CreateIndex");
        load(src, dst.modifyIndex, "ModifyIndex");
        load(src, dst.lockIndex, "LockIndex");
        load(src, dst.key, "Key");
        load(src, dst.flags, "Flags");
        load(src, value, "Value");
        dst.value = helpers::decodeBase64(value);
        load(src, dst.session, "Session");
    }

    struct TxnResults
    {
        std::vector<KeyValue> results;
    };

    void load(const s11n::Json &src, TxnResults& dst)
    {
        using s11n::load;

        const auto &arr = src["Results"].array_items();
        auto &res = dst.results;
        res.reserve(arr.size());
        for (const auto &item : arr) {
            res.emplace_back();
            load(item["KV"], res.back());
        }
    }

    struct TxnErrorResults
    {
        std::vector<TxnError> errors;
    };

    void load(const s11n::Json &src, TxnError& dst)
    {
        using s11n::load;

        load(src, dst.opIndex, "OpIndex");
        load(src, dst.what, "What");
    }

    void load(const s11n::Json &src, TxnErrorResults& dst)
    {
        using s11n::load;

        const auto &errors_obj = src["Errors"];
        if (errors_obj.is_null()) {
            return;
        }
        const auto &errors = errors_obj.array_items();

        auto &res = dst.errors;
        res.reserve(errors.size());
        for (const auto &error : errors) {
            res.emplace_back();
            load(error, res.back());
        }
    }

namespace impl {

    StringList parseKeys(const std::string& resp)
    {
        return s11n::parseJson<StringList>(resp);
    }

    std::vector<KeyValue> parseValues(const std::string& resp)
    {
        return s11n::parseJson<std::vector<KeyValue>>(resp);
    }

    TxnAborted txnParseErrors(const std::string &resp)
    {
        return TxnAborted(s11n::parseJson<TxnErrorResults>(resp).errors);
    }

    std::vector<KeyValue> txnParseValues(const std::string &resp)
    {
        return s11n::parseJson<TxnResults>(resp).results;
    }

    class TxnOpAppender: public boost::static_visitor<>
    {
    public:
        TxnOpAppender(s11n::Json::array &arr)
            : m_arr(arr)
        {}

        void operator ()(const txn_ops::Set &op) const
        {
            append(s11n::Json::object{
                {"Verb", "set"},
                {"Key", op.key},
                {"Value", helpers::encodeBase64(op.value)},
                {"Flags", static_cast<FlagsType>(op.flags)},
            });
        }

        void operator ()(const txn_ops::CompareSet &op) const
        {
            append(s11n::Json::object{
                {"Verb", "cas"},
                {"Key", op.key},
                {"Value", helpers::encodeBase64(op.value)},
                {"Index", static_cast<IndexType>(op.expectedIndex)},
                {"Flags", static_cast<FlagsType>(op.flags)},
            });
        }

        void operator ()(const txn_ops::Get &op) const
        {
            append(s11n::Json::object{
                {"Verb", "get"},
                {"Key", op.key},
            });
        }

        void operator ()(const txn_ops::GetAll &op) const
        {
            append(s11n::Json::object{
                {"Verb", "get-tree"},
                {"Key", op.keyPrefix},
            });
        }

        void operator ()(const txn_ops::CheckIndex &op) const
        {
            append(s11n::Json::object{
                {"Verb", "check-index"},
                {"Key", op.key},
                {"Index", static_cast<IndexType>(op.expectedIndex)},
            });
        }

        void operator ()(const txn_ops::CheckNotExists &op) const
        {
            append(s11n::Json::object{
                {"Verb", "check-not-exists"},
                {"Key", op.key},
            });
        }

        void operator ()(const txn_ops::Erase &op) const
        {
            append(s11n::Json::object{
                {"Verb", "delete"},
                {"Key", op.key},
            });
        }

        void operator ()(const txn_ops::EraseAll &op) const
        {
            append(s11n::Json::object{
                {"Verb", "delete-tree"},
                {"Key", op.keyPrefix},
            });
        }

        void operator ()(const txn_ops::CompareErase &op) const
        {
            append(s11n::Json::object{
                {"Verb", "delete-cas"},
                {"Key", op.key},
                {"Index", static_cast<IndexType>(op.expectedIndex)},
            });
        }

        void operator ()(const txn_ops::Lock &op) const
        {
            append(s11n::Json::object{
                {"Verb", "lock"},
                {"Key", op.key},
                {"Value", helpers::encodeBase64(op.value)},
                {"Flags", static_cast<FlagsType>(op.flags)},
                {"Session", op.session},
            });
        }

        void operator ()(const txn_ops::Unlock &op) const
        {
            append(s11n::Json::object{
                {"Verb", "unlock"},
                {"Key", op.key},
                {"Value", helpers::encodeBase64(op.value)},
                {"Flags", static_cast<FlagsType>(op.flags)},
                {"Session", op.session},
            });
        }

        void operator ()(const txn_ops::CheckSession &op) const
        {
            append(s11n::Json::object{
                {"Verb", "check-session"},
                {"Key", op.key},
                {"Session", op.session},
            });
        }

    private:
        s11n::Json::array &m_arr;

        void append(s11n::Json::object &&o) const
        {
            m_arr.emplace_back(s11n::Json::object{{"KV", std::move(o)}});
        }

        using FlagsType = double;
        using IndexType = double;
    };

    std::string txnBodyJson(const std::vector<TxnOperation> &ops)
    {
        s11n::Json::array arr;
        arr.reserve(ops.size());
        for (const auto &op : ops) {
            boost::apply_visitor(TxnOpAppender(arr), op);
        }
        return s11n::Json(std::move(arr)).dump();
    }
}
}}
