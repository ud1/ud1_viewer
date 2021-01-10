#include "stateholder.h"
#include <QTimer>
#include <QFile>
#include <QDataStream>

StateHolder::StateHolder() : QObject()
{
    moveToThread(&thread);

    thread.start();

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(16);

    qtime.start();
}

StateHolder::~StateHolder()
{
    delete timer;
    thread.exit();
    thread.wait();
}

void StateHolder::process(const Obj &obj)
{
    if (obj.type == "tick")
    {
        frames.push_back(std::make_shared<Frame>());
        Frame &frame = **frames.rbegin();
        frame.tick = frames.size() - 1;


        frameWasChanged = true;
        /*int el = qtime.elapsed();
        if (el > 16)
        {
            update();
            qtime.start();
        }*/

        if (frames.size() == 1)
            emit frameChanged(*frames.rbegin(), 0);
        return;
    }

    if (obj.type == "fieldSize")
    {
        int w = obj.getIntProp("w");
        int h = obj.getIntProp("h");
        int cellSize = obj.getIntProp("cellSize");

        if (w > 0 && h > 0)
            emit fieldSizeChange(w, h, cellSize);
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
        std::lock_guard<std::recursive_mutex> guard(frame.mutex);
        frame.objs.push_back(obj);
    }

    if (frames.size() == 1)
    {
        frameWasChanged = true;
    }
}

void StateHolder::onNewConnection()
{
    frames.clear();
    emit frameChanged(nullptr, 0);
}

void StateHolder::tickChanged(int tick)
{
    if (tick < (int) frames.size())
    {
        emit frameChanged(frames[tick], frames.size());
    }
}

void StateHolder::update()
{
    if (frameWasChanged)
    {
        frameWasChanged = false;
        if (frames.size() > 1)
            emit frameChanged(frames[frames.size() - 2], frames.size());
    }
}

void StateHolder::onFileOpen(const QString &fileName)
{
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
