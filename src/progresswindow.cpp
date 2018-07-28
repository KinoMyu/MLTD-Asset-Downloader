#include <mutex>
#include <QTextStream>
#include "progresswindow.h"
#include "ui_progresswindow.h"

ProgressWindow::ProgressWindow(QWidget *parent) :
    QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::ProgressWindow),
    stop { false },
    file_count { 0 },
    downloaded_file_count { 0 }
{
    ui->setupUi(this);
    connect(ui->stopButton, SIGNAL(clicked(bool)), this, SLOT(stopDownload()));
    connect(ui->stopButton, SIGNAL(clicked(bool)), parent, SLOT(stop()));
    connect(this, SIGNAL(fileDownloaded()), this, SLOT(updateUI()));
}

ProgressWindow::~ProgressWindow()
{
    delete ui;
}

bool ProgressWindow::getStop()
{
    return stop;
}

void ProgressWindow::incrementFileCount()
{
    ++file_count;
    ui->progressBar->setMaximum(file_count);
}

void ProgressWindow::incrementDownloadedFileCount()
{
    std::unique_lock<std::mutex> lock(mtx);
    ++downloaded_file_count;
    fileDownloaded();
}

void ProgressWindow::updateUI()
{
    ui->progressBar->setValue(downloaded_file_count);
    QString result;
    QTextStream(&result) << downloaded_file_count << "/" << file_count << " Downloaded";
    ui->label->setText(result);
}

void ProgressWindow::stopDownload()
{
    stop = true;
}

int ProgressWindow::getFileCount()
{
    return file_count;
}

int ProgressWindow::getDownloadedCount()
{
    return downloaded_file_count;
}
