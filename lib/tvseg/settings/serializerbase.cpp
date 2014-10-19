#include "serializerbase.h"

namespace tvseg {
namespace settings {

SerializerBase::SerializerBase(BackendPtr backend, std::string location):
    backend_(backend),
    lastLocation_(location)
{
}

void SerializerBase::save(std::string location)
{
    if (location.empty()) {
        location = lastLocation_;
    } else {
        lastLocation_ = location;
    }

    saveImpl(location);
}

void SerializerBase::load(std::string location)
{
    if (location.empty()) {
        location = lastLocation_;
    } else {
        lastLocation_ = location;
    }

    loadImpl(location);
}

} // namespace settings
} // namespace tvseg
