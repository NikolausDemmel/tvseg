#ifndef TVSEG_SETTINGS_DEFINE_SETTINGS_H
#define TVSEG_SETTINGS_DEFINE_SETTINGS_H

#include "tvseg/settings/macroutils.h"
#include "tvseg/settings/settings.h"
#include "tvseg/settings/entrybase.h"

#include <boost/call_traits.hpp>

/////////////////////////////////////////////////////////////////////////////////
/// USAGE
/////////////////////////////////////////////////////////////////////////////////

///
/// The following
///
///     TVSEG_SETTINGS_DEFINE_SETTINGS(
///         (tvseg), SolverSettings, "solver", SolverParams,
///         (float, lambda, 1.0)
///         (unsigned int, maxSteps, 1000))
///
/// is expanded to this class definitions:
///
///     namespace tvseg {
///
///     class SolverSettings : public tvseg::settings::Settings
///     {
///     public:
///         typedef boost::shared_ptr<SolverSettings> Ptr;
///         typedef boost::shared_ptr<const SolverSettings> ConstPtr;
///         typedef SolverParams params_type;
///
///     private:
///         tvseg::settings::EntryBase<float>::Ptr lambda_;
///         tvseg::settings::EntryBase<unsigned int>::Ptr maxSteps_;
///
///     public:
///         SolverSettings(tvseg::settings::BackendPtr backend) :
///             tvseg::settings::Settings(backend, "solver"),
///             lambda_(addEntry("lambda", tvseg::settings::EntryBase<float>::make(1.0))),
///             maxSteps_(addEntry("maxSteps", tvseg::settings::EntryBase<unsigned int>::make(1000)))
///         {}
///
///         boost::call_traits<float>::const_reference lambda() const
///         {
///             return lambda_->get();
///         }
///
///         void set_lambda(boost::call_traits<float>::param_type value)
///         {
///             lambda_->set(value);
///         }
///
///         boost::call_traits<unsigned int>::const_reference maxSteps() const
///         {
///             return maxSteps_->get();
///         }
///
///         void set_maxSteps(boost::call_traits<unsigned int>::param_type value)
///         {
///             maxSteps_->set(value);
///         }
///
///         params_type params() const
///         {
///             params_type p;
///             p.lambda = lambda();
///             p.maxSteps = maxSteps();
///             return p;
///         }
///     };
///
///     }



/////////////////////////////////////////////////////////////////////////////////
/// DEFINE_SETTINGS
/////////////////////////////////////////////////////////////////////////////////

#define TVSEG_SETTINGS_DEFINE_SETTINGS(                                         \
    NAMESPACE_SEQ, NAME, GROUP_NAME, PARAMS_NAME, ATTRIBUTES)                   \
    TVSEG_SETTINGS_DEFINE_SETTINGS_IMPL(                                        \
        (0)NAMESPACE_SEQ,                                                       \
        NAME,                                                                   \
        GROUP_NAME,                                                             \
        PARAMS_NAME,                                                            \
        BOOST_PP_CAT(TVSEG_SETTINGS_DEFINE_FILLER_0(0,0,0)ATTRIBUTES,_END),     \
        3)



#define TVSEG_SETTINGS_DEFINE_SETTINGS_IMPL(                                    \
    NAMESPACE_SEQ, NAME, GROUP_NAME, PARAMS_NAME,                               \
    ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                                       \
                                                                                \
    TVSEG_SETTINGS_NAMESPACE_DEFINITION_BEGIN(NAMESPACE_SEQ)                    \
                                                                                \
    class NAME : public tvseg::settings::Settings                               \
    {                                                                           \
    public:                                                                     \
        typedef boost::shared_ptr<NAME> Ptr;                                    \
        typedef boost::shared_ptr<const NAME> ConstPtr;                         \
        typedef PARAMS_NAME params_type;                                        \
                                                                                \
        BOOST_PP_IF(BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(ATTRIBUTES_SEQ)),            \
            TVSEG_SETTINGS_DEFINE_NONEMPTY_SETTINGS_IMPL,                       \
            TVSEG_SETTINGS_DEFINE_EMPTY_SETTINGS_IMPL)(                         \
                NAME, GROUP_NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)         \
    };                                                                          \
                                                                                \
    TVSEG_SETTINGS_NAMESPACE_DEFINITION_END(NAMESPACE_SEQ)



#define TVSEG_SETTINGS_DEFINE_EMPTY_SETTINGS_IMPL(                              \
    NAME, GROUP_NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                     \
                                                                                \
    explicit NAME(tvseg::settings::BackendPtr backend) :                        \
        tvseg::settings::Settings(backend, GROUP_NAME)                          \
    {}                                                                          \
                                                                                \
    params_type params() const                                                  \
    {                                                                           \
        return params_type;                                                     \
    }                                                                           \
                                                                                \
    void setParams(const params_type &)                                         \
    {                                                                           \
    }



#define TVSEG_SETTINGS_DEFINE_NONEMPTY_SETTINGS_IMPL(                           \
    NAME, GROUP_NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                     \
                                                                                \
        TVSEG_SETTINGS_DEFINE_SETTINGS_IMPL_IMPL(                               \
            NAME, GROUP_NAME,                                                   \
            BOOST_PP_SEQ_TAIL(ATTRIBUTES_SEQ), ATTRIBUTE_TUPEL_SIZE)



