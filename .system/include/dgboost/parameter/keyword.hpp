// Copyright Daniel Wallin, David Abrahams 2005.
// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef KEYWORD_050328_HPP
#define KEYWORD_050328_HPP

#include <dgboost/parameter/aux_/tag.hpp>
#include <dgboost/parameter/aux_/default.hpp>
#include <dgboost/parameter/keyword_fwd.hpp>
#include <dgboost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
#include <dgboost/core/enable_if.hpp>
#include <utility>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mp11/integral.hpp>
#include <dgboost/mp11/utility.hpp>
#include <type_traits>
#else
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/type_traits/is_same.hpp>
#include <dgboost/type_traits/is_scalar.hpp>
#include <dgboost/type_traits/is_const.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter {

    // Instances of unique specializations of keyword<...> serve to
    // associate arguments with parameter names.  For example:
    //
    //     struct rate_;             // parameter names
    //     struct skew_;
    //
    //     namespace
    //     {
    //         keyword<rate_> rate;  // keywords
    //         keyword<skew_> skew;
    //     }
    //
    //     ...
    //
    //     f(rate = 1, skew = 2.4);
    template <typename Tag>
    struct keyword
    {
        typedef Tag tag;

        inline BOOST_CONSTEXPR keyword()
        {
        }

        template <typename T>
        inline BOOST_CONSTEXPR typename ::dgboost::lazy_enable_if<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::dgboost::mp11::mp_if<
                ::std::is_scalar<T>
              , ::dgboost::mp11::mp_true
              , ::dgboost::mp11::mp_if<
                    ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mp11::mp_true
                  , ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >
            >
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_scalar<T>
              , ::dgboost::mpl::true_
              , ::dgboost::mpl::eval_if<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::mpl::if_<
                        ::dgboost::is_same<
                            typename Tag::qualifier
                          , ::dgboost::parameter::forward_reference
                        >
                      , ::dgboost::mpl::true_
                      , ::dgboost::mpl::false_
                    >
                >
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , ::dgboost::parameter::aux::tag<Tag,T const&>
        >::type
            operator=(T const& x) const
        {
            typedef typename ::dgboost::parameter::aux
            ::tag<Tag,T const&>::type result;
            return result(x);
        }

        template <typename Default>
        inline BOOST_CONSTEXPR typename ::dgboost::enable_if<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::dgboost::mp11::mp_if<
                ::std::is_scalar<Default>
              , ::dgboost::mp11::mp_true
              , ::dgboost::mp11::mp_if<
                    ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mp11::mp_true
                  , ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >
            >
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_scalar<Default>
              , ::dgboost::mpl::true_
              , ::dgboost::mpl::eval_if<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::mpl::if_<
                        ::dgboost::is_same<
                            typename Tag::qualifier
                          , ::dgboost::parameter::forward_reference
                        >
                      , ::dgboost::mpl::true_
                      , ::dgboost::mpl::false_
                    >
                >
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , ::dgboost::parameter::aux::default_<Tag,Default const>
        >::type
            operator|(Default const& d) const
        {
            return ::dgboost::parameter::aux::default_<Tag,Default const>(d);
        }

        template <typename T>
        inline BOOST_CONSTEXPR typename ::dgboost::lazy_enable_if<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::dgboost::mp11::mp_if<
                ::dgboost::mp11::mp_if<
                    ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::out_reference
                    >
                  , ::dgboost::mp11::mp_true
                  , ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >
              , ::dgboost::mp11::mp_if<
                    ::std::is_const<T>
                  , ::dgboost::mp11::mp_false
                  , ::dgboost::mp11::mp_true
                >
              , ::dgboost::mp11::mp_false
            >
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
            typename ::dgboost::mpl::eval_if<
                typename ::dgboost::mpl::if_<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::out_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >::type
              , ::dgboost::mpl::if_<
                    ::dgboost::is_const<T>
                  , ::dgboost::mpl::false_
                  , ::dgboost::mpl::true_
                >
              , ::dgboost::mpl::false_
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , ::dgboost::parameter::aux::tag<Tag,T&>
        >::type
            operator=(T& x) const
        {
            typedef typename ::dgboost::parameter::aux
            ::tag<Tag,T&>::type result;
            return result(x);
        }

        template <typename Default>
        inline BOOST_CONSTEXPR typename ::dgboost::enable_if<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::dgboost::mp11::mp_if<
                ::dgboost::mp11::mp_if<
                    ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::out_reference
                    >
                  , ::dgboost::mp11::mp_true
                  , ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >
              , ::dgboost::mp11::mp_if<
                    ::std::is_const<Default>
                  , ::dgboost::mp11::mp_false
                  , ::dgboost::mp11::mp_true
                >
              , ::dgboost::mp11::mp_false
            >
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
            typename ::dgboost::mpl::eval_if<
                typename ::dgboost::mpl::if_<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::out_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >::type
              , ::dgboost::mpl::if_<
                    ::dgboost::is_const<Default>
                  , ::dgboost::mpl::false_
                  , ::dgboost::mpl::true_
                >
              , ::dgboost::mpl::false_
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , ::dgboost::parameter::aux::default_<Tag,Default>
        >::type
            operator|(Default& d) const
        {
            return ::dgboost::parameter::aux::default_<Tag,Default>(d);
        }

        template <typename Default>
        inline BOOST_CONSTEXPR
        ::dgboost::parameter::aux::lazy_default<Tag,Default const>
            operator||(Default const& d) const
        {
            return ::dgboost::parameter::aux
            ::lazy_default<Tag,Default const>(d);
        }

        template <typename Default>
        inline BOOST_CONSTEXPR
        ::dgboost::parameter::aux::lazy_default<Tag,Default>
            operator||(Default& d) const
        {
            return ::dgboost::parameter::aux::lazy_default<Tag,Default>(d);
        }

        template <typename T>
        inline BOOST_CONSTEXPR typename ::dgboost::lazy_enable_if<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::dgboost::mp11::mp_if<
                ::std::is_scalar<T>
              , ::dgboost::mp11::mp_false
              , ::dgboost::mp11::mp_if<
                    ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mp11::mp_true
                  , ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >
            >
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_scalar<T>
              , ::dgboost::mpl::false_
              , ::dgboost::mpl::eval_if<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::mpl::if_<
                        ::dgboost::is_same<
                            typename Tag::qualifier
                          , ::dgboost::parameter::forward_reference
                        >
                      , ::dgboost::mpl::true_
                      , ::dgboost::mpl::false_
                    >
                >
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , ::dgboost::parameter::aux::tag<Tag,T const>
        >::type
            operator=(T const&& x) const
        {
            typedef typename ::dgboost::parameter::aux
            ::tag<Tag,T const>::type result;
            return result(::std::forward<T const>(x));
        }

        template <typename T>
        inline BOOST_CONSTEXPR typename ::dgboost::lazy_enable_if<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::dgboost::mp11::mp_if<
                ::std::is_scalar<T>
              , ::dgboost::mp11::mp_false
              , ::dgboost::mp11::mp_if<
                    ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::consume_reference
                    >
                  , ::dgboost::mp11::mp_true
                  , ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >
            >
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_scalar<T>
              , ::dgboost::mpl::false_
              , ::dgboost::mpl::eval_if<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::consume_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::mpl::if_<
                        ::dgboost::is_same<
                            typename Tag::qualifier
                          , ::dgboost::parameter::forward_reference
                        >
                      , ::dgboost::mpl::true_
                      , ::dgboost::mpl::false_
                    >
                >
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , ::dgboost::parameter::aux::tag<Tag,T>
        >::type
            operator=(T&& x) const
        {
            typedef typename ::dgboost::parameter::aux::tag<Tag,T>::type result;
            return result(::std::forward<T>(x));
        }

        template <typename Default>
        inline BOOST_CONSTEXPR typename ::dgboost::enable_if<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::dgboost::mp11::mp_if<
                ::std::is_scalar<Default>
              , ::dgboost::mp11::mp_false
              , ::dgboost::mp11::mp_if<
                    ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mp11::mp_true
                  , ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >
            >
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_scalar<Default>
              , ::dgboost::mpl::false_
              , ::dgboost::mpl::eval_if<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::mpl::if_<
                        ::dgboost::is_same<
                            typename Tag::qualifier
                          , ::dgboost::parameter::forward_reference
                        >
                      , ::dgboost::mpl::true_
                      , ::dgboost::mpl::false_
                    >
                >
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , ::dgboost::parameter::aux::default_r_<Tag,Default const>
        >::type
            operator|(Default const&& d) const
        {
            return ::dgboost::parameter::aux::default_r_<Tag,Default const>(
                ::std::forward<Default const>(d)
            );
        }

        template <typename Default>
        inline BOOST_CONSTEXPR typename ::dgboost::enable_if<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::dgboost::mp11::mp_if<
                ::std::is_scalar<Default>
              , ::dgboost::mp11::mp_false
              , ::dgboost::mp11::mp_if<
                    ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::consume_reference
                    >
                  , ::dgboost::mp11::mp_true
                  , ::std::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >
            >
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_scalar<Default>
              , ::dgboost::mpl::false_
              , ::dgboost::mpl::eval_if<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::consume_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::mpl::if_<
                        ::dgboost::is_same<
                            typename Tag::qualifier
                          , ::dgboost::parameter::forward_reference
                        >
                      , ::dgboost::mpl::true_
                      , ::dgboost::mpl::false_
                    >
                >
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , ::dgboost::parameter::aux::default_r_<Tag,Default>
        >::type
            operator|(Default&& d) const
        {
            return ::dgboost::parameter::aux
            ::default_r_<Tag,Default>(::std::forward<Default>(d));
        }

     public: // Insurance against ODR violations
        // Users will need to define their keywords in header files.  To
        // prevent ODR violations, it's important that the keyword used in
        // every instantiation of a function template is the same object.
        // We provide a reference to a common instance of each keyword
        // object and prevent construction by users.
        static ::dgboost::parameter::keyword<Tag> const instance;

        // This interface is deprecated.
        static ::dgboost::parameter::keyword<Tag>& get()
        {
            return const_cast< ::dgboost::parameter::keyword<Tag>&>(instance);
        }
    };

    template <typename Tag>
    ::dgboost::parameter::keyword<Tag> const ::dgboost::parameter
    ::keyword<Tag>::instance = ::dgboost::parameter::keyword<Tag>();
}} // namespace dgboost::parameter

