#ifndef TVSEG_UTIL_ERRORHANDLING_H
#define TVSEG_UTIL_ERRORHANDLING_H

#include "tvseg/util/sharedptr.h"

#include <string>
#include <iostream>
#include <exception>
#include <sstream>


// TODO: use logging facilities instead of cout (can't for now as CUDA + barfs on the logging header)


namespace tvseg {


class Exception: public std::exception
{
    public:
        explicit Exception(const char *message = "", const char *func = NULL, const char *file = NULL, unsigned int line = 0);
        explicit Exception(const char *msg1, const char *msg2, const char *func = NULL, const char *file = NULL, unsigned int line = 0);
        explicit Exception(const std::string &message, const char *func = NULL, const char *file = NULL, unsigned int line = 0);

        virtual ~Exception() throw() {}

        inline const char * func() const { return func_; }
        inline const char * file() const { return file_; }
        inline unsigned int lineNumer() const { return line_; }

        inline const char* what() const throw()
        {
            return message_->c_str();
        }

    private:
        SharedPtr<std::string> message_; //< exceptions shall not throw on copy const, so use smart ptr
        const char* func_;
        const char* file_;
        const unsigned int line_;
};

std::ostream & operator << (std::ostream &oss, const Exception &rhs);



namespace _detail {

inline void _assert(const bool condition, const char *const message, const char *const func, const char *const file, int const line)
{
    if (!condition) {
        tvseg::Exception e(message, func, file, line);
        std::cerr << e << std::endl;
        throw e;
    }
}

} // namespace _detail
} // namespace tvseg


#define ASSERT(C) tvseg::_detail::_assert(C, #C " assertion faild!", __func__, __FILE__, __LINE__)


// M should be a string literal.
#define ASSERT_MSG(C, M) tvseg::_detail::_assert(C, #C " assertion faild! Message:" M, __func__, __FILE__, __LINE__)


// This macro is identical in every respect to ASSERT(C) but it does get compiled to a
// no-op in release builds. It is therefor of utmost importance to not put statements into
// this macro that cause side effects required for correct program execution.
#ifdef _DEBUG
# define DEBUG_ASSERT(C) tvseg::_detail::_assert(C, #C " debug assertion faild!", __func__, __FILE__, __LINE__)
#else
# define DEBUG_ASSERT(C)
#endif


#define ASSERT_NOT_NULL(P) tvseg::_detail::_assert((P) == 0, #P " not null assertion faild!", __func__, __FILE__, __LINE__)


#define NOT_IMPLEMENTED() tvseg::_detail::_assert(false, "Implementation missing!", __func__, __FILE__, __LINE__)


#define NOT_REACHABLE() tvseg::_detail::_assert(false, "Should be not reachable!", __func__, __FILE__, __LINE__)


#endif // TVSEG_UTIL_ERRORHANDLING_H
