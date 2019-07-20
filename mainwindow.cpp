#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include <QFileDialog>
#include "settings.h"
#include <QFile>
#include <sstream>

RefsView::RefsView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tcpServer(this)
{
    ui->setupUi(this);
    ui->splitter->setSizes(QList<int>({1, 1}));

    connect(ui->view, SIGNAL(statusChanged(QString)), ui->viewStatus, SLOT(setText(QString)));
    connect(ui->treeWidget, SIGNAL(expanded(QModelIndex)), this, SLOT(resizeCols()));
    connect(&stateHolder, &StateHolder::frameChanged, ui->view, &View::changeFrame);
    connect(&stateHolder, &StateHolder::frameChanged, this, &RefsView::onframeChanged);
    connect(&stateHolder, &StateHolder::fieldSizeChange, ui->view, &View::fieldSizeChange);
    connect(&stateHolder, &StateHolder::field3d, ui->view, &View::field3d);
    connect(&stateHolder, &StateHolder::staticObject, ui->view, &View::addStaticObject);
    connect(&tcpServer, &TcpServer::newObj, &stateHolder, &StateHolder::process);
    connect(&tcpServer, &TcpServer::onNewConnection, &stateHolder, &StateHolder::onNewConnection);
    connect(ui->scrollTick, &QScrollBar::valueChanged, &stateHolder, &StateHolder::tickChanged);

    connect(ui->treeWidget, &QTreeWidget::itemCollapsed, this, &RefsView::itemCollapsed);
    connect(ui->treeWidget, &QTreeWidget::itemExpanded, this, &RefsView::itemExpanded);

    connect(ui->view, &View::keyEvent, &tcpServer, &TcpServer::sendKeyEvent);
    connect(this, &RefsView::onFileOpen, &stateHolder, &StateHolder::onFileOpen);

    tcpServer.start();
}

RefsView::~RefsView()
{
    delete ui;
}

QString getItemName(QTreeWidgetItem *item) {
    QString t = item->text(0);
    QTreeWidgetItem *parent = dynamic_cast<QTreeWidgetItem *>(item->parent());
    if (parent)
        return parent->text(0) + "->" + t;

    return t;
}

void addSObjToTree(const SObj &obj, QTreeWidgetItem *parentItem)
{
    for (const auto &p : obj) {
        QTreeWidgetItem *pitem = new QTreeWidgetItem(parentItem);
        pitem->setText(0, QString::fromUtf8(p.first.data(), p.first.size()));

        std::string val = toString(p.second);
        pitem->setText(1, QString::fromUtf8(val.data(), val.size()));
    }
}

void RefsView::addObjToTree(const Obj &obj)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, QString::fromUtf8(obj.type.data(), obj.type.size()));

    QString parentName = getItemName(item);
    if (expandedItems.count(parentName))
        item->setExpanded(true);

    addSObjToTree(obj.props, item);

    for (auto &&o : obj.subObjs) {
        QTreeWidgetItem *oitem = new QTreeWidgetItem(item);
        oitem->setText(0, QString::fromUtf8(o.first.data(), o.first.size()));

        QString itemName = getItemName(oitem);
        if (expandedItems.count(itemName))
            oitem->setExpanded(true);

        addSObjToTree(o.second, oitem);
    }

    if (obj.type == "log")
    {
        std::string str = getStr("text", obj.props);
        ui->logText->appendPlainText(QString::fromUtf8(str.c_str()));
    }
}

void RefsView::onframeChanged(std::shared_ptr<Frame> renderFrame, int totalCount)
{
    ui->treeWidget->clear();
    ui->logText->clear();

    if (!renderFrame)
    {
        ui->scrollTick->setRange(0, 0);
        ui->statusBar->showMessage("");
    }
    else
    {
        std::lock_guard<std::recursive_mutex> guard(renderFrame->mutex);

        bool oldState = ui->scrollTick->blockSignals(true);
        ui->scrollTick->setRange(0, totalCount - 1);
        ui->scrollTick->setValue(renderFrame->tick);
        ui->scrollTick->blockSignals(oldState);

        for (const Obj &obj : renderFrame->objs)
        {
            addObjToTree(obj);
        }

        resizeCols();

        ui->statusBar->showMessage(QString("Tick: ") + QString::number(renderFrame->tick + 1) + "/" + QString::number(totalCount));
    }
}

void RefsView::resizeCols()
{
    ui->treeWidget->resizeColumnToContents(0);
}

void RefsView::on_actionOptions_triggered()
{
    SettingsDialog settingsDialog(this);
    connect(&settingsDialog, &SettingsDialog::settingsChanged, &tcpServer, &TcpServer::settingsChanged);
    connect(&settingsDialog, &SettingsDialog::settingsChanged, ui->view, &View::settingsChanged);
    settingsDialog.setModal(true);
    settingsDialog.exec();
}

void RefsView::itemExpanded(QTreeWidgetItem *item)
{
    QString name = getItemName(item);
    expandedItems.insert(name);
}

void RefsView::itemCollapsed(QTreeWidgetItem *item)
{
    QString name = getItemName(item);
    expandedItems.erase(name);
}

void RefsView::on_actionOpen_triggered()
{
    Settings settings;
    QString fileName = QFileDialog::getOpenFileName(this, "Open file", settings.outputDir, "Dump files (*.vbin)", nullptr, QFileDialog::DontResolveSymlinks);
    if (fileName.isEmpty())
        return;

    emit onFileOpen(fileName);
}
