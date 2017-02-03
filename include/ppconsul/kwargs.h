//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/mpl/pair.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/reverse_iter_fold.hpp>
#include <boost/mpl/reverse.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/bind.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/apply_wrap.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/remove_if.hpp>

#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/vector_tie.hpp>
#include <boost/fusion/include/mpl.hpp>

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/cat.hpp>

#include <type_traits>
#include <utility>


namespace kwargs {

    template<typename K, typename V>
    using KwArg = boost::fusion::pair<K, V>;

    namespace detail {

        using boost::mpl::pair;
        using boost::mpl::first;
        using boost::mpl::second;
        using boost::mpl::begin;
        using boost::mpl::end;
        using boost::mpl::fold;
        using boost::mpl::transform;
        using boost::mpl::reverse_iter_fold;
        using boost::mpl::reverse;
        using boost::mpl::set;
        using boost::mpl::vector;
        using boost::mpl::has_key;
        using boost::mpl::insert;
        using boost::mpl::is_sequence;
        using boost::mpl::identity;
        using boost::mpl::push_back;
        using boost::mpl::deref;
        using boost::mpl::distance;
        using boost::mpl::bool_;
        using boost::mpl::bind;
        using boost::mpl::true_;
        using boost::mpl::false_;
        using boost::mpl::and_;
        using boost::mpl::if_;
        using boost::mpl::eval_if;
        //using boost::mpl::_;
        using boost::mpl::_1;
        using boost::mpl::_2;
        using boost::mpl::contains;
        using boost::mpl::find;
        using boost::mpl::prior;
        using boost::mpl::apply;
        using boost::mpl::size;
        //using boost::mpl::insert_range;
        //using boost::mpl::quote1;
        //using boost::mpl::protect;
        using boost::mpl::remove_if;

        struct KeywordParameterTag {};

        template<class K, class V>
        struct KeywordBase: public KeywordParameterTag
        {
            typedef KeywordBase Base;
            typedef K This;
            typedef V Value;

            KwArg<This, Value> operator= (Value&& v) const { return{ std::move(v) }; }
            const KwArg<This, const Value&> operator= (const Value& v) const { return{ v }; }

            static const K instance;
        };

        template<class T>
        using decay_t = typename std::decay<T>::type;

        template<class K, class V>
        const K KeywordBase<K, V>::instance = {};

        template<class T, class Enabler = void>
        struct is_keyword: std::false_type {};

        template<class T>
        struct is_keyword<T, typename std::enable_if<std::is_base_of<KeywordParameterTag, T>::value>::type>: std::true_type{};
        
        template<class T>
        struct is_keywords_group: std::false_type {};

        template<class... T>
        struct is_keywords_group<set<T...>>: std::true_type{};

        template<class T>
        struct KeywordsGroup
        {
            typedef T type;

            static const KeywordsGroup instance;
        };

        template<class T>
        const KeywordsGroup<T> KeywordsGroup<T>::instance = {};

        struct keywords_of_impl
        {
            template <class T, class Enabler = void>
            struct apply;

            template <class T>
            struct apply<KeywordsGroup<T>>
            {
                typedef typename KeywordsGroup<T>::type type;
            };

            template <class T>
            struct apply<T, typename std::enable_if<is_keyword<T>::value>::type>
            {
                typedef vector<T> type;
            };
        };

        template<class... Keywords>
        struct keywords_of: transform<
            vector<decay_t<Keywords>...>,
            keywords_of_impl
        > {};

        template<class Sequence, class Range>
        struct assoc_insert_range: boost::mpl::fold<
            Range,
            Sequence,
            boost::mpl::insert<boost::mpl::_1, boost::mpl::_2>
        >
        {};

        template<class... Keywords>
        struct group_keywords: fold<
            typename keywords_of<Keywords...>::type,
            set<>,
            assoc_insert_range<_1, _2>
        > {};
        
        template<class... Keywords>
        KeywordsGroup<typename group_keywords<Keywords...>::type> make_keywords_group(Keywords...);

        struct arg_keyword
        {
            template <class Arg>
            struct apply
            {
                typedef typename decay_t<Arg>::first_type type;
            };
        };

        template<class... Args>
        struct arg_keywords: vector<
            typename apply<arg_keyword, Args>::type...
        > {};

        template<class... Args>
        struct unique_arg_keywords: fold<
            typename arg_keywords<Args...>::type,
            set<>,
            insert<_1, _2>
        > {};

        template<class KeywordGroup>
        struct not_in_group
        {
            template<class... Args>
            struct apply: remove_if<
                typename arg_keywords<Args...>::type,
                has_key<typename KeywordGroup::type, _1>
            > {};
        };