#define TVSEG_SETTINGS_DEFINE_SETTINGS_IMPL_IMPL(                               \
    NAME, GROUP_NAME, ATTRIBUTES_SEQ, ATTRIBUTE_TUPEL_SIZE)                     \
                                                                                \
    private:                                                                    \
    BOOST_PP_SEQ_FOR_EACH_R(                                                    \
        1,                                                                      \
        TVSEG_SETTINGS_DEFINE_SETTINGS_ATTR_I,                                  \
        ATTRIBUTE_TUPEL_SIZE,                                                   \
        ATTRIBUTES_SEQ)                                                         \
                                                                                \
    public:                                                                     \
    NAME(tvseg::settings::BackendPtr backend)                                   \
      : tvseg::settings::Settings(backend, GROUP_NAME)                          \
        BOOST_PP_SEQ_FOR_EACH_I_R(                                              \
            1,                                                                  \
            TVSEG_SETTINGS_DEFINE_SETTINGS_SEQ_DEFAULT_CTOR_FILLER_I,           \
            ATTRIBUTE_TUPEL_SIZE,                                               \
            ATTRIBUTES_SEQ)                                                     \
    {}                                                                          \
                                                                                \
    BOOST_PP_SEQ_FOR_EACH_R(                                                    \
        1,                                                                      \
        TVSEG_SETTINGS_DEFINE_SETTINGS_ATTR_ACCESSOR_I,                         \
        ATTRIBUTE_TUPEL_SIZE,                                                   \
        ATTRIBUTES_SEQ)                                                         \
                                                                                \
    params_type params() const                                                  \
    {                                                                           \
        params_type p;                                                          \
        BOOST_PP_SEQ_FOR_EACH_R(                                                \
            1,                                                                  \
            TVSEG_SETTINGS_DEFINE_SETTINGS_ASSIGN_PARAMS_I,                     \
            ATTRIBUTE_TUPEL_SIZE,                                               \
            ATTRIBUTES_SEQ)                                                     \
        return p;                                                               \
    }                                                                           \
                                                                                \
    void setParams(const params_type &p)                                        \
    {                                                                           \
        BOOST_PP_SEQ_FOR_EACH_R(                                                \
            1,                                                                  \
            TVSEG_SETTINGS_DEFINE_SETTINGS_ASSIGN_FROM_PARAMS_I,                \
            ATTRIBUTE_TUPEL_SIZE,                                               \
            ATTRIBUTES_SEQ)                                                     \
    }



#define TVSEG_SETTINGS_DEFINE_SETTINGS_ATTR_I(                                  \
    R, ATTRIBUTE_TUPEL_SIZE, ATTRIBUTE)                                         \
                                                                                \
    tvseg::settings::EntryBase<                                                 \
        BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)                   \
    >::Ptr                                                                      \
        BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE),_);



#define TVSEG_SETTINGS_DEFINE_SETTINGS_SEQ_DEFAULT_CTOR_FILLER_I(               \
    R, ATTRIBUTE_TUPEL_SIZE, I, ATTRIBUTE)                                      \
                                                                                \
    BOOST_PP_COMMA()                                                            \
    BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE),_)       \
        (addEntry(BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(                       \
                ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE)),                             \
            tvseg::settings::EntryBase<                                         \
                BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)           \
            >::make(BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,2,ATTRIBUTE))))



#define TVSEG_SETTINGS_DEFINE_SETTINGS_ASSIGN_PARAMS_I(                         \
    R, ATTRIBUTE_TUPEL_SIZE, ATTRIBUTE)                                         \
                                                                                \
    p.BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE) =                   \
        BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE)();



#define TVSEG_SETTINGS_DEFINE_SETTINGS_ASSIGN_FROM_PARAMS_I(                    \
    R, ATTRIBUTE_TUPEL_SIZE, ATTRIBUTE)                                         \
                                                                                \
    BOOST_PP_CAT(set_,BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE))    \
        (p.BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE));



#define TVSEG_SETTINGS_DEFINE_SETTINGS_ATTR_ACCESSOR_I(                         \
    R, ATTRIBUTE_TUPEL_SIZE, ATTRIBUTE)                                         \
                                                                                \
    boost::call_traits<                                                         \
        BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)                   \
    >::const_reference                                                          \
            BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE)() const       \
    {                                                                           \
        return BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(                                \
            ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE),_)->get();                        \
    }                                                                           \
                                                                                \
    void BOOST_PP_CAT(                                                          \
        set_,BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE))             \
        (boost::call_traits<                                                    \
            BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,0,ATTRIBUTE)               \
         >::param_type value)                                                   \
    {                                                                           \
        BOOST_PP_CAT(                                                           \
            BOOST_PP_TUPLE_ELEM(ATTRIBUTE_TUPEL_SIZE,1,ATTRIBUTE),              \
            _->set(value));                                                     \
    }



#endif // TVSEG_SETTINGS_DEFINE_SETTINGS_H
