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
#include <boost/mpl/contains.hpp>
#include <boost/mpl/front.hpp>

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
        using boost::mpl::true_;
        using boost::mpl::false_;
        using boost::mpl::and_;
        using boost::mpl::if_;
        using boost::mpl::eval_if;
        using boost::mpl::_1;
        using boost::mpl::_2;
        using boost::mpl::contains;
        using boost::mpl::find;
        using boost::mpl::prior;

        struct KeywordParameterTag {};

        template<class K, class V>
        struct KeywordBase: public KeywordParameterTag
        {
            typedef KeywordBase Base;
            typedef K This;
            typedef V Value;

            KwArg<This, Value> operator= (Value&& v) const { return{ std::move(v) }; }
            const KwArg<This, const Value&> operator= (const Value& v) const { return{ v }; }
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
        struct get_keywords: transform<
            vector<Args...>,
            get_keyword
        > {};

        template<class... Args>
        struct get_unique_keywords: fold<
            typename get_keywords<Args...>::type,
            set<>,
            insert<_1, _2>
        > {};

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
                : transform<typename get_keywords<Args...>::type, check_keyword_in_group<KeywordGroup>> {};
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
        struct has_keyword: contains<typename get_keywords<Args...>::type, Keyword>::type {};

        template<class Keyword, class... Args>
        struct keyword_index
        {
            typedef typename reverse<typename get_keywords<Args...>::type>::type TypesVector_;
            typedef typename prior<
                typename distance<
                    typename find<TypesVector_, Keyword>::type,
                    typename end<TypesVector_>::type
                >::type
            >::type type;
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
        boost::mpl::size<typename detail::get_keywords<Args...>::type>::type::value ==
        boost::mpl::size<typename detail::get_unique_keywords<Args...>::type>::type::value
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
        return std::move(v.second);
    }

    using detail::is_kwargs;
    using detail::has_keyword;

    template<class... Args>
    using enable_if_kwargs_t = typename std::enable_if<is_kwargs<Args...>::value>::type;

    /*template<class Keyword, class... Args>
    inline typename std::enable_if<is_kwargs<Args...>::value, typename Keyword::Value>::type
        get(Keyword, Args&&... args)
    {
        auto a = boost::fusion::vector_tie(args...);
        return std::move(get_value(boost::fusion::at<typename detail::keyword_index<Keyword, Args...>::type>(a)));
    }*/

    template<class Keyword, class Default, class... Args>
    inline typename std::enable_if<
        is_kwargs<Args...>::value && has_keyword<Keyword, Args...>::value,
        typename Keyword::Value
    >::type get(Keyword, Default&&, Args&&... args)
    {
        auto a = boost::fusion::vector_tie(args...);
        return std::move(get_value(boost::fusion::at<typename detail::keyword_index<Keyword, Args...>::type>(a)));
    }

    template<class Keyword, class Default, class... Args>
    inline typename std::enable_if<
        is_kwargs<Args...>::value && !detail::has_keyword<Keyword, Args...>::value,
        Default&&
    >::type get(Keyword, Default&& d, Args&&...)
    {
        return std::forward<Default>(d);
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
} const keyword = KWARGS__CLS_NAME(keyword){};

// Get unique type of keyword. `decltype(keyword)`
#define KWARGS_KW_TAG(keyword) decltype(keyword)

// Define keywords group. Keywords must be specified in brackets.
// Ex: KWARGS_KEYWORDS_GROUP(get_request, (url, seconds_to_wait))
#define KWARGS_KEYWORDS_GROUP(group, keywords) \
    typedef decltype(kwargs::detail::group_keywords(KWARGS__UNFOLD keywords)) group;

// Check arguments contain no duplicated keywords.
// Ex: KWARGS_CHECK_UNIQUE(Args)
#define KWARGS_CHECK_UNIQUE(args) \
    BOOST_MPL_ASSERT_MSG(kwargs::is_unique_keywords<args...>::value, KEYWORD_PARAMETER_PASSED_MORE_THAN_ONCE, (args...));

// Check passed argument types are a subset of specified keywords group (previously defined with KWARGS_KEYWORDS_GROUP)
// Ex: KWARGS_CHECK_IN_GROUP(Args, get_request)
#define KWARGS_CHECK_IN_GROUP(args, group) \
    { typename kwargs::detail::check_keywords_in_group<group>::apply<args...>::type t_; (void)t_; }

// Check passed argument types are a subset of specified list of keywords. Keywords must be specified in brackets.
// Ex: KWARGS_CHECK_IN_LIST(Args, (url, seconds_to_wait))
#define KWARGS_CHECK_IN_LIST(args, keywords) \
    KWARGS_CHECK_IN_GROUP(args, decltype(kwargs::detail::group_keywords(KWARGS__UNFOLD keywords)))
