#ifndef TVSEG_UTIL_HELPERS_H
#define TVSEG_UTIL_HELPERS_H

#include <string>

namespace tvseg {

std::string replaceString(const std::string &base, const std::string &match, const std::string &replace);

std::string currentDateTimeString();
std::string currentDateString();

}

#endif // TVSEG_UTIL_HELPERS_H
