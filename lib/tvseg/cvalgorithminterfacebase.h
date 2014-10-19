#ifndef TVSEG_CVALGORITHMINTERFACEBASE_H
#define TVSEG_CVALGORITHMINTERFACEBASE_H

#include "tvseg/cvalgorithminterface.h"

#include <boost/foreach.hpp>

namespace tvseg {

class CVAlgorithmInterfaceBase : public CVAlgorithmInterface
{
public:
    CVAlgorithmInterfaceBase() {}

    CVAlgorithmInterfaceBase(std::string typeName, std::string name, settings::SettingsPtr settings):
        typeName_(typeName),
        name_(name),
        settings_(settings)
    {}

    CVAlgorithmInterfaceBase(const CVAlgorithmInterface* rhs):
        typeName_(rhs->typeName()),
        name_(rhs->name()),
        settings_(rhs->settings()),
        actions_(rhs->actions())
    {}

public: // CVAlgorithmInterface interface
    std::string typeName() const { return typeName_; }
    std::string name() const { return name_; }
    settings::SettingsPtr settings() const { return settings_; }
    action_list_t actions() const { return actions_; }

    bool hasAction(std::string name) const
    {
        BOOST_FOREACH(const Action& a, actions_) {
            if (a.name == name) {
                return true;
            }
        }

        return false;
    }

    Action action(std::string name) const
    {
        BOOST_FOREACH(const Action& a, actions_) {
            if (a.name == name) {
                return a;
            }
        }

        return Action();
    }

public:
    void setTypeName(std::string typeName) { typeName_ = typeName; }
    void setName(std::string name) { name_ = name; }
    void setSettings(settings::SettingsPtr settings) { settings_ = settings; }
    void setActions(const action_list_t &actions) { actions_ = actions; }

    void clearActions() { actions_.clear(); }
    void addAction(const Action &action) { actions_.push_back(action); }

private:
    std::string typeName_;
    std::string name_;
    settings::SettingsPtr settings_;
    action_list_t actions_;
};

typedef boost::shared_ptr<CVAlgorithmInterfaceBase> CVAlgorithmInterfaceBasePtr;
typedef boost::shared_ptr<CVAlgorithmInterfaceBase const> CVAlgorithmInterfaceBaseConstPtr;

} // namespace tvseg

#endif // TVSEG_CVALGORITHMINTERFACEBASE_H
