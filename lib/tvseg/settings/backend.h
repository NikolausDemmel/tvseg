#ifndef TVSEG_SETTINGS_BACKEND_H
#define TVSEG_SETTINGS_BACKEND_H

#include "tvseg/settings/entry.h"
#include "tvseg/util/logging.h"

#include <map>
#include <boost/shared_ptr.hpp>


namespace tvseg {
namespace settings {

class Backend
{

public: // types    
    typedef std::string key_type;
    typedef std::string raw_value_type;
    typedef EntryPtr entry_type;

    struct Blob
    {
        Blob(raw_value_type new_raw_value = raw_value_type(), entry_type new_entry = entry_type()) :
            raw_value(new_raw_value),
            entry(new_entry)
        {}

        void updateRaw() { if (entry) raw_value = entry->toString(); }
        void updateFromRaw() { if (entry) entry->fromString(raw_value); }

        raw_value_type raw_value;
        entry_type entry;
    };

    typedef Blob blob_type;
    typedef std::map<key_type, blob_type> store_type;
    typedef store_type::iterator iterator;
    typedef store_type::const_iterator const_iterator;

public: // static helpers
    static key_type concatNames(key_type prefix, key_type name);

    static const key_type nameSeparator;

public:

    template<class EntryType>
    boost::shared_ptr<EntryType> addEntry(key_type name, boost::shared_ptr<EntryType> defaultValueEntry)
    {
        iterator it = store_.find(name);
        if (store_.end() == it) {
            // add new entry
            store_[name].entry = defaultValueEntry;
            return defaultValueEntry;
        } else {
            blob_type& blob = it->second;
            if (!blob.entry) {
                // entry loaded from settings file, but no value object connected
                blob.entry = defaultValueEntry;
                blob.updateFromRaw();
                return defaultValueEntry;
            } else {
                // value object present; make sure it is compatible
                boost::shared_ptr<EntryType> castPtr = boost::dynamic_pointer_cast<EntryType>(blob.entry);
                if (castPtr) {
                    // type is ok, now check configuration (default values etc)
                    defaultValueEntry->compareConfig(*castPtr);
                    return castPtr;
                } else {
                    LFATALF("Could not add entry (%s) to settings backend, since existing Entry object (%s) is incompatible.",
                            typeid(*defaultValueEntry).name(), typeid(*blob.entry).name());
                    return defaultValueEntry;
                }
            }
        }
    }

    //virtual void setEntry(key_type name, entry_type value) = 0;
    //virtual entry_type getEntry(key_type name) = 0;

    blob_type & at(key_type name) { return store_[name]; }

    iterator begin() { return store_.begin(); }
    const_iterator begin() const { return store_.begin(); }
    iterator end() { return store_.end(); }
    const_iterator end() const { return store_.end(); }

private:
    store_type store_;

};


typedef boost::shared_ptr<Backend> BackendPtr;
typedef boost::shared_ptr<const Backend> BackendConstPtr;


} // namespace settings
} // namespace tvseg

#endif // TVSEG_SETTINGS_BACKEND_H