#else   // !defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)

#if !defined(BOOST_NO_SFINAE)
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/core/enable_if.hpp>
#include <dgboost/type_traits/is_same.hpp>
#include <dgboost/type_traits/is_scalar.hpp>
#include <dgboost/type_traits/is_const.hpp>
#endif  // BOOST_NO_SFINAE

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter {

    // Instances of unique specializations of keyword<...> serve to
    // associate arguments with parameter names.  For example:
    //
    //     struct rate_;             // parameter names
    //     struct skew_;
    //
    //     namespace
    //     {
    //         keyword<rate_> rate;  // keywords
    //         keyword<skew_> skew;
    //     }
    //
    //     ...
    //
    //     f(rate = 1, skew = 2.4);
    template <typename Tag>
    struct keyword
    {
        typedef Tag tag;

        inline BOOST_CONSTEXPR keyword()
        {
        }

        template <typename T>
#if defined(BOOST_NO_SFINAE)
        inline typename ::dgboost::parameter::aux::tag<Tag,T const&>::type
#else
        inline BOOST_CONSTEXPR typename ::dgboost::lazy_enable_if<
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_scalar<T>
              , ::dgboost::mpl::true_
              , ::dgboost::mpl::eval_if<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::mpl::if_<
                        ::dgboost::is_same<
                            typename Tag::qualifier
                          , ::dgboost::parameter::forward_reference
                        >
                      , ::dgboost::mpl::true_
                      , ::dgboost::mpl::false_
                    >
                >
            >::type
          , ::dgboost::parameter::aux::tag<Tag,T const&>
        >::type
#endif  // BOOST_NO_SFINAE
            operator=(T const& x) const
        {
            typedef typename ::dgboost::parameter::aux
            ::tag<Tag,T const&>::type result;
            return result(x);
        }

        template <typename Default>
#if defined(BOOST_NO_SFINAE)
        inline ::dgboost::parameter::aux::default_<Tag,Default const>
#else
        inline BOOST_CONSTEXPR typename ::dgboost::enable_if<
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_scalar<Default>
              , ::dgboost::mpl::true_
              , ::dgboost::mpl::eval_if<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::in_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::mpl::if_<
                        ::dgboost::is_same<
                            typename Tag::qualifier
                          , ::dgboost::parameter::forward_reference
                        >
                      , ::dgboost::mpl::true_
                      , ::dgboost::mpl::false_
                    >
                >
            >::type
          , ::dgboost::parameter::aux::default_<Tag,Default const>
        >::type
#endif  // BOOST_NO_SFINAE
            operator|(Default const& d) const
        {
            return ::dgboost::parameter::aux::default_<Tag,Default const>(d);
        }

        template <typename T>
#if defined(BOOST_NO_SFINAE)
        inline typename ::dgboost::parameter::aux::tag<Tag,T&>::type
#else
        inline BOOST_CONSTEXPR typename ::dgboost::lazy_enable_if<
            typename ::dgboost::mpl::eval_if<
                typename ::dgboost::mpl::if_<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::out_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >::type
              , ::dgboost::mpl::if_<
                    ::dgboost::is_const<T>
                  , ::dgboost::mpl::false_
                  , ::dgboost::mpl::true_
                >
              , ::dgboost::mpl::false_
            >::type
          , ::dgboost::parameter::aux::tag<Tag,T&>
        >::type
#endif  // BOOST_NO_SFINAE
            operator=(T& x) const
        {
            typedef typename ::dgboost::parameter::aux
            ::tag<Tag,T&>::type result;
            return result(x);
        }

        template <typename Default>
#if defined(BOOST_NO_SFINAE)
        inline ::dgboost::parameter::aux::default_<Tag,Default>
#else
        inline BOOST_CONSTEXPR typename ::dgboost::enable_if<
            typename ::dgboost::mpl::eval_if<
                typename ::dgboost::mpl::if_<
                    ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::out_reference
                    >
                  , ::dgboost::mpl::true_
                  , ::dgboost::is_same<
                        typename Tag::qualifier
                      , ::dgboost::parameter::forward_reference
                    >
                >::type
              , ::dgboost::mpl::if_<
                    ::dgboost::is_const<Default>
                  , ::dgboost::mpl::false_
                  , ::dgboost::mpl::true_
                >
              , ::dgboost::mpl::false_
            >::type
          , ::dgboost::parameter::aux::default_<Tag,Default>
        >::type
#endif  // BOOST_NO_SFINAE
            operator|(Default& d) const
        {
            return ::dgboost::parameter::aux::default_<Tag,Default>(d);
        }

        template <typename Default>
        inline BOOST_CONSTEXPR
        ::dgboost::parameter::aux::lazy_default<Tag,Default const>
            operator||(Default const& d) const
        {
            return ::dgboost::parameter::aux
            ::lazy_default<Tag,Default const>(d);
        }

        template <typename Default>
        inline BOOST_CONSTEXPR
        ::dgboost::parameter::aux::lazy_default<Tag,Default>
            operator||(Default& d) const
        {
            return ::dgboost::parameter::aux::lazy_default<Tag,Default>(d);
        }

     public: // Insurance against ODR violations
        // Users will need to define their keywords in header files.  To
        // prevent ODR violations, it's important that the keyword used in
        // every instantiation of a function template is the same object.
        // We provide a reference to a common instance of each keyword
        // object and prevent construction by users.
        static ::dgboost::parameter::keyword<Tag> const instance;

        // This interface is deprecated.
        static ::dgboost::parameter::keyword<Tag>& get()
        {
            return const_cast< ::dgboost::parameter::keyword<Tag>&>(instance);
        }
    };

    template <typename Tag>
    ::dgboost::parameter::keyword<Tag> const ::dgboost::parameter
    ::keyword<Tag>::instance = ::dgboost::parameter::keyword<Tag>();
}} // namespace dgboost::parameter