        template<class Sequence>
        struct unique_indexes_impl
        {
            typedef typename transform<
                Sequence,
                detail::arg_keyword
            >::type TypesVector_;

            typedef typename begin<TypesVector_>::type Begin_;

            typedef typename reverse_iter_fold <
                TypesVector_,
                pair<vector<>, set<>>,
                pair<
                    if_<
                        has_key<second<_1>, deref<_2>>,
                        first<_1>,
                        push_back<first<_1>, distance<Begin_, _2>>
                    >,
                    insert<
                        second<_1>,
                        deref<_2>
                    >
                >
            >::type::first ReversedIndexes;

            typedef typename reverse<ReversedIndexes>::type type;
        };

        template<class... Keywords>
        struct as_keywords_sequence
        {
            typedef vector<Keywords...> type;
        };

        template<class KeywordOrSequence>
        struct as_keywords_sequence<KeywordOrSequence>: eval_if<
            is_sequence<KeywordOrSequence>,
            identity<KeywordOrSequence>,
            vector<KeywordOrSequence>
        > {};

        template<class... Args>
        struct args_storage
        {
            typedef boost::fusion::vector<Args&...> type;
        };

        template<class ArgOrSequence>
        struct args_storage<ArgOrSequence>
        {
            typedef typename std::conditional<
                boost::fusion::traits::is_sequence<ArgOrSequence>::value,
                typename std::add_lvalue_reference<ArgOrSequence>::type,
                boost::fusion::vector<ArgOrSequence&>
            >::type type;
        };

        template<class... ArgsOrSequence>
        struct args_sequence_wrapper
        {
            typedef typename args_storage<ArgsOrSequence...>::type Kv;

            template<class... T>
            args_sequence_wrapper(T&... t): m_s(t...) {}

            template<class Index>
            auto operator()(Index) const -> decltype(boost::fusion::at<Index>(std::declval<const Kv&>()))
            {
                return boost::fusion::at<Index>(m_s);
            }

            template<class Index>
            auto operator()(Index) -> decltype(boost::fusion::at<Index>(std::declval<Kv&>()))
            {
                return boost::fusion::at<Index>(m_s);
            }

            Kv m_s;
        };

        template<class T>
        struct is_kwarg_impl: false_ {};

        template<class K, class V>
        struct is_kwarg_impl<KwArg<K, V>>: true_ {};

        template<class T>
        struct is_kwarg: is_kwarg_impl<
            typename std::remove_cv<
                typename std::remove_reference<T>::type
            >::type
        > {};

        template<class... Args>
        struct is_kwargs_impl: fold<
            vector<Args...>,
            true_,
            and_<
                is_kwarg<_2>,
                _1
            >
        > {};

        template<class... Args>
        struct is_kwargs: is_kwargs_impl<Args...>::type {};

        template<class Keyword, class... Args>
        struct has_keyword: contains<typename arg_keywords<Args...>::type, decay_t<Keyword>>::type {};

        template<class Keyword, class... Args>
        struct keyword_index
        {
            typedef typename reverse<typename arg_keywords<Args...>::type>::type TypesVector_;
            typedef typename prior<
                typename distance<
                    typename find<TypesVector_, Keyword>::type,
                    typename end<TypesVector_>::type
                >::type
            >::type type;
        };
    }

    template<class... KeywordsOrSequence>
    struct as_keywords_sequence: detail::as_keywords_sequence<KeywordsOrSequence...> {};

    template<class Arg>
    struct keyword_of
    {
        typedef typename detail::arg_keyword::apply<Arg>::type type;
    };

    template<class Arg>
    using keyword_of_t = typename keyword_of<Arg>::type;

    template<class... Args>
    struct is_unique_keywords: boost::mpl::bool_<
        boost::mpl::size<typename detail::arg_keywords<Args...>::type>::type::value ==
        boost::mpl::size<typename detail::unique_arg_keywords<Args...>::type>::type::value
    > {};

    template<class... ArgsOrSequence>
    struct unique_indexes: detail::unique_indexes_impl<typename as_keywords_sequence<ArgsOrSequence...>::type> {};

    template<class... ArgsOrSequence>
    using unique_indexes_t = typename unique_indexes<ArgsOrSequence...>::type;

    template<class... Args>
    inline auto unique_args(Args&... args) ->
        decltype(boost::fusion::transform(
            unique_indexes_t<Args...>(),
            detail::args_sequence_wrapper<Args...>(args...))
        )
    {
        return boost::fusion::transform(
            unique_indexes_t<Args...>(),
            detail::args_sequence_wrapper<Args...>(args...));
    }

