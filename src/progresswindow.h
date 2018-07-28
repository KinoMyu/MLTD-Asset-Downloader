#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QDialog>
#include <mutex>

namespace Ui {
class ProgressWindow;
}

class ProgressWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressWindow(QWidget *parent = 0);
    ~ProgressWindow();
    bool getStop();
    void incrementFileCount();
    void incrementDownloadedFileCount();
    int getFileCount();
    int getDownloadedCount();

private:
    Ui::ProgressWindow *ui;
    bool stop;
    int file_count;
    int downloaded_file_count;
    std::mutex mtx;

signals:
    void fileDownloaded();

private slots:
    void stopDownload();
    void updateUI();
};

#endif // PROGRESSWINDOW_H
