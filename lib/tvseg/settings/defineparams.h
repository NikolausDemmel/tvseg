#ifndef TVSEG_SETTINGS_DEFINE_PARAMS_H
#define TVSEG_SETTINGS_DEFINE_PARAMS_H

// FIXME: include boost license info in project
// These macro are derived from the following files
//#include <boost/fusion/adapted/struct/define_struct.hpp>
//#include <boost/fusion/include/define_struct.hpp>
//BOOST_FUSION_DEFINE_STRUCT(
//    (demo), employee,
//    (std::string, name)
//    (int, age))

#include "tvseg/settings/macroutils.h"

#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <ostream>
// FIXME: call_traits (issue with nvcc...)
//#include <boost/call_traits.hpp>



/////////////////////////////////////////////////////////////////////////////////
/// USAGE
/////////////////////////////////////////////////////////////////////////////////

///
/// The following
///
///     TVSEG_SETTINGS_DEFINE_PARAMS(
///         (tvseg), SolverParams,
///         (float, lambda, 1.0)
///         (unsigned int, maxSteps, 1000))
///
/// is expanded to this class definitions:
///
///     namespace tvseg {
///
///     struct SolverParams
///     {
///         float lambda;
///         unsigned int maxSteps;
///
///         SolverParams() :
///             lambda(1.0),
///             maxSteps(1000)
///         {}
///
///         boost::call_traits<float>::const_reference lambdaDefault()
///         {
///             static const float value(1.0);
///             return value;
///         }
///
///         boost::call_traits<unsigned int>::const_reference maxStepsDefault()
///         {
///             static const unsigned int value(1000);
///             return value;
///         }
///
///         SolverParams(
///                 boost::call_traits<float>::param_type _lambda,
///                 boost::call_traits<unsigned int>::param_type _maxSteps) :
///             lambda(_lambda),
///             maxSteps(_maxSteps)
///         {}
///
///         bool operator== (const SolverOutput &rhs) const
///         {
///            if (lambda != rhs.lambda) { return false; }
///            if (maxSteps != rhs.maxSteps) { return false; }
///            return true;
///         }
///
///         bool operator!= (const SolverOutput &rhs) const
///         {
///             return !(*this == rhs);
///         }
///
///     };
///
///     inline std::ostream & operator << (std::ostream &oss, const SolverParams &rhs)
///     {
///         oss << "{ " "lambda" ":"
///             << rhs.lambda
///             << ", " "maxSteps" ":"
///             << rhs.maxSteps
///             << " }";
///         return oss;
///     }
///
///     }



/////////////////////////////////////////////////////////////////////////////////
/// DEFINE_PARAMS
/////////////////////////////////////////////////////////////////////////////////

#define TVSEG_SETTINGS_DEFINE_PARAMS(NAMESPACE_SEQ, NAME, ATTRIBUTES)           \
    TVSEG_SETTINGS_DEFINE_PARAMS_IMPL(                                          \
        (0)NAMESPACE_SEQ,                                                       \
        NAME,                                                                   \
        BOOST_PP_CAT(TVSEG_SETTINGS_DEFINE_FILLER_0(0,0,0)ATTRIBUTES,_END),     \
        3)



#define TVSEG_SETTINGS_DEFINE_PARAMS_IMPL(                                      \
    NAMESPACE_SEQ, NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                  \
                                                                                \
    TVSEG_SETTINGS_NAMESPACE_DEFINITION_BEGIN(NAMESPACE_SEQ)                    \
                                                                                \
    struct NAME                                                                 \
    {                                                                           \
        BOOST_PP_IF(BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ATTRIBUTES_SEQ)),            \
            TVSEG_SETTINGS_DEFINE_NONEMPTY_PARAMS_IMPL,                         \
            TVSEG_SETTINGS_DEFINE_EMPTY_PARAMS_IMPL)(                           \
                NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                     \
    };                                                                          \
                                                                                \
    BOOST_PP_IF(BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ATTRIBUTES_SEQ)),                \
        TVSEG_SETTINGS_DEFINE_NONEMPTY_PARAMS_OSS_OPERATOR_IMPL,                \
        TVSEG_SETTINGS_DEFINE_EMPTY_PARAMS_OSS_OPERATOR_IMPL)(                  \
            NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                         \
                                                                                \
    TVSEG_SETTINGS_NAMESPACE_DEFINITION_END(NAMESPACE_SEQ)



#define TVSEG_SETTINGS_DEFINE_EMPTY_PARAMS_IMPL(                                \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)



#define TVSEG_SETTINGS_DEFINE_NONEMPTY_PARAMS_IMPL(                             \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
        TVSEG_SETTINGS_DEFINE_PARAMS_IMPL_IMPL(                                 \
            NAME, BOOST_PP_SEQ_TAIL(ATTRIBUTES_SEQ), ATTRIBUTE_TUPEL_SIZE)      \
                                                                                \
        TVSEG_SETTINGS_DEFINE_PARAMS_CTOR(                                      \
            NAME, BOOST_PP_SEQ_TAIL(ATTRIBUTES_SEQ), ATTRIBUTE_TUPEL_SIZE)      \
                                                                                \
        TVSEG_SETTINGS_DEFINE_PARAMS_COMPARISON(                                \
            NAME, BOOST_PP_SEQ_TAIL(ATTRIBUTES_SEQ), ATTRIBUTE_TUPEL_SIZE)



#define TVSEG_SETTINGS_DEFINE_PARAMS_IMPL_IMPL(                                 \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
    BOOST_PP_SEQ_FOR_EACH_R(                                                    \
        1,                                                                      \
        TVSEG_SETTINGS_DEFINE_PARAMS_ATTR_I,                                    \
        ATTRIBUTE_TUPEL_SIZE,                                                   \
        ATTRIBUTES_SEQ)                                                         \
                                                                                \
    NAME()                                                                      \
      : BOOST_PP_SEQ_FOR_EACH_I_R(                                              \
            1,                                                                  \
            TVSEG_SETTINGS_DEFINE_PARAMS_SEQ_DEFAULT_CTOR_FILLER_I,             \
            ATTRIBUTE_TUPEL_SIZE,                                               \
            ATTRIBUTES_SEQ)                                                     \
    {}                                                                          \
                                                                                \
    BOOST_PP_SEQ_FOR_EACH_R(                                                    \
        1,                                                                      \
        TVSEG_SETTINGS_DEFINE_PARAMS_DEFAULT_ATTR_I,                            \
        ATTRIBUTE_TUPEL_SIZE,                                                   \
        ATTRIBUTES_SEQ)



#define TVSEG_SETTINGS_DEFINE_PARAMS_ATTR_I(R, ATTRIBUTE_TUPEL_SIZE, ATTRIBUTE) \
                                                                                \
    BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)                       \
        BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE);


