#include <QJsonDocument>
#include <QJsonObject>
#include <QFileDialog>
#include <QThreadPool>
#include <QTextStream>
#include <QScreen>
#include <QDir>
#include <sstream>
#include <deque>
#include <map>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "progresswindow.h"
#include "filedownloader.h"
#include "utils.h"
#include "../curl/curl.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->downloadButton, SIGNAL(clicked(bool)), this, SLOT(save()));
    connect(ui->loadButton, SIGNAL(clicked(bool)), this, SLOT(load()));
    connect(ui->addButton, SIGNAL(clicked(bool)), this, SLOT(addToList()));
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removeFromList(QListWidgetItem*)));
    connect(ui->clearButton, SIGNAL(clicked(bool)), this, SLOT(clearList()));
    connect(ui->diffButton, SIGNAL(clicked(bool)), this, SLOT(diff()));
    connect(ui->filterBox, SIGNAL(returnPressed()), this, SLOT(filter()));
    connect(ui->searchBox, SIGNAL(returnPressed()), this, SLOT(search()));

    curl_global_init(CURL_GLOBAL_ALL);
}

MainWindow::~MainWindow()
{
    curl_global_cleanup();
    delete ui;
}

void MainWindow::load()
{
    std::string s;

    ui->statusBar->showMessage("Connecting to Matsurihime API");

    char curlerror[CURL_ERROR_SIZE];
    if(openURL("https://api.matsurihi.me/mltd/v1/version/latest", s, curlerror) != CURLE_OK)
    {
        ui->statusBar->showMessage("ERROR: Could not connect to Matsurihime API (" + QString(curlerror) + ")");
        return;
    }
    QJsonDocument jsonResponse = QJsonDocument::fromJson(QByteArray::fromStdString(s));
    ver = std::to_string(jsonResponse["res"]["version"].toInt());
    std::string path = jsonResponse["res"]["indexName"].toString().toStdString();

    ui->statusBar->showMessage("Downloading manifest file");

    if(openURL("https://td-assets.bn765.com/" + ver + "/production/" + "2017.3/" + "Android/" + path, s, curlerror) != CURLE_OK)
    {
        ui->statusBar->showMessage("ERROR: Could not connect to MLTD servers (" + QString(curlerror) + ")");
        return;
    }

    ui->statusBar->showMessage("Building tree");
    setUpdatesEnabled(false);
    try
    {
        buildTree(s);
    }
    catch (...)
    {
        ui->statusBar->showMessage("ERROR: Could not build tree");
        return;
    }
    ui->assetTree->sortItems(0, Qt::AscendingOrder);
    setUpdatesEnabled(true);
    ui->statusBar->showMessage("Successfully loaded asset version " + QString::fromStdString(ver));
}

void MainWindow::diff()
{
    std::string s;

    ui->statusBar->showMessage("Connecting to Matsurihime API");

    char curlerror[CURL_ERROR_SIZE];
    if(openURL("https://api.matsurihi.me/mltd/v1/version/assets/" + ui->diffVersion->text().toStdString(), s, curlerror) != CURLE_OK)
    {
        ui->statusBar->showMessage("ERROR: Could not connect to Matsurihime API (" + QString(curlerror) + ")");
        return;
    }
    QJsonDocument jsonResponse = QJsonDocument::fromJson(QByteArray::fromStdString(s));
    if(jsonResponse["error"]["status"].toInt() == 404)
    {
        ui->statusBar->showMessage("ERROR: Could not find asset version specified");
        return;
    }
    ver = std::to_string(jsonResponse["version"].toInt());
    std::string path = jsonResponse["indexName"].toString().toStdString();

    ui->statusBar->showMessage("Downloading manifest file");

    if(openURL("https://td-assets.bn765.com/" + ver + "/production/" + "2017.3/" + "Android/" + path, s, curlerror) != CURLE_OK)
    {
        ui->statusBar->showMessage("ERROR: Could not connect to MLTD servers (" + QString(curlerror) + ")");
        return;
    }
    if(s.substr(0, 5) == "<?xml")
    {
        ui->statusBar->showMessage("ERROR: Asset version is too old to diff");
        return;
    }

    ui->statusBar->showMessage("Diffing...");
    setUpdatesEnabled(false);
    compareTree(s);
    setUpdatesEnabled(true);

    ui->statusBar->showMessage("New items will be highlighted in yellow");
}

