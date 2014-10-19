#ifndef TVSEG_SETTINGS_SERIALIZER_H
#define TVSEG_SETTINGS_SERIALIZER_H

#include <string>

#include <boost/shared_ptr.hpp>

namespace tvseg {
namespace settings {

class Serializer
{
public:
    virtual ~Serializer() {}

    virtual void save(std::string location = "") = 0;
    virtual void load(std::string location = "") = 0;
};

typedef boost::shared_ptr<Serializer> SerializerPtr;
typedef boost::shared_ptr<const Serializer> SerializerConstPtr;

} // namespace settings
} // namespace tvseg

#endif // TVSEG_SETTINGS_SERIALIZER_H
