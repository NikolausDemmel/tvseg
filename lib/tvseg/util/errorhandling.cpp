
#include "tvseg/util/errorhandling.h"

#include <sstream>

namespace tvseg {


    Exception::Exception(const char *message, const char *func, const char *file, unsigned int line) :
        message_(new std::string()),
        func_(func),
        file_(file),
        line_(line)
    {
        std::ostringstream oss;
        if (file_ && line_ > 0) {
            oss << file_ << ":" << line_ << " ";
        }

        if (message) {
            oss << message;
        }

        if (func_) {
            oss << " \"" << func_ << "\"";
        }

        *message_ = oss.str();
    }

    Exception::Exception(const char *msg1, const char *msg2, const char *func, const char *file, unsigned int line) :
        message_(new std::string),
        func_(func),
        file_(file),
        line_(line)
    {
        std::ostringstream oss;
        if (file_ && line_ > 0) {
            oss << std::string(file_) << ":" << line_ << " ";
        }

        if (msg1) {
            oss << msg1;
        }

        if (msg2) {
            oss << msg2;
        }

        if (func_) {
            oss << " \"" << func_ << "\"";
        }

        *message_ = oss.str();
    }

    Exception::Exception(const std::string &message, const char *func, const char *file, unsigned int line) :
        message_(new std::string),
        func_(func),
        file_(file),
        line_(line)
    {
        std::ostringstream oss;
        if (file_ && line_ > 0) {
            oss << std::string(file_) << ":" << line_ << ": ";
        }

        oss << message;

        if (func_) {
            oss << " \"" << func_ << "\"";
        }

        *message_ = oss.str();
    }

    std::ostream & operator << (std::ostream &oss, const Exception &rhs)
    {
        oss << rhs.what();
        return oss;
    }


}
