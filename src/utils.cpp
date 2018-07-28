#include <sstream>
#include <string>
#include <deque>
#include "utils.h"
#include "curl/curl.h"


CURLcode openURL(const std::string &url, std::string &reply, char *errormsg)
{
    reply = "";
    CURL *curl;
    CURLcode code = CURLE_FAILED_INIT;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);
        if(errormsg)
        {
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errormsg);
        }
        code = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return code;
}

std::deque<std::string> split(const std::string& str, char delim)
{
    std::deque<std::string> list;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim))
    {
        list.push_back(token);
    }
    return list;
}

size_t writeToString(char *ptr, size_t size, size_t nmemb, void *str)
{
    ((std::string *)str)->append((char *)ptr, size * nmemb);
    return size * nmemb;
}
