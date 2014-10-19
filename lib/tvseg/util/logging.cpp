
#include "tvseg/util/logging.h"


#include <boost/shared_array.hpp>


namespace easyloggingpp {


void vformatToBuffer(boost::shared_array<char>& buffer, size_t& buffer_size, const char* fmt, va_list args)
{
#ifdef _MSC_VER
    va_list arg_copy = args; // dangerous?
#else
    va_list arg_copy;
    va_copy(arg_copy, args);
#endif
#ifdef _MSC_VER
    size_t total = vsnprintf_s(buffer.get(), buffer_size, buffer_size, fmt, args);
#else
    size_t total = vsnprintf(buffer.get(), buffer_size, fmt, args);
#endif
    if (total >= buffer_size)
    {
        buffer_size = total + 1;
        buffer.reset(new char[buffer_size]);

#ifdef _MSC_VER
        vsnprintf_s(buffer.get(), buffer_size, buffer_size, fmt, arg_copy);
#else
        vsnprintf(buffer.get(), buffer_size, fmt, arg_copy);
#endif
    }
    va_end(arg_copy);
}

void formatToBuffer(boost::shared_array<char>& buffer, size_t& buffer_size, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vformatToBuffer(buffer, buffer_size, fmt, args);

    va_end(args);
}

std::string formatToString(const char* fmt, ...)
{
    boost::shared_array<char> buffer;
    size_t size = 0;

    va_list args;
    va_start(args, fmt);

    vformatToBuffer(buffer, size, fmt, args);

    va_end(args);

    return std::string(buffer.get(), size);
}

//#define INITIAL_BUFFER_SIZE 4096
//static boost::mutex g_print_mutex;
//static boost::shared_array<char> g_print_buffer(new char[INITIAL_BUFFER_SIZE]);
//static size_t g_print_buffer_size = INITIAL_BUFFER_SIZE;
//static boost::thread::id g_printing_thread_id;
//void print(FilterBase* filter, void* logger_handle, Level level,
//           const char* file, int line, const char* function, const char* fmt, ...)
//{
//    if (g_shutting_down)
//        return;

//    if (g_printing_thread_id == boost::this_thread::get_id())
//    {
//        fprintf(stderr, "Warning: recursive print statement has occurred.  Throwing out recursive print.\n");
//        return;
//    }

//    boost::mutex::scoped_lock lock(g_print_mutex);

//    g_printing_thread_id = boost::this_thread::get_id();

//    va_list args;
//    va_start(args, fmt);

//    vformatToBuffer(g_print_buffer, g_print_buffer_size, fmt, args);

//    va_end(args);

//    bool enabled = true;

//    if (filter)
//    {
//        FilterParams params;
//        params.file = file;
//        params.function = function;
//        params.line = line;
//        params.level = level;
//        params.logger = logger_handle;
//        params.message = g_print_buffer.get();
//        enabled = filter->isEnabled(params);
//        level = params.level;

//        if (!params.out_message.empty())
//        {
//            size_t msg_size = params.out_message.size();
//            if (g_print_buffer_size <= msg_size)
//            {
//                g_print_buffer_size = msg_size + 1;
//                g_print_buffer.reset(new char[g_print_buffer_size]);
//            }

//            memcpy(g_print_buffer.get(), params.out_message.c_str(), msg_size + 1);
//        }
//    }

//    if (enabled)
//    {
//        if (level == levels::Error)
//        {
//            g_last_error_message = g_print_buffer.get();
//        }
//        try
//        {
//            ::ros::console::impl::print(logger_handle, level, g_print_buffer.get(), file, function, line);
//        }
//        catch (std::exception& e)
//        {
//            fprintf(stderr, "Caught exception while logging: [%s]\n", e.what());
//        }
//    }

//    g_printing_thread_id = boost::thread::id();
//}


}