void MainWindow::compareTree(const std::string& s)
{
    std::unordered_set<std::string> unseenitems;
    unseenitems.reserve(tree.size());
    for(auto item : tree)
    {
        unseenitems.insert(item.first);
        if(item.second != nullptr)
        {
            item.second->setBackgroundColor(0, QColor(255, 255, 255, 255));
        }
    }
    std::stringstream ss(s);
    ss.ignore();
    while(!ss.eof())
    {
        int magic_byte = ss.get();
        ss.ignore();

        if(magic_byte != 0xCE)
        {
            ss.ignore();
            magic_byte = ss.get();
            if(ss.eof())
            {
                break;
            }
            if(magic_byte == 0xD9)
            {
                ss.ignore();
            }

            std::string nString;
            std::getline(ss, nString, (char)0x93);
            ss.ignore();

            int nSize = ss.get();
            std::string hString(nSize, ' ');
            ss.read(&hString[0], nSize);
            ss.ignore();

            nSize = ss.get();
            std::string hName(nSize, ' ');
            ss.read(&hName[0], nSize);

            filterPath(nString);

            std::replace(nString.begin(), nString.end(), '_', '/');
            std::deque<std::string> list = split(nString, '/');

            auto it = unseenitems.find(nString);
            if(it != unseenitems.end())
            {
                unseenitems.erase(it);
            }
        }
    }
    for(auto name : unseenitems)
    {
        auto it = filename_to_hash.find(name);
        if(it != filename_to_hash.end())
        {
            QTreeWidgetItem* node = tree[name];
            while(node != nullptr)
            {
                node->setBackgroundColor(0, QColor(255, 255, 0, 100));
                node = node->parent();
            }
        }
    }
}

void MainWindow::filterPath(std::string& nString)
{
    if(nString[0] >= '0' && nString[0] <= '9')
    {
        size_t p = nString.find_first_of('_');
        if(p > 6 && p != std::string::npos)
        {
            nString = "characard/" + nString;
        }
        else
        {
            nString = "charasign/" + nString;
        }
    }
    if(nString.substr(0, 4) == "blog")
    {
        nString = "blog/" + nString;
    }
    if(nString.substr(0, 4) == "ex4c")
    {
        nString = "4koma/" + nString;
    }
    if(nString.substr(0, 4) == "exwb")
    {
        nString = "whiteboard/" + nString;
    }
    if(nString.substr(0, 4) == "room")
    {
        nString = "room/" + nString;
    }
    if(nString.substr(0, 5) == "gasha")
    {
        nString = "gasha/" + nString;
    }
    if(nString.substr(0, 5) == "stage")
    {
        nString = "stage/" + nString;
    }
    if(nString.substr(0, 5) == "event")
    {
        nString = "event/" + nString;
    }
    if(nString.substr(0, 4) == "tuna")
    {
        nString = "tuna/" + nString;
    }
    if(nString.substr(0, 9) == "coingasha")
    {
        nString = "coingasha/" + nString;
    }
    if(nString.substr(0, 8) == "tutorial")
    {
        nString = "tutorial/" + nString;
    }
    if(nString.substr(0, 8) == "yokosuka")
    {
        nString = "yokosuka/" + nString;
    }
    if(nString.substr(0, 9) == "selection")
    {
        nString = "selection/" + nString;
    }
    if(nString.substr(0, 16) == "costumesalesinfo")
    {
        nString = "costumesalesinfo/" + nString;
    }
}

