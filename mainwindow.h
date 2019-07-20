#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "d_format.hpp"
#include "tcpserver.h"
#include <QTreeWidgetItem>
#include <set>
#include "stateholder.h"

namespace Ui {
class MainWindow;
}

class RefsView : public QMainWindow
{
    Q_OBJECT

public:
    explicit RefsView(QWidget *parent = 0);
    ~RefsView();

public slots:
    void resizeCols();
    void onframeChanged(std::shared_ptr<Frame> renderFrame, int totalCount);

signals:
    void onFileOpen(const QString &file);

private slots:
    void on_actionOptions_triggered();
    void itemExpanded(QTreeWidgetItem *item);
    void itemCollapsed(QTreeWidgetItem *item);

    void on_actionOpen_triggered();

private:
    void addObjToTree(const Obj &obj);
    Ui::MainWindow *ui;
    TcpServer tcpServer;
    StateHolder stateHolder;

    std::set<QString> expandedItems;
};

#endif // MAINWINDOW_H