// FIXME: call_traits
//#define TVSEG_SETTINGS_DEFINE_PARAMS_DEFAULT_ATTR_I(                            \
//    R, ATTRIBUTE_TUPEL_SIZE, ATTRIBUTE)                                         \
//                                                                                \
//    boost::call_traits<                                                         \
//        BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)                   \
//        >::const_reference                                                      \
//        BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(                                       \
//            ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE),Default())                        \
//    {                                                                           \
//        static const BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)      \
//            value(BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,2,ATTRIBUTE));       \
//        return value;                                                           \
//    }



// FIXME: call_traits
#define TVSEG_SETTINGS_DEFINE_PARAMS_DEFAULT_ATTR_I(                            \
    R, ATTRIBUTE_TUPEL_SIZE, ATTRIBUTE)                                         \
                                                                                \
    const BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE) &               \
        BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(                                       \
            ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE),Default())                        \
    {                                                                           \
        static const BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)      \
            value(BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,2,ATTRIBUTE));       \
        return value;                                                           \
    }



#define TVSEG_SETTINGS_DEFINE_PARAMS_SEQ_DEFAULT_CTOR_FILLER_I(                 \
    R, ATTRIBUTE_TUPEL_SIZE, I, ATTRIBUTE)                                      \
                                                                                \
    BOOST_PP_COMMA_IF(I)                                                        \
    BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE)                       \
    (BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,2,ATTRIBUTE))



#define TVSEG_SETTINGS_DEFINE_PARAMS_CTOR(                                      \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
    BOOST_PP_IF(BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ATTRIBUTES_SEQ)),                \
        TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_N,                                    \
        TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_1)(                                   \
            NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)



// FIXME: call_traits
//#define TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_1(                                    \
//        NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                             \
//                                                                                \
//        explicit                                                                \
//        NAME(boost::call_traits<                                                \
//                BOOST_PP_TUPLE_ELEM(                                            \
//                    ATTRIBUTE_TUPEL_SIZE,0,BOOST_PP_SEQ_HEAD(ATTRIBUTES_SEQ))   \
//            >::param_type BOOST_PP_CAT(_,                                       \
//                BOOST_PP_TUPLE_ELEM(                                            \
//                    ATTRIBUTE_TUPEL_SIZE,1,BOOST_PP_SEQ_HEAD(ATTRIBUTES_SEQ)))) \
//          : BOOST_PP_TUPLE_ELEM(                                                \
//                ATTRIBUTE_TUPEL_SIZE,1,BOOST_PP_SEQ_HEAD(ATTRIBUTES_SEQ))       \
//            (BOOST_PP_CAT(_,BOOST_PP_TUPLE_ELEM(                                \
//                ATTRIBUTE_TUPEL_SIZE,1,BOOST_PP_SEQ_HEAD(ATTRIBUTES_SEQ))))     \
//        {}



