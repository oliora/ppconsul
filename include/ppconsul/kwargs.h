//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/reverse_iter_fold.hpp>
#include <boost/mpl/reverse.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/as_sequence.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/front.hpp>

#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/vector.hpp>

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/cat.hpp>

#include <type_traits>
#include <utility>


namespace ppconsul { namespace param {
    namespace detail {

        using namespace boost::mpl;

        template<typename P, typename V>
        using Pair = boost::fusion::pair<P, V>;

        struct KeywordParameterTag {};

        template<class K, class V>
        struct KeywordBase: public KeywordParameterTag
        {
            typedef KeywordBase Base;
            typedef K This;
            typedef V Value;

            Pair<This, Value> operator= (Value&& v) { return{ std::move(v) }; }
            Pair<This, const Value&> operator= (const Value& v) { return{ v }; }
        };

        struct get_parameter_type
        {
            template <class T>
            struct apply
            {
                typedef typename std::remove_reference<T>::type::first_type type;
            };
        };

        template<class... Parameters>
        using parameter_types_t = typename boost::mpl::transform<
            boost::mpl::vector<Parameters...>,
            get_parameter_type
        >::type;

        template<class... Parameters>
        using unique_parameter_types_t = typename boost::mpl::fold<
            parameter_types_t<Parameters...>,
            boost::mpl::set0<>,
            boost::mpl::insert<boost::mpl::_1, boost::mpl::_2>
        >::type;

        template<class... Keywords>
        using keyword_group_t = boost::mpl::set<Keywords...>;

        template<class KeywordGroup>
        struct check_parameter_in_group
        {
            template <class T>
            struct apply
                : boost::mpl::has_key<KeywordGroup, T>
            {
                BOOST_MPL_ASSERT_MSG(apply::value, WRONG_KEYWORD_PARAMETER_PASSED, (types<T, KeywordGroup>));
            };
        };

        template<class KeywordGroup>
        struct check_parameters_in_group
        {
            template<class... Parameters>
            struct apply
                : boost::mpl::transform<parameter_types_t<Parameters...>, check_parameter_in_group<KeywordGroup>> {};
        };

        template<class... Keywords>
        keyword_group_t<Keywords...> group_keywords(Keywords...);

        template<class Sequence>
        struct unique_indexes_impl
        {
            typedef typename transform<
                Sequence,
                detail::get_parameter_type
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

        template<class... Ts>
        struct as_sequence_impl
        {
            typedef boost::mpl::vector<Ts...> type;
        };

        template<class T>
        struct as_sequence_impl<T>: boost::mpl::eval_if<
            boost::mpl::is_sequence<T>,
            boost::mpl::identity<T>,
            boost::mpl::vector<T>
        > {};

        template<class... Ts>
        struct fusion_sequence_wrapper_storage
        {
            typedef boost::fusion::vector<Ts&...> type;
        };

        template<class T>
        struct fusion_sequence_wrapper_storage<T>
        {
            typedef typename std::conditional<
                boost::fusion::traits::is_sequence<T>::value,
                typename std::add_lvalue_reference<T>::type,
                boost::fusion::vector<T&>
            >::type type;
        };

        template<class... Ts>
        struct fusion_sequence_wrapper
        {
            typedef typename fusion_sequence_wrapper_storage<Ts...>::type Storage;

            template<class... Ts2>
            fusion_sequence_wrapper(Ts2&... ts): m_s(ts...) {}

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
    //struct is_parameter<Pair<P, V>, typename std::enable_if<std::is_base_of<KeywordParameterTag, P>::value>::type>: std::true_type{};

    template<class... Parameters>
    struct as_sequence: detail::as_sequence_impl<Parameters...> {};

    template<class Parameter>
    using parameter_keyword_t = typename detail::get_parameter_type::apply<Parameter>::type;

    template<class... Parameters>
    struct is_parameters_unique: boost::mpl::bool_<
        boost::mpl::size<detail::parameter_types_t<Parameters...>>::type::value ==
        boost::mpl::size<detail::unique_parameter_types_t<Parameters...>>::type::value
    > {};

    template<class... Parameters>
    struct unique_indexes: detail::unique_indexes_impl<typename as_sequence<Parameters...>::type> {};

    template<class... Parameters>
    using unique_indexes_t = typename unique_indexes<Parameters...>::type;

