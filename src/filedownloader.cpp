#include <string>
#include "filedownloader.h"
#include "progresswindow.h"
#include "utils.h"
#include "curl/curl.h"

FileDownloader::FileDownloader(const std::string& url, const std::string& path, ProgressWindow* p)
    : url { url },
      path { path },
      p { p }
{}

void FileDownloader::run()
{
    if(!p->getStop())
    {
        CURL *curl;
        FILE *fp;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
            fp = fopen(path.c_str(), "wb");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            fclose(fp);
            if(res != CURLE_OK)
            {
                remove(path.c_str());
                return;
            }
            p->incrementDownloadedFileCount();
        }
    }
}
