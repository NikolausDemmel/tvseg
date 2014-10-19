
#include "tvseg/settings/entrybase.h"

#include "tvseg/util/arrayio.h"


namespace tvseg {
namespace settings {


template<>
std::string EntryBase<byte_array_t>::toString(const byte_array_t &value) const
{
    static char syms[] = "0123456789ABCDEF";
    std::ostringstream ss;
    for (byte_array_t::const_iterator it = value.begin(); it != value.end(); ++it)
        ss << syms[((*it >> 4) & 0xf)] << syms[*it & 0xf];
    return ss.str();
}

template<>
byte_array_t EntryBase<byte_array_t>::parseString(const std::string &str) const
{
    static int nibbles[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15 };
    byte_array_t val;
    val.reserve(str.size()/2);
    for (std::string::const_iterator it = str.begin(); it < str.end(); it += 2) {
        char v = 0;
        if (std::isxdigit(*it)) {
            v = nibbles[std::toupper(*it) - '0'] << 4;
        } else {
            // failure
            LWARNINGF("Could not parse string '%s' as hex array at character '%c', using default value.", str.c_str(), *it);
            val = defaultValue_;
            break;
        }
        if (it + 1 < str.end() && std::isxdigit(*(it + 1))) {
            v += nibbles[std::toupper(*(it + 1)) - '0'];
        } else {
            // failure
            LWARNINGF("Could not parse string '%s' as hex array at character '%c', using default value.", str.c_str(), *(it+1));
            val = defaultValue_;
            break;
        }
        val.push_back(v);
    }
    return val;
}


}
}