void MainWindow::buildTree(const std::string &s)
{
    std::stringstream ss(s);
    ss.ignore();
    while(!ss.eof())
    {
        int magic_byte = ss.get();
        ss.ignore();

        if(magic_byte != 0xCE)
        {
            ss.ignore();
            magic_byte = ss.get();
            if(ss.eof())
            {
                break;
            }
            if(magic_byte == 0xD9)
            {
                ss.ignore();
            }

            std::string nString;
            std::getline(ss, nString, (char)0x93);
            ss.ignore();

            int nSize = ss.get();
            std::string hString(nSize, ' ');
            ss.read(&hString[0], nSize);
            ss.ignore();

            nSize = ss.get();
            std::string hName(nSize, ' ');
            ss.read(&hName[0], nSize);

            filterPath(nString);

            std::replace(nString.begin(), nString.end(), '_', '/');
            std::deque<std::string> list = split(nString, '/');

            addToTree(list, ui->assetTree);
            filename_to_hash[nString] = hName;
        }
    }
}

void MainWindow::addToTree(std::deque<std::string> filename, QTreeWidget *parent)
{
    if(filename.empty()) return;
    std::string built_path = filename[0];
    if(tree.find(built_path) == tree.end())
    {
        tree[built_path] = new QTreeWidgetItem(parent);
        tree[built_path]->setData(0, 5, QString::fromStdString(built_path));
        tree[built_path]->setText(0, QString::fromStdString(filename[0]));
    }
    filename.pop_front();
    addToSubTree(filename, built_path, tree[built_path]);
}

void MainWindow::addToSubTree(std::deque<std::string> &filename, std::string& built_path, QTreeWidgetItem *parent)
{
    if(filename.empty()) return;
    built_path += "/" + filename[0];
    if(tree.find(built_path) == tree.end())
    {
        tree[built_path] = new QTreeWidgetItem(parent);
        tree[built_path]->setData(0, 5, QString::fromStdString(built_path));
        tree[built_path]->setText(0, QString::fromStdString(filename[0]));
    }
    filename.pop_front();
    addToSubTree(filename, built_path, tree[built_path]);
}

void MainWindow::addToList()
{
    for(auto item : ui->assetTree->selectedItems())
    {
        bool add = true;
        QString path = item->data(0, 5).toString();
        if(item->childCount() > 0)
        {
            path += "/*";
        }
        for(int i = 0; i < ui->listWidget->count(); ++i)
        {
            QListWidgetItem* listitem = ui->listWidget->item(i);
            QRegExp regex(listitem->text());
            regex.setPatternSyntax(QRegExp::Wildcard);
            if(regex.exactMatch(path))
            {
                add = false;
                break;
            }
            regex.setPattern(path);
            if(regex.exactMatch(listitem->text()))
            {
                ui->listWidget->removeItemWidget(listitem);
                delete listitem;
                --i;
            }
        }
        if(add)
        {
            ui->listWidget->addItem(path);
        }
    }
}

void MainWindow::clearList()
{
    ui->listWidget->clear();
}

void MainWindow::removeFromList(QListWidgetItem *listitem)
{
    ui->listWidget->removeItemWidget(listitem);
    delete listitem;
}

void MainWindow::filter()
{
    QString filter_query = ui->filterBox->text();
    QStringList filter_terms = filter_query.split(' ', QString::SkipEmptyParts);
    for(QTreeWidgetItemIterator it(ui->assetTree, QTreeWidgetItemIterator::Hidden); *it; ++it)
    {
        (*it)->setHidden(false);
    }
    for(QTreeWidgetItemIterator it(ui->assetTree); *it; ++it)
    {
        for(auto term : filter_terms)
        {
            if(!(*it)->data(0, 5).toString().contains(term, Qt::CaseInsensitive))
            {
                (*it)->setHidden(true);
                break;
            }
        }
    }
    for(QTreeWidgetItemIterator it(ui->assetTree, QTreeWidgetItemIterator::NotHidden); *it; ++it)
    {
        auto parent = (*it)->parent();
        while(parent && parent->isHidden())
        {
            parent->setHidden(false);
            parent = parent->parent();
        }
    }
}