// FIXME: call_traits
#define TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_1(                                    \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
    explicit                                                                    \
    NAME(BOOST_PP_TUPLE_ELEM(                                                   \
            ATTRIBUTE_TUPEL_SIZE,0,BOOST_PP_SEQ_HEAD(ATTRIBUTES_SEQ))           \
        BOOST_PP_CAT(_, BOOST_PP_TUPLE_ELEM(                                    \
            ATTRIBUTE_TUPEL_SIZE,1,BOOST_PP_SEQ_HEAD(ATTRIBUTES_SEQ))))         \
      : BOOST_PP_TUPLE_ELEM(                                                    \
            ATTRIBUTE_TUPEL_SIZE,1,BOOST_PP_SEQ_HEAD(ATTRIBUTES_SEQ))           \
        (BOOST_PP_CAT(_,BOOST_PP_TUPLE_ELEM(                                    \
            ATTRIBUTE_TUPEL_SIZE,1,BOOST_PP_SEQ_HEAD(ATTRIBUTES_SEQ))))         \
    {}



#define TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_N(                                    \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
    NAME(BOOST_PP_SEQ_FOR_EACH_I_R(                                             \
            1,                                                                  \
            TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_ARG_I,                            \
            ATTRIBUTE_TUPEL_SIZE,                                               \
            ATTRIBUTES_SEQ))                                                    \
      : BOOST_PP_SEQ_FOR_EACH_I_R(                                              \
            1,                                                                  \
            TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_FILLER_I,                         \
            ATTRIBUTE_TUPEL_SIZE,                                               \
            ATTRIBUTES_SEQ)                                                     \
    {}



// FIXME: call_traits
//#define TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_ARG_I(                                \
//        R, ATTRIBUTE_TUPEL_SIZE, I, ATTRIBUTE)                                  \
//                                                                                \
//        BOOST_PP_COMMA_IF(I)                                                    \
//        boost::call_traits<                                                     \
//            BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)               \
//        >::param_type BOOST_PP_CAT(                                             \
//            _, BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE))



// FIXME: call_traits
#define TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_ARG_I(                                \
    R, ATTRIBUTE_TUPEL_SIZE, I, ATTRIBUTE)                                      \
                                                                                \
    BOOST_PP_COMMA_IF(I)                                                        \
    BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)                       \
    BOOST_PP_CAT(                                                               \
        _, BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE))



#define TVSEG_SETTINGS_DEFINE_PARAMS_CTOR_FILLER_I(                             \
    R, ATTRIBUTE_TUPEL_SIZE, I, ATTRIBUTE)                                      \
                                                                                \
    BOOST_PP_COMMA_IF(I)                                                        \
    BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE)                       \
        (BOOST_PP_CAT(_,BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE)))



#define TVSEG_SETTINGS_DEFINE_PARAMS_COMPARISON(                                \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
    bool operator== (const NAME &rhs) const {                                   \
        BOOST_PP_SEQ_FOR_EACH_R(                                                \
            1,                                                                  \
            TVSEG_SETTINGS_DEFINE_PARAMS_COMPARE_I,                             \
            ATTRIBUTE_TUPEL_SIZE,                                               \
            ATTRIBUTES_SEQ)                                                     \
        return true;                                                            \
    }                                                                           \
                                                                                \
    bool operator!= (const NAME &rhs) const { return !(*this == rhs); }



#define TVSEG_SETTINGS_DEFINE_PARAMS_COMPARE_I(                                 \
    R, ATTRIBUTE_TUPEL_SIZE, ATTRIBUTE)                                         \
                                                                                \
    if (BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE) !=                \
        rhs.BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE)) {            \
        return false;                                                           \
    }



#define TVSEG_SETTINGS_DEFINE_EMPTY_PARAMS_OSS_OPERATOR_IMPL(                   \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
    inline std::ostream & operator << (std::ostream &oss, const NAME &)         \
    {                                                                           \
        oss << "{}";                                                            \
        return oss;                                                             \
    }



#define TVSEG_SETTINGS_DEFINE_NONEMPTY_PARAMS_OSS_OPERATOR_IMPL(                \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
    TVSEG_SETTINGS_DEFINE_NONEMPTY_PARAMS_OSS_OPERATOR_IMPL_IMPL(               \
        NAME, BOOST_PP_SEQ_TAIL(ATTRIBUTES_SEQ), ATTRIBUTE_TUPEL_SIZE)




#define TVSEG_SETTINGS_DEFINE_NONEMPTY_PARAMS_OSS_OPERATOR_IMPL_IMPL(           \
    NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                 \
                                                                                \
    inline std::ostream & operator << (std::ostream &oss, const NAME &rhs)      \
    {                                                                           \
        oss << "{ "                                                             \
        BOOST_PP_SEQ_FOR_EACH_I_R(                                              \
            1,                                                                  \
            TVSEG_SETTINGS_DEFINE_PARAMS_OSS_ATTR_I,                            \
            ATTRIBUTE_TUPEL_SIZE,                                               \
            ATTRIBUTES_SEQ)                                                     \
        " }";                                                                   \
        return oss;                                                             \
    }



#define TVSEG_SETTINGS_DEFINE_PARAMS_OSS_ATTR_I(                                \
    R, ATTRIBUTE_TUPEL_SIZE, I, ATTRIBUTE)                                      \
                                                                                \
    BOOST_PP_IF(I, ", ", BOOST_PP_EMPTY())                                      \
    BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE))   \
    ":" << rhs.BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE) <<



#endif // TVSEG_SETTINGS_DEFINE_PARAMS_H