#endif  // BOOST_PARAMETER_HAS_PERFECT_FORWARDING

#include <dgboost/parameter/aux_/name.hpp>
#include <dgboost/preprocessor/stringize.hpp>

// Reduces boilerplate required to declare and initialize keywords without
// violating ODR.  Declares a keyword tag type with the given name in
// namespace tag_namespace, and declares and initializes a reference in an
// anonymous namespace to a singleton instance of that type.
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#define BOOST_PARAMETER_KEYWORD(tag_namespace, name)                         \
    namespace tag_namespace                                                  \
    {                                                                        \
        struct name                                                          \
        {                                                                    \
            static BOOST_CONSTEXPR char const* keyword_name()                \
            {                                                                \
                return BOOST_PP_STRINGIZE(name);                             \
            }                                                                \
            using _ = BOOST_PARAMETER_TAG_PLACEHOLDER_TYPE(name);            \
            using _1 = _;                                                    \
            BOOST_PARAMETER_TAG_MP11_PLACEHOLDER_BINDING(binding_fn, name);  \
            BOOST_PARAMETER_TAG_MP11_PLACEHOLDER_VALUE(fn, name);            \
            using qualifier = ::dgboost::parameter::forward_reference;         \
        };                                                                   \
    }                                                                        \
    namespace                                                                \
    {                                                                        \
        ::dgboost::parameter::keyword<tag_namespace::name> const& name         \
            = ::dgboost::parameter::keyword<tag_namespace::name>::instance;    \
    }
/**/
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
#define BOOST_PARAMETER_KEYWORD(tag_namespace, name)                         \
    namespace tag_namespace                                                  \
    {                                                                        \
        struct name                                                          \
        {                                                                    \
            static BOOST_CONSTEXPR char const* keyword_name()                \
            {                                                                \
                return BOOST_PP_STRINGIZE(name);                             \
            }                                                                \
            typedef BOOST_PARAMETER_TAG_PLACEHOLDER_TYPE(name) _;            \
            typedef BOOST_PARAMETER_TAG_PLACEHOLDER_TYPE(name) _1;           \
            typedef ::dgboost::parameter::forward_reference qualifier;         \
        };                                                                   \
    }                                                                        \
    namespace                                                                \
    {                                                                        \
        ::dgboost::parameter::keyword<tag_namespace::name> const& name         \
            = ::dgboost::parameter::keyword<tag_namespace::name>::instance;    \
    }
/**/
#endif  // BOOST_PARAMETER_CAN_USE_MP11

#endif  // include guard

