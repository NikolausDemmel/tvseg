#ifndef TVSEG_SETTINGS_SETTINGS_H
#define TVSEG_SETTINGS_SETTINGS_H

#include "backend.h"
#include "entry.h"
#include "entrybase.h"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <list>
#include <utility>
#include <typeinfo>

//#include "LDEBUGF"

namespace tvseg {
namespace settings {

//class Typeinfo
//{
//    Typeinfo(std::type_info* id) :
//        id_(id)
//    {
//        if (!id) {
//            LERROR << "Instatiated Typeinfo with NULL pointer. Bad things are bound to happen.";
//        }
//    }

//    bool operator == (const Typeinfo &o) const { return *id_ == *o->id_; }
//    bool operator != (const Typeinfo &o) const { return *id_ != *o->id_; }

//    std::string toString() { return id_->name(); }

//private:
//    std::type_info* id_;
//};

class Settings
{

public:
    typedef Backend::key_type key_type;
    typedef Backend::entry_type entry_type;
    typedef std::map<key_type, entry_type> entry_map_type;

public:
    // TODO: Regsiter settings with backend
    Settings(BackendPtr backend, key_type prefix = "");
    // TODO: Add children to parent, get backend from parent. Replace backend in children when called
    //Settings(const Settings* parent, key_type prefix = "");

    virtual ~Settings() {}

//    template<class T>
//    const T& get(key_type name)
//    {

//    }

//    template<class T>
//    void set(key_type name, const T& value);


    const entry_map_type &entries() const { return entries_; }

    const std::vector<key_type> names() const
    {
        std::vector<key_type> keys;
        boost::copy(entries_ | boost::adaptors::map_keys, std::back_inserter(keys));
        return keys;
    }

    bool hasName(key_type name) const { return entries_.find(name) != entries_.end(); }

    entry_type getEntry(key_type name) const
    {
        entry_map_type::const_iterator it = entries_.find(name);
        if (it != entries_.end()) {
            return it->second;
        } else {
            return entry_type(); // empty pointer
        }
    }

    // FIXME: Add descriptions to entries and use as tool tips

    // FIXME:
    // std::list<SettingsPtr> children() const;

    const key_type& prefix() const { return prefix_; }

protected:
    template<class EntryTypePtr>
    EntryTypePtr addEntry(key_type name, EntryTypePtr defaultValueEntry)
    {
        if (hasName(name)) {
            LWARNING << "Entry '" << name << "' already added to settings. Ignoring.";
            return defaultValueEntry;
        }
        key_type backendKey = Backend::concatNames(prefix(), name);
        EntryTypePtr e = backend_->addEntry(backendKey, defaultValueEntry);
        entries_[name] = e;
        return e;
    }

    template<class T>
    typename EntryBase<T>::Ptr add(key_type name, const T &defaultValue = T())
    {
        return addEntry(name, EntryBase<T>::make(defaultValue));
    }

private:
    BackendPtr backend_;
    const key_type prefix_;
    entry_map_type entries_;
};


typedef boost::shared_ptr<Settings> SettingsPtr;
typedef boost::shared_ptr<const Settings> SettingsConstPtr;


} // namespace settings
} // namespace tvseg

#endif // TVSEG_SETTINGS_SETTINGS_H