    template<class... Parameters>
    auto unique_parameters(Parameters&... params) ->
        decltype(boost::fusion::transform(
            unique_indexes_t<Parameters...>(),
            detail::fusion_sequence_wrapper<Parameters...>(params...))
        )
    {
        return boost::fusion::transform(
            unique_indexes_t<Parameters...>(),
            detail::fusion_sequence_wrapper<Parameters...>(params...));
    }

    template<typename P, typename V>
    inline V parameter_value(detail::Pair<P, V>&& v)
    {
        return std::forward<V>(v.second);
    }

    template<typename P, typename V>
    inline const V& parameter_value(const detail::Pair<P, V>& v)
    {
        return v.second;
    }
}}

#define PPCONSUL_PARAM__UNFOLD(...) __VA_ARGS__
#define PPCONSUL_PARAM__CLS_NAME(keyword) BOOST_PP_CAT(keyword, _keyword_)

// Define keyword parameter of specified type.
// Ex: PPCONSUL_PARAM(url, std::string)
#define PPCONSUL_PARAM(keyword, type)                                                   \
struct PPCONSUL_PARAM__CLS_NAME(keyword)                                                \
    : ppconsul::param::detail::KeywordBase<PPCONSUL_PARAM__CLS_NAME(keyword), type>{    \
    using Base::operator=;                                                              \
} keyword;

// Define keyword parameter of specified type with specified string name attached.
// Note that the name can be retrieved with PPCONSUL_PARAM_NAME(keyword) macro.
// Ex: PPCONSUL_PARAM_NAMED(url, std::string, "addr")
#define PPCONSUL_PARAM_NAMED(keyword, type, name_)  \
    PPCONSUL_PARAM(keyword, type)                   \
    inline const char *parameter_name(PPCONSUL_PARAM__CLS_NAME(keyword)) { return name_; }

// Same as PPCONSUL_PARAM_NAMED(keyword, type "<keyword>")
// Ex: PPCONSUL_PARAM_DEF_NAMED(url, std::string)
#define PPCONSUL_PARAM_DEF_NAMED(keyword, type) PPCONSUL_PARAM_NAMED(keyword, type, BOOST_PP_STRINGIZE(keyword))

// Get name of keyword parameter. Compilation error if parameter was defined without the name.
// Ex: PPCONSUL_PARAM_NAME(url)
#define PPCONSUL_PARAM_NAME(keyword_cls) \
    parameter_name(keyword_cls)

// Get keyword type. Same as decltype(keyword)
#define PPCONSUL_PARAM_KW_TYPE(keyword) \
    decltype(keyword)

// Define keyword parameters group. keywords must be specified in brackets.
// Ex: PPCONSUL_PARAMETER_GROUP(get_request, (url, seconds_to_wait))
#define PPCONSUL_PARAM_GROUP(group, keywords) \
    typedef decltype(ppconsul::param::detail::group_keywords(PPCONSUL_PARAM__UNFOLD keywords)) group;

// Check parameters contain no duplicated parameters.
// Ex: PPCONSUL_PARAM_CHECK_UNIQUE(Parameters)
#define PPCONSUL_PARAM_CHECK_UNIQUE(parameters)                      \
    BOOST_MPL_ASSERT_MSG(                                            \
        ppconsul::param::is_parameters_unique<parameters...>::value, \
        KEYWORD_PARAMETER_PASSED_MORE_THAN_ONCE,                     \
        (types<parameters...>));

// Check parameters contains no duplicated parameters and is a subset of specified keyword parameters group (defined with PPCONSUL_PARAMETER_GROUP)
// Ex: PPCONSUL_PARAMETER_CHECK_GROUP(Parameters, get_request);
#define PPCONSUL_PARAM_CHECK_GROUP(parameters, keywords_group) \
    { typename ppconsul::param::detail::check_parameters_in_group<keywords_group>::apply<parameters...>::type t_; (void)t_; }

// Check parameters contains no duplicated parameters and is a subset of specified keywords set. keywords must be specified in brackets.
// Ex: PPCONSUL_PARAMETER_CHECK(Parameters, (url, seconds_to_wait));
#define PPCONSUL_PARAM_CHECK(parameters, keywords) \
    PPCONSUL_PARAM_CHECK_GROUP(parameters, decltype(ppconsul::param::detail::group_keywords(PPCONSUL_PARAM__UNFOLD keywords)))
