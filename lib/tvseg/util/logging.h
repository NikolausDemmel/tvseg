#ifndef TVSEG_UTIL_LOGGING_H
#define TVSEG_UTIL_LOGGING_H



// FIXME: introduce define for tvseg to en-/disable qt logging support

// FIXME: improve multithreaded logging better. Maybe have separate thread to do the logging, other threads post messages to queue

#include <Qt>

#define _ELPP_QT_LOGGING
#define _ELPP_STL_LOGGING
#define _ELPP_THREAD_SAFE

//#define _DISABLE_VERBOSE_LOGS

#include "tvseg/3rdparty/easylogging++.h"


namespace boost {
template<typename T> class shared_array;
}


namespace easyloggingpp {

void vformatToBuffer(boost::shared_array<char>& buffer, size_t& buffer_size, const char* fmt, va_list args);
void formatToBuffer(boost::shared_array<char>& buffer, size_t& buffer_size, const char* fmt, ...);
std::string formatToString(const char* fmt, ...);
//inline const char* formatToString(const char* str) { return str; }

// FIXME: Make more efficient with global buffer

}



// TODO: Add note about code taken for rosconsole and License


//#define ROS_DEBUG(...) ROS_LOG(::ros::console::levels::Debug, ROSCONSOLE_DEFAULT_NAME, __VA_ARGS__)

//#define CINFOF(loggerId, ...) _ELPP_LOG_WRITER(loggerId, easyloggingpp::Level::Info) << formatToString(FMT, __VA_ARGS__)

//#define CLOG(LEVEL, loggerId) C##LEVEL(loggerId)

//#define LINFO CINFO("trivial")

#define LINFOF(...) LINFO << ::easyloggingpp::formatToString(__VA_ARGS__)
#define LWARNINGF(...) LWARNING << ::easyloggingpp::formatToString(__VA_ARGS__)
#define LDEBUGF(...) LDEBUG << ::easyloggingpp::formatToString(__VA_ARGS__)
#define LERRORF(...) LERROR << ::easyloggingpp::formatToString(__VA_ARGS__)
#define LFATALF(...) LFATAL << ::easyloggingpp::formatToString(__VA_ARGS__)


#endif // TVSEG_UTIL_LOGGING_H
