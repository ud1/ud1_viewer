#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QDataStream>
#include "d_format.hpp"
#include <QFile>
#include <QKeyEvent>

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

    void start();
signals:
    void newObj(const Obj &obj);
    void onNewConnection();

public slots:
    void newConnection();
    void readMessage();
    void disconnected();
    void settingsChanged();
    void sendKeyEvent(const Obj &obj);

private:
    QTcpServer tcpServer;
    QTcpSocket *socket = nullptr;
    QDataStream dataStream;
    QFile outputFile;
};

#endif // TCPSERVER_H