void MainWindow::search()
{
    QTreeWidgetItemIterator it(ui->assetTree, QTreeWidgetItemIterator::NotHidden);
    if(ui->assetTree->selectedItems().length() > 0)
    {
        it = QTreeWidgetItemIterator(ui->assetTree->selectedItems()[ui->assetTree->selectedItems().length() - 1], QTreeWidgetItemIterator::NotHidden);
        ++it;
        QTreeWidgetItemIterator iter(ui->assetTree,QTreeWidgetItemIterator::Selected);
        for( ; *iter; ++iter)
        {
            (*iter)->setSelected(false);
        }
    }
    QString search_query = ui->searchBox->text();
    QStringList search_terms = search_query.split(' ', QString::SkipEmptyParts);
    for( ; *it; ++it)
    {
        bool found = true;
        for(auto term : search_terms)
        {
            if(!(*it)->text(0).contains(term, Qt::CaseInsensitive))
            {
                found = false;
                break;
            }
        }
        if(found)
        {
            (*it)->setSelected(true);
            ui->assetTree->scrollToItem(*it);
            break;
        }
    }
}

void MainWindow::save()
{
    ProgressWindow p(this);
    p.open();

    QThreadPool pool;
    pool.setMaxThreadCount(pool.maxThreadCount() * 4);

    QString root_dir = QFileDialog::getExistingDirectory(this, tr("Save to..."), "", QFileDialog::ShowDirsOnly);
    if(root_dir != "")
    {
        QString filter_query = ui->downloadFilter->text();
        QStringList filter_terms = filter_query.split(' ', QString::SkipEmptyParts);
        ui->statusBar->showMessage("Downloading...");
        for(int i = 0; i < ui->listWidget->count(); ++i)
        {
            QListWidgetItem* listitem = ui->listWidget->item(i);
            QString name = listitem->text();
            QRegExp regex(name);
            regex.setPatternSyntax(QRegExp::Wildcard);
            auto it = filename_to_hash.lower_bound(name.toLocal8Bit().toStdString());
            for( ; it != filename_to_hash.end() && regex.exactMatch(QString::fromStdString(it->first)); ++it)
            {
                QString relative_path = QString::fromStdString(it->first);
                bool found = true;
                for(auto term : filter_terms)
                {
                    if(!relative_path.contains(term, Qt::CaseInsensitive))
                    {
                        found = false;
                        break;
                    }
                }
                if(found)
                {
                    p.incrementFileCount();
                    QDir relative_folder_path = QFileInfo(relative_path).dir();
                    QString absolute_folder_path = relative_folder_path.filePath(root_dir + "/" + relative_folder_path.path());
                    relative_folder_path.mkpath(absolute_folder_path);

                    std::string url = "https://td-assets.bn765.com/" + ver + "/production/" + "2017.3/" + "Android/" + it->second;
                    std::string full_path = relative_folder_path.filePath(root_dir + "/" + relative_path).toLocal8Bit().toStdString();

                    FileDownloader* download_task = new FileDownloader(url, full_path, &p);
                    pool.start(download_task);
                }
            }
        }

        QScreen* screen = QApplication::primaryScreen();
        qreal refresh_rate = screen->refreshRate();
        if(refresh_rate <= 0)
        {
            refresh_rate = 60;
        }
        while(!pool.waitForDone(1000 / refresh_rate))
        {
            QCoreApplication::processEvents();
        }
        QString result;
        if(p.getStop())
        {
            QTextStream(&result) << "Download stopped. ";
        }
        else if(p.getDownloadedCount() < p.getFileCount())
        {
            QTextStream(&result) << "Download error, try checking your connection. ";
        }
        else
        {
            QTextStream(&result) << "Download complete. ";
        }
        QTextStream(&result) << p.getDownloadedCount() << "/" << p.getFileCount() << " files downloaded.";
        ui->statusBar->showMessage(result);
    }
}

void MainWindow::stop()
{
    ui->statusBar->showMessage("Stopping download. Waiting for current downloads to finish.");
}
