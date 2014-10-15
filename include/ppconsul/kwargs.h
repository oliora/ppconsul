//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
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
#include <boost/mpl/front.hpp>

#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/mpl.hpp>

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/cat.hpp>

#include <type_traits>
#include <utility>


namespace kwargs {

    template<typename K, typename V>
    using KwArg = boost::fusion::pair<K, V>;

    namespace detail {

        using namespace boost::mpl;

        struct KeywordParameterTag {};

        template<class K, class V>
        struct KeywordBase: public KeywordParameterTag
        {
            typedef KeywordBase Base;
            typedef K This;
            typedef V Value;

            KwArg<This, Value> operator= (Value&& v) { return{ std::move(v) }; }
            KwArg<This, const Value&> operator= (const Value& v) { return{ v }; }
        };

        struct get_keyword
        {
            template <class Arg>
            struct apply
            {
                typedef typename std::remove_reference<Arg>::type::first_type type;
            };
        };

        template<class... Args>
        using get_keywords_t = typename transform<
            vector<Args...>,
            get_keyword
        >::type;

        template<class... Args>
        using get_unique_keywords_t = typename fold<
            get_keywords_t<Args...>,
            set0<>,
            insert<_1, _2>
        >::type;

        template<class... Keywords>
        using keyword_group_t = set<Keywords...>;

        template<class KeywordGroup>
        struct check_keyword_in_group
        {
            template <class Arg>
            struct apply: has_key<KeywordGroup, Arg>
            {
                BOOST_MPL_ASSERT_MSG(apply::value, WRONG_KEYWORD_PARAMETER_PASSED, (types<Arg, KeywordGroup>));
            };
        };

        template<class KeywordGroup>
        struct check_keywords_in_group
        {
            template<class... Args>
            struct apply
                : transform<get_keywords_t<Args...>, check_keyword_in_group<KeywordGroup>> {};
        };

        template<class... Keywords>
        keyword_group_t<Keywords...> group_keywords(Keywords...);

        template<class Sequence>
        struct unique_indexes_impl
        {
            typedef typename transform<
                Sequence,
                detail::get_keyword
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
            typedef boost::mpl::vector<Keywords...> type;
        };

        template<class KeywordOrSequence>
        struct as_keywords_sequence<KeywordOrSequence>: boost::mpl::eval_if<
            boost::mpl::is_sequence<KeywordOrSequence>,
            boost::mpl::identity<KeywordOrSequence>,
            boost::mpl::vector<KeywordOrSequence>
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
            typedef typename args_storage<ArgsOrSequence...>::type Storage;

            template<class... T>
            args_sequence_wrapper(T&... t): m_s(t...) {}

            template<class Index>
            auto operator()(Index) const -> decltype(boost::fusion::at<Index>(std::declval<const Storage&>()))
            {
                return boost::fusion::at<Index>(m_s);
            }

            template<class Index>
            auto operator()(Index) -> decltype(boost::fusion::at<Index>(std::declval<Storage&>()))
            {
                return boost::fusion::at<Index>(m_s);
            }

            Storage m_s;
        };
    }

    //template<class T, class Enabler = void>
    //struct is_parameter: std::false_type {};
    //
    //template<class P, class V>
    //struct is_parameter<KeywordArgument<P, V>, typename std::enable_if<std::is_base_of<KeywordParameterTag, P>::value>::type>: std::true_type{};

    template<class... KeywordsOrSequence>
    struct as_keywords_sequence: detail::as_keywords_sequence<KeywordsOrSequence...> {};

    template<class Arg>
    struct keyword_of
    {
        typedef typename detail::get_keyword::apply<Arg>::type type;
    };

    template<class Arg>
    using keyword_of_t = typename keyword_of<Arg>::type;

    template<class... Args>
    struct is_unique_keywords: boost::mpl::bool_<
        boost::mpl::size<detail::get_keywords_t<Args...>>::type::value ==
        boost::mpl::size<detail::get_unique_keywords_t<Args...>>::type::value
    > {};

    template<class... ArgsOrSequence>
    struct unique_indexes: detail::unique_indexes_impl<typename as_keywords_sequence<ArgsOrSequence...>::type> {};

    template<class... ArgsOrSequence>
    using unique_indexes_t = typename unique_indexes<ArgsOrSequence...>::type;

    template<class... Args>
    auto unique_args(Args&... args) ->
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
    K get_keyword(const KwArg<K, V>& v)
    {
        return K();
    }

    /*template<typename K, typename V>
    typename std::remove_reference<V>::type& get_value(KwArg<K, V>&& v)
    {
        return v.second;
    }*/

    // TODO: allow access to the rvalue references and non-const lvalue references

    template<typename K, typename V>
    typename std::add_const<
        typename std::remove_reference<V>::type
    >::type& get_value(const KwArg<K, V>& v)
    {
        return v.second;
    }
}

#define KWARGS__UNFOLD(...) __VA_ARGS__
#define KWARGS__CLS_NAME(keyword) BOOST_PP_CAT(keyword, _keyword_)

// Define keyword parameter of specified type.
// Ex: KWARGS_KEYWORD(url, std::string)
#define KWARGS_KEYWORD(keyword, type)                                   \
struct KWARGS__CLS_NAME(keyword)                                        \
    : kwargs::detail::KeywordBase<KWARGS__CLS_NAME(keyword), type> {    \
    using Base::operator=;                                              \
} keyword;

// Get unique type of keyword. `decltype(keyword)`
#define KWARGS_KW_TAG(keyword) decltype(keyword)

// Define keywords group. Keywords must be specified in brackets.
// Ex: KWARGS_KEYWORDS_GROUP(get_request, (url, seconds_to_wait))
#define KWARGS_KEYWORDS_GROUP(group, keywords) \
    typedef decltype(kwargs::detail::group_keywords(KWARGS__UNFOLD keywords)) group;

// Check arguments contain no duplicated keywords.
// Ex: KWARGS_CHECK_UNIQUE(Args)
#define KWARGS_CHECK_UNIQUE(args) \
    BOOST_MPL_ASSERT_MSG(kwargs::is_unique_keywords<args...>::value, KEYWORD_PARAMETER_PASSED_MORE_THAN_ONCE, (types<args...>));

// Check passed argument types are a subset of specified keywords group (previously defined with KWARGS_KEYWORDS_GROUP)
// Ex: KWARGS_CHECK_IN_GROUP(Args, get_request);
#define KWARGS_CHECK_IN_GROUP(args, group) \
    { typename kwargs::detail::check_keywords_in_group<group>::apply<args...>::type t_; (void)t_; }

// Check passed argument types are a subset of specified list of keywords. Keywords must be specified in brackets.
// Ex: KWARGS_CHECK_IN_LIST(Args, (url, seconds_to_wait));
#define KWARGS_CHECK_IN_LIST(args, keywords) \
    KWARGS_CHECK_IN_GROUP(args, decltype(kwargs::detail::group_keywords(KWARGS__UNFOLD keywords)))
