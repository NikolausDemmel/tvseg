#ifndef TVSEG_UI_UTIL_VALUETRANSITION_H
#define TVSEG_UI_UTIL_VALUETRANSITION_H


#include "tvseg/util/logging.h"

#include <QSignalTransition>
#include <QStateMachine>


namespace tvseg_ui {
namespace util {


///< Transition on a signal with single value argument. Transition for given value only.
template<class V>
class ValueTransition : public QSignalTransition
{
public:
    static ValueTransition* make(QObject *sender, const char *signal, V value, QAbstractState* targetState = NULL)
    {
        ValueTransition* trans = new ValueTransition(sender, signal, value);
        if (targetState) {
            trans->setTargetState(targetState);
        }
        return trans;
    }

    ValueTransition(QObject *sender, const char *signal, V value):
        QSignalTransition(sender, signal),
        value_(value)
    {}

protected:
    bool eventTest(QEvent *e)
    {
        if (!QSignalTransition::eventTest(e))
            return false;
        QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
        QVariant arg = se->arguments().at(0);
        if (arg.canConvert<V>()) {
            return (arg.value<V>() == value_);
        } else {
            LWARNING << "Cannot convert signal argument type " << arg.typeName() << " to " << typeid(V).name();
            return false;
        }
    }

private:
    const bool value_;
};


}
}

#endif // TVSEG_UI_UTIL_VALUETRANSITION_H
