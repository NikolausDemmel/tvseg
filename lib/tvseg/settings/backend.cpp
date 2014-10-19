#include "backend.h"

namespace tvseg {
namespace settings {

const Backend::key_type Backend::nameSeparator = "/";

Backend::key_type Backend::concatNames(key_type prefix, key_type name)
{
    if (prefix != key_type() && name != key_type()) {
        return prefix + nameSeparator + name;
    } else {
        return prefix + name;
    }
}

} // namespace settings
} // namespace tvseg
