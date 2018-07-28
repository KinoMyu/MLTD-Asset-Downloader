#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QRunnable>
#include <string>
#include "progresswindow.h"

class FileDownloader : public QRunnable
{
public:
    FileDownloader(const std::string& url, const std::string& path, ProgressWindow* p);
    void run();

private:
    std::string url;
    std::string path;
    ProgressWindow* p;
};

#endif // FILEDOWNLOADER_H
