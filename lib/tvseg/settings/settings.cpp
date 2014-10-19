#include "settings.h"

namespace tvseg {
namespace settings {

Settings::Settings(const BackendPtr backend, const key_type prefix) :
    backend_(backend),
    prefix_(prefix)
{
}

//Settings::Settings(const BackendPtr backend, const key_type prefix, const Settings * const parent) :
//    backend_(backend),
//    prefix_(Backend::concatNames((parent ? parent->prefix() : key_type()), prefix))
//{
//}

} // namespace settings
} // namespace tvseg
