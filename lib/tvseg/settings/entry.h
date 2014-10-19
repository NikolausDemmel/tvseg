#ifndef TVSEG_SETTINGS_ENTRY_H
#define TVSEG_SETTINGS_ENTRY_H

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

namespace tvseg {
namespace settings {

// TODO: default values
// TODO: input value checking such as range (min/max), but also custom bool functions

class Entry
{
public:
    virtual ~Entry() {}

    virtual std::string toString() const = 0;
    virtual void fromString(const std::string &) = 0;

    virtual boost::any value() const = 0;
    virtual boost::any defaultValue() const = 0;

    virtual bool setValue(const boost::any &value) = 0;
};

typedef boost::shared_ptr<Entry> EntryPtr;
typedef boost::shared_ptr<const Entry> EntryConstPtr;


} // namespace settings
} // namespace tvseg

#endif // TVSEG_SETTINGS_ENTRY_H
