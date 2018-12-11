#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "d_format.hpp"
#include "tcpserver.h"
#include <QTreeWidgetItem>
#include <set>

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
    void process(const Obj &obj);
    void onNewConnection();

signals:
    void frameChanged(std::shared_ptr<Frame> renderFrame);
    void fieldSizeChange(int w, int h);
    void field3d(const P &minP, const P &maxP, double hMin, double hMax, double cellSize);
    void staticObject(const SObj &sobj);

private slots:
    void on_actionOptions_triggered();
    void tickChanged(int tick);
    void itemExpanded(QTreeWidgetItem *item);
    void itemCollapsed(QTreeWidgetItem *item);

    void on_actionOpen_triggered();

private:
    void addObjToTree(const Obj &obj);
    Ui::MainWindow *ui;
    std::vector<std::shared_ptr<Frame>> frames;
    TcpServer tcpServer;

    std::set<QString> expandedItems;
};

#endif // MAINWINDOW_H
