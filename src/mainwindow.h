#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QListWidget>
#include <map>
#include <deque>
#include <thread>
#include <unordered_set>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void load();
    void addToList();
    void clearList();
    void removeFromList(QListWidgetItem *listitem);
    void filter();
    void search();
    void save();
    void stop();
    void diff();

private:
    void compareTree(const std::string& s);
    void filterPath(std::string& nString);
    void addToTree(std::deque<std::string> filename, QTreeWidget *parent);
    void addToSubTree(std::deque<std::string> &filename, std::string& built_path, QTreeWidgetItem *parent);
    void buildTree(const std::string& s);

    Ui::MainWindow *ui;
    std::string ver, unity_ver;
    std::deque<std::string> download_list;
    std::map<std::string, QTreeWidgetItem*> tree;
    std::map<std::string, std::string> filename_to_hash;
};

#endif // MAINWINDOW_H
