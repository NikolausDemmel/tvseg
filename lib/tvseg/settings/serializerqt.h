#ifndef TVSEG_SETTINGS_SERIALIZERQT_H
#define TVSEG_SETTINGS_SERIALIZERQT_H

#include "serializerbase.h"

// TODO: Move to GUI frontend?

namespace tvseg {
namespace settings {

class SerializerQt : public SerializerBase
{
public:
    SerializerQt(BackendPtr backend, std::string location = "", bool autoLoad = false);

protected:
    virtual void saveImpl(std::string location);
    virtual void loadImpl(std::string location);

};

} // namespace settings
} // namespace tvseg

#endif // TVSEG_SETTINGS_SERIALIZERQT_H
