
#include "tvseg/util/helpers.h"

#include <boost/regex.hpp>

namespace tvseg {


std::string replaceString(const std::string &base, const std::string &match, const std::string &replace)
{
    boost::regex re(match);
    std::string result = boost::regex_replace(base, re, replace);
    return result;
}


std::string currentDateTimeString() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,80,"%Y%m%d_%H%M%S",timeinfo);
    return std::string(buffer);
}


std::string currentDateString() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,80,"%Y%m%d",timeinfo);
    return std::string(buffer);
}


}
