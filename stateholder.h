#ifndef STATEHOLDER_H
#define STATEHOLDER_H

#include <QObject>
#include <QThread>
#include <QTime>
#include <QTimer>
#include "d_format.hpp"

class StateHolder : public QObject
{
    Q_OBJECT
public:
    explicit StateHolder();
    ~StateHolder();

signals:
    void frameChanged(std::shared_ptr<Frame> renderFrame, int totalCount);
    void fieldSizeChange(int w, int h, int cellSize);
    void field3d(const P &minP, const P &maxP, double hMin, double hMax, double cellSize);
    void staticObject(const SObj &sobj);

public slots:
    void process(const Obj &obj);
    void onNewConnection();
    void tickChanged(int tick);
    void update();
    void onFileOpen(const QString &fileName);

private:
    QThread thread;
    std::vector<std::shared_ptr<Frame>> frames;
    bool frameWasChanged = false;
    QTime qtime;
    QTimer *timer;
};

#endif // STATEHOLDER_H