    template<typename K, typename V>
    inline K get_keyword(const KwArg<K, V>& v)
    {
        return K();
    }

    template<typename K, typename V>
    inline typename std::add_const<
        typename std::remove_reference<V>::type
    >::type& get_value(const KwArg<K, V>& v)
    {
        return v.second;
    }

    template<typename K, typename V>
    inline typename std::remove_reference<V>::type& get_value(KwArg<K, V>& v)
    {
        return v.second;
    }

    template<class... Args>
    struct is_kwargs : std::integral_constant<bool, detail::is_kwargs<Args...>::value> {};

    template<class Keyword, class... Args>
    struct has_keyword : std::integral_constant<bool, detail::has_keyword<Keyword, Args...>::value> {};

    template<class... Args>
    using enable_if_kwargs_t = typename std::enable_if<is_kwargs<Args...>::value>::type;

    template<class Keyword, class... Args>
    inline typename std::enable_if<
        is_kwargs<Args...>::value,
        typename Keyword::Value
    >::type get(const Keyword&, Args&&... args)
    {
        // TODO: check forwarding
        static_assert(has_keyword<Keyword, Args...>::value, "Required keyword is not present");
        auto a = boost::fusion::vector_tie(args...);
        return std::move(get_value(boost::fusion::at<typename detail::keyword_index<Keyword, Args...>::type>(a)));
    }

    template<class Keyword, class Default, class... Args>
    inline typename std::enable_if<
        is_kwargs<Args...>::value && has_keyword<Keyword, Args...>::value,
        typename Keyword::Value
    >::type get_opt(const Keyword&, Default&&, Args&&... args)
    {
        // TODO: check forwarding
        auto a = boost::fusion::vector_tie(args...);
        return std::move(get_value(boost::fusion::at<typename detail::keyword_index<Keyword, Args...>::type>(a)));
    }

    template<class Keyword, class Default, class... Args>
    inline typename std::enable_if<
        is_kwargs<Args...>::value && !has_keyword<Keyword, Args...>::value,
        Default&&
    >::type get_opt(const Keyword&, Default&& d, Args&&...)
    {
        return std::forward<Default>(d);
    }
}

#define KWARGS__UNFOLD(...) __VA_ARGS__
#define KWARGS__CLS_NAME(keyword) BOOST_PP_CAT(keyword, _keyword__)
#define KWARGS__CLS_GROUP_NAME(group) BOOST_PP_CAT(group, _group__)

// Define keyword parameter of specified type.
// Ex: KWARGS_KEYWORD(url, std::string);
#define KWARGS_KEYWORD(keyword, type)                                   \
struct KWARGS__CLS_NAME(keyword)                                        \
    : kwargs::detail::KeywordBase<KWARGS__CLS_NAME(keyword), type> {    \
    using Base::operator=;                                              \
};                                                                      \
static const KWARGS__CLS_NAME(keyword)& keyword = KWARGS__CLS_NAME(keyword)::instance;

// Get unique type of keyword
#define KWARGS_KW_TAG(keyword) kwargs::detail::decay_t<decltype(keyword)>

// Define keywords group. Keywords must be specified in brackets.
// Ex: KWARGS_KEYWORDS_GROUP(get_request, (url, seconds_to_wait))
#define KWARGS_KEYWORDS_GROUP(group, keywords)                                                                      \
    typedef decltype(kwargs::detail::make_keywords_group(KWARGS__UNFOLD keywords)) KWARGS__CLS_GROUP_NAME(group);   \
    static const KWARGS__CLS_GROUP_NAME(group)& group = KWARGS__CLS_GROUP_NAME(group)::instance;

// Check arguments contain no duplicated keywords.
// Ex: KWARGS_CHECK_UNIQUE(Args)
#define KWARGS_CHECK_UNIQUE(args) \
    BOOST_MPL_ASSERT_MSG(kwargs::is_unique_keywords<args...>::value, KEYWORD_PARAMETER_PASSED_MORE_THAN_ONCE, (args...));

// Check passed argument types are a subset of specified keywords list. The list can include groups as well.
// Ex: KWARGS_CHECK_IN_LIST(Args, (token, get_request))
#define KWARGS_CHECK_IN_LIST(args, keywords) BOOST_MPL_ASSERT_MSG(                  \
    !boost::mpl::size<                                                              \
        typename kwargs::detail::not_in_group<                                      \
            decltype(kwargs::detail::make_keywords_group(KWARGS__UNFOLD keywords))  \
        >::apply<args...>::type                                                     \
    >::value,                                                                       \
    WRONG_KEYWORD_PARAMETER_PASSED,                                                 \
    (args...));
