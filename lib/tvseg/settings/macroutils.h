#ifndef TVSEG_SETTINGS_MACRO_UTILS_H
#define TVSEG_SETTINGS_MACRO_UTILS_H

#include "boost/preprocessor/if.hpp"
#include "boost/preprocessor/dec.hpp"
#include "boost/preprocessor/seq/for_each.hpp"
#include "boost/preprocessor/tuple/eat.hpp"
#include "boost/preprocessor/seq/seq.hpp"
#include "boost/preprocessor/seq/size.hpp"
#include "boost/preprocessor/repeat.hpp"

/////////////////////////////////////////////////////////////////////////////////
/// NAMESPACE
/////////////////////////////////////////////////////////////////////////////////

#define TVSEG_SETTINGS_NAMESPACE_BEGIN_I(R,DATA,ELEM)                           \
    namespace ELEM {
#define TVSEG_SETTINGS_NAMESPACE_END_I(Z,I,DATA) }
#define TVSEG_SETTINGS_NAMESPACE_DECLARATION_I(Z,I,ELEM) ELEM::


#define TVSEG_SETTINGS_NAMESPACE_DEFINITION_BEGIN(NAMESPACE_SEQ)                \
    BOOST_PP_IF(                                                                \
        BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(NAMESPACE_SEQ)),                         \
        BOOST_PP_SEQ_FOR_EACH_R,                                                \
        BOOST_PP_TUPLE_EAT(4))(                                                 \
            1,                                                                  \
            TVSEG_SETTINGS_NAMESPACE_BEGIN_I,                                   \
            _,                                                                  \
            BOOST_PP_SEQ_TAIL(NAMESPACE_SEQ))



#define TVSEG_SETTINGS_NAMESPACE_DEFINITION_END(NAMESPACE_SEQ)                  \
    BOOST_PP_REPEAT_1(                                                          \
        BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(NAMESPACE_SEQ)),                         \
        TVSEG_SETTINGS_NAMESPACE_END_I,                                         \
        _)



#define TVSEG_SETTINGS_NAMESPACE_DECLARATION(NAMESPACE_SEQ)                     \
    BOOST_PP_IF(                                                                \
        BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(NAMESPACE_SEQ)),                         \
        BOOST_PP_SEQ_FOR_EACH_R,                                                \
        BOOST_PP_TUPLE_EAT(4))(                                                 \
            1,                                                                  \
            TVSEG_SETTINGS_NAMESPACE_DECLARATION_I,                             \
            _,                                                                  \
            BOOST_PP_SEQ_TAIL(NAMESPACE_SEQ))



/////////////////////////////////////////////////////////////////////////////////
/// TUPLE HELPERS
/////////////////////////////////////////////////////////////////////////////////

#define TVSEG_SETTINGS_DEFINE_FILLER_0(X, Y, Z)                                 \
    ((X, Y, Z)) TVSEG_SETTINGS_DEFINE_FILLER_1
#define TVSEG_SETTINGS_DEFINE_FILLER_1(X, Y, Z)                                 \
    ((X, Y, Z)) TVSEG_SETTINGS_DEFINE_FILLER_0
#define TVSEG_SETTINGS_DEFINE_FILLER_0_END
#define TVSEG_SETTINGS_DEFINE_FILLER_1_END

#endif // TVSEG_SETTINGS_MACRO_UTILS_H
