#ifndef TVSEG_SETTINGS_SERIALIZERBASE_H
#define TVSEG_SETTINGS_SERIALIZERBASE_H

#include "serializer.h"
#include "backend.h"

namespace tvseg {
namespace settings {

class SerializerBase : public Serializer

{
public: // types
    typedef Backend::iterator iterator;
    typedef Backend::const_iterator const_iterator;

public:
    SerializerBase(BackendPtr backend, std::string location = "");

    // Serializer interface
public:
    void save(std::string location = "");
    void load(std::string location = "");

protected:
    virtual void saveImpl(std::string location) = 0;
    virtual void loadImpl(std::string location) = 0;

    iterator begin() { return backend_->begin(); }
    const_iterator begin() const { return backend_->begin(); }
    iterator end() { return backend_->end(); }
    const_iterator end() const { return backend_->end(); }

    BackendConstPtr backend() const { return backend_; }
    BackendPtr backend() { return backend_; }

private:
    BackendPtr backend_;
    std::string lastLocation_;
};

} // namespace settings
} // namespace tvseg

#endif // TVSEG_SETTINGS_SERIALIZERBASE_H
