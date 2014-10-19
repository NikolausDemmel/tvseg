#ifndef TVSEG_CVALGORITHMINTERFACE_H
#define TVSEG_CVALGORITHMINTERFACE_H

#include "tvseg/settings/settings.h"

#include "tvseg/util/includeopencv.h"

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>


namespace tvseg {

class CVAlgorithmInterface
{
public:
    struct Action {
        typedef boost::function<void ()> compute_fn_t;
        typedef boost::function<void ()> clear_fn_t;
        typedef boost::function<bool ()> result_available_fn_t;
        typedef boost::function<cv::Mat ()> result_fn_t;

        Action() {}

        Action(std::string actionName, compute_fn_t compute, clear_fn_t clear, result_available_fn_t resultAvailable, result_fn_t result):
            name(actionName),
            computeFn(compute),
            clearFn(clear),
            resultAvailableFn(resultAvailable),
            resultFn(result)
        {}

        void compute() const { if (computeFn) computeFn(); }
        void clear() const { if (clearFn) clearFn(); }
        bool resultAvailable() const { return resultAvailableFn ? resultAvailableFn() : false; }
        cv::Mat result() const { return resultFn ? resultFn() : cv::Mat(); }

        std::string name;
        compute_fn_t computeFn;
        clear_fn_t clearFn;
        result_available_fn_t resultAvailableFn;
        result_fn_t resultFn;
    };

public:
    typedef std::list<Action> action_list_t;

public:
    virtual ~CVAlgorithmInterface() {}

    virtual std::string typeName() const = 0;
    virtual std::string name() const = 0;
    virtual settings::SettingsPtr settings() const = 0;
    virtual action_list_t actions() const = 0;
    virtual bool hasAction(std::string name) const = 0;
    virtual Action action(std::string name) const = 0;
};

typedef boost::shared_ptr<CVAlgorithmInterface> CVAlgorithmInterfacePtr;
typedef boost::shared_ptr<CVAlgorithmInterface const> CVAlgorithmInterfaceConstPtr;

} // namespace tvseg

#endif // TVSEG_CVALGORITHMINTERFACE_H
