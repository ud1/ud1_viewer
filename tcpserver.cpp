#include "tcpserver.h"
#include "settings.h"
#include <QMessageBox>
#include <QTcpSocket>
#include <cstdint>
#include "d_format.hpp"
#include <sstream>
#include <QDir>
#include <cstdlib>
#include <QDebug>

TcpServer::TcpServer(QObject *parent) : QObject(parent)
{
    connect(&tcpServer, &QTcpServer::newConnection, this, &TcpServer::newConnection);
}

void TcpServer::start()
{
    Settings settings;
    if (!tcpServer.listen(QHostAddress::Any, settings.port))
    {
        // TODO parent?
        QMessageBox::critical(nullptr, "Open TCP server error", tcpServer.errorString());
        return;
    }
}

void TcpServer::newConnection()
{
    if (socket)
    {
        socket->close();
    }
    socket = tcpServer.nextPendingConnection();
    if (!socket)
        return;

    socket->setSocketOption(QAbstractSocket::LowDelayOption, 0);

    connect(socket, &QAbstractSocket::disconnected,
                socket, &QObject::deleteLater);

    connect(socket, &QIODevice::readyRead, this, &TcpServer::readMessage);
    connect(socket, &QAbstractSocket::disconnected, this, &TcpServer::disconnected);
    dataStream.resetStatus();
    dataStream.setDevice(socket);

    if (outputFile.isOpen())
        outputFile.close();

    Settings settings;
    if (settings.saveToFile && !settings.outputDir.isEmpty())
    {
        QDir().mkpath(settings.outputDir);
        QString filePath = settings.outputDir;
        if (!filePath.endsWith("/"))
            filePath += "/";

        filePath += QString::number(time(0));
        filePath += ".vbin";

        outputFile.setFileName(filePath);
        outputFile.open(QIODevice::WriteOnly);
    }

    emit onNewConnection();
}

void TcpServer::readMessage()
{
    while(true)
    {
        dataStream.startTransaction();

        quint32 size;
        dataStream >> size;

        if (dataStream.status() != QDataStream::Ok)
        {
            dataStream.abortTransaction();
            return;
        }

        if (!size)
            return;

        std::string data;
        data.resize(size);
        dataStream.readRawData(&data[0], size);
        if (!dataStream.commitTransaction())
            return;

        std::istringstream iss(data);

        Obj obj = readObj(iss);
        if (iss)
        {
            emit newObj(obj);
        }

        if (outputFile.isOpen())
        {
            QDataStream outStream;
            outStream.setDevice(&outputFile);
            outStream << size;
            outStream.writeRawData(&data[0], size);
        }
    }
}

void TcpServer::disconnected()
{
    if (outputFile.isOpen())
        outputFile.close();

    socket = nullptr;
}

void TcpServer::settingsChanged()
{
    if (tcpServer.isListening())
        tcpServer.close();

    if (outputFile.isOpen())
        outputFile.close();

    start();
}

void TcpServer::sendKeyEvent(const Obj &obj)
{
    if (!socket)
        return;

    qDebug() << "SEND ";

    std::ostringstream oss;
    writeObj(oss, obj);

    std::string str = oss.str();
    QDataStream outStream;
    outStream.setDevice(socket);
    outStream << (uint32_t) str.size();
    int written = outStream.writeRawData(str.c_str(), str.size());

    socket->flush();
}
