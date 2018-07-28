#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <deque>
#include "../curl/curl.h"

CURLcode openURL(const std::string &url, std::string &reply, char *errormsg);
std::deque<std::string> split(const std::string &str, char delim);
size_t writeToString(char *ptr, size_t size, size_t nmemb, void *str);

#endif // UTILS_H
