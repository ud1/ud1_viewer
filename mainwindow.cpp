#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include <QFileDialog>
#include "settings.h"
#include <QFile>
#include <sstream>

RefsView::RefsView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setSizes(QList<int>({1, 1}));

    connect(ui->view, SIGNAL(statusChanged(QString)), ui->viewStatus, SLOT(setText(QString)));
    connect(ui->treeWidget, SIGNAL(expanded(QModelIndex)), this, SLOT(resizeCols()));
    connect(this, &RefsView::frameChanged, ui->view, &View::changeFrame);
    connect(this, &RefsView::fieldSizeChange, ui->view, &View::fieldSizeChange);
    connect(this, &RefsView::field3d, ui->view, &View::field3d);
    connect(this, &RefsView::staticObject, ui->view, &View::addStaticObject);
    connect(&tcpServer, &TcpServer::newObj, this, &RefsView::process);
    connect(&tcpServer, &TcpServer::onNewConnection, this, &RefsView::onNewConnection);
    connect(ui->scrollTick, &QScrollBar::valueChanged, this, &RefsView::tickChanged);

    connect(ui->treeWidget, &QTreeWidget::itemCollapsed, this, &RefsView::itemCollapsed);
    connect(ui->treeWidget, &QTreeWidget::itemExpanded, this, &RefsView::itemExpanded);

    connect(ui->view, &View::keyEvent, &tcpServer, &TcpServer::sendKeyEvent);

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
    for (auto &&p : obj) {
        QTreeWidgetItem *pitem = new QTreeWidgetItem(parentItem);
        pitem->setText(0, QString::fromUtf8(p.first.data(), p.first.size()));

        std::string val = toString(p.second);
        pitem->setText(1, QString::fromUtf8(val.data(), val.size()));
    }
}

void RefsView::process(const Obj &obj)
{
    if (obj.type == "tick")
    {
        frames.push_back(std::make_shared<Frame>());
        Frame &frame = **frames.rbegin();
        frame.tick = obj.getIntProp("num");

        ui->scrollTick->setRange(0, frames.size() - 1);
        ui->treeWidget->clear();

        ui->scrollTick->setValue(std::max(0, (int) frames.size() - 2));
        if (frames.size() == 1)
            emit frameChanged(*frames.rbegin());
        return;
    }

    if (obj.type == "fieldSize")
    {
        int w = obj.getIntProp("w");
        int h = obj.getIntProp("h");

        if (w > 0 && h > 0)
            emit fieldSizeChange(w, h);
        return;
    }

    if (obj.type == "field3d")
    {
        P minP = obj.getPProp("minP");
        P maxP = obj.getPProp("maxP");
        double hMin = obj.getDoubleProp("hMin");
        double hMax = obj.getDoubleProp("hMax");
        double cellSize = obj.getDoubleProp("cellSize");

        emit field3d(minP, maxP, hMin, hMax, cellSize);
        return;
    }

    if (obj.type == "static")
    {
        for (const auto &p : obj.subObjs)
            emit staticObject(p.second);

        return;
    }

    if (frames.empty())
        return;

    Frame &frame = **frames.rbegin();
    {
        std::lock_guard<std::mutex> guard(frame.mutex);
        frame.objs.push_back(obj);
    }

    if (frames.size() == 1)
    {
        addObjToTree(obj);
        resizeCols();
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

void RefsView::onNewConnection()
{
    ui->treeWidget->clear();
    frames.clear();
    ui->scrollTick->setRange(0, 0);
    emit frameChanged(nullptr);
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

void RefsView::tickChanged(int tick)
{
    if (tick >= 0 && tick < (int) frames.size())
    {
        std::shared_ptr<Frame> frame = frames[tick];
        ui->treeWidget->clear();
        ui->logText->clear();

        for (const Obj &obj : frame->objs)
        {
            addObjToTree(obj);
        }

        resizeCols();
        emit frameChanged(frame);

        ui->statusBar->showMessage(QString("Tick: ") + QString::number(tick + 1) + "/" + QString::number(frames.size()));
    }
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

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    onNewConnection();

    QDataStream dataStream(&file);

    while(true)
    {
        quint32 size;
        dataStream >> size;

        if (dataStream.status() != QDataStream::Ok)
            break;

        std::string data;
        data.resize(size);
        dataStream.readRawData(&data[0], size);

        if (dataStream.status() != QDataStream::Ok)
            break;

        std::istringstream iss(data);

        Obj obj = readObj(iss);
        if (iss)
        {
            process(obj);
        }
    }

    file.close();
}
