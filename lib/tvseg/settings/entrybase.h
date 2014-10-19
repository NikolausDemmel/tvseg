#ifndef TVSEG_SETTINGS_ENTRYBASE_H
#define TVSEG_SETTINGS_ENTRYBASE_H

#include "tvseg/settings/entry.h"
#include "tvseg/util/logging.h"

#include <sstream>

namespace tvseg {
namespace settings {

template<class T>
class EntryBase : public Entry
{
public:
    typedef boost::shared_ptr<EntryBase<T> > Ptr;
    typedef boost::shared_ptr<const EntryBase<T> > ConstPtr;

public:
    explicit EntryBase(const T &defaultValue = T()):
        defaultValue_(defaultValue),
        value_(defaultValue)
    {
    }

public: // static creator method

    static Ptr make(const T &defaultValue = T());

public: // Entry interface
    std::string toString() const
    {
        return toString(get());
    }

    void fromString(const std::string &str)
    {
        value_ = parseString(str);
    }

    boost::any value() const
    {
        return boost::any(get());
    }

    boost::any defaultValue() const
    {
        return boost::any(getDefault());
    }

    bool setValue(const boost::any &value)
    {
        const T *p = boost::any_cast<T>(&value);
        if (p) {
            set(*p);
            return true;
        } else {
            return false;
        }
    }

public:
    virtual const T & get() const
    {
        return value_;
    }

    virtual const T & getDefault() const
    {
        return defaultValue_;
    }

    virtual void set(const T &value)
    {
        value_ = value;
    }

    virtual void compareConfig(const EntryBase &o)
    {
        // Compare configuration (default values, range restrictions, etc) and omit warnings

        if (o.defaultValue_ != defaultValue_) {
            LWARNINGF("Mismatch when comparing Entry configuration. Default values do not match. '%s' != '%s'.",
                     toString(defaultValue_).c_str(), toString(o.defaultValue_).c_str());
        }
    }

protected:
    virtual std::string toString(const T &value) const;
    virtual T parseString(const std::string &str) const;

protected:
    T defaultValue_;
    T value_;
};


template<class T>
std::string EntryBase<T>::toString(const T &value) const
{
    std::ostringstream ss;
    ss << std::boolalpha << value;
    return ss.str();
}


template<class T>
T EntryBase<T>::parseString(const std::string &str) const
{
    T val = defaultValue_;
    std::istringstream ss(str);
    ss >> std::boolalpha >> val;
//  TODO: How to reliably check if parsing was successful?
//    if(!(ss >> std::boolalpha >> val)) {
//        // failure
//        LWARNINGF("Could not parse string '%s' as type '%s', using default value.", str.c_str(), typeid(T).name());
//        val = defaultValue_;
//    }
    return val;
}

template<class T>
typename EntryBase<T>::Ptr EntryBase<T>::make(const T &defaultValue)
{
    return EntryBase<T>::Ptr(new EntryBase<T>(defaultValue));
}


typedef std::vector<char> byte_array_t;
typedef std::vector<float> float_array_t;
typedef std::vector<uint> uint_array_t;

// specialization for byte array
template<>
std::string EntryBase<byte_array_t>::toString(const byte_array_t &value) const;

template<>
byte_array_t EntryBase<byte_array_t>::parseString(const std::string &str) const;


typedef EntryBase<std::string> StringEntry;
typedef EntryBase<float> FloatEntry;
typedef EntryBase<double> DoubleEntry;
typedef EntryBase<unsigned int> UIntEntry;
typedef EntryBase<int> IntEntry;
typedef EntryBase<bool> BoolEntry;
typedef EntryBase<byte_array_t> ByteArrayEntry;
typedef EntryBase<float_array_t> FloatArrayEntry;
typedef EntryBase<uint_array_t> UIntArrayEntry;


} // namespace settings
} // namespace tvseg

#endif // TVSEG_SETTINGS_ENTRYBASE_H
