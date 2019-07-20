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
#include <boost/endian/conversion.hpp>

using boost::asio::ip::tcp;

TcpServer::TcpServer(QObject *parent) : QObject(parent)
{

}

TcpServer::~TcpServer()
{
    stop();
}

void TcpServer::start()
{
    try
    {
        Settings settings;

        boost::asio::ip::tcp::endpoint endpoint(tcp::v4(), settings.port);
        asioTcpServer.reset();
        asioTcpServer.reset(new AsioTcpServer(io_service, endpoint));

        connect(asioTcpServer.get(), &AsioTcpServer::onNewConnection, this, &TcpServer::newConnection, Qt::QueuedConnection);
        connect(asioTcpServer.get(), &AsioTcpServer::onNewData, this, &TcpServer::onNewData);

        thread = std::thread([this](){
            LOG("RUN");
            io_service.run();
            LOG("RUN FINISH");
        });
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        QMessageBox::critical(nullptr, "Open TCP server error", e.what());
    }
}

void TcpServer::stop()
{
    LOG("STOP");
    io_service.stop();

    if (thread.joinable())
        thread.join();
}

void TcpServer::newConnection()
{
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

void TcpServer::onNewData(const std::string &data)
{
    std::istringstream iss(data);

    Obj obj = readObj(iss);
    if (iss)
    {
        emit newObj(obj);
    }

    if (outputFile.isOpen())
    {
        uint32_t size = data.size();
        QDataStream outStream;
        outStream.setDevice(&outputFile);
        outStream << size;
        outStream.writeRawData(&data[0], size);
        LOG("WRITTEN " << size);
    }
}

void TcpServer::disconnected()
{
    if (outputFile.isOpen())
        outputFile.close();

    qDebug("DISCONNECTED");
}

void TcpServer::settingsChanged()
{
    if (outputFile.isOpen())
        outputFile.close();

    stop();
    start();
}

void TcpServer::sendKeyEvent(const Obj &obj)
{
    std::ostringstream oss;
    writeObj(oss, obj);

    std::string str = oss.str();

    if (asioTcpServer)
        asioTcpServer->sendCmd(str);
}

void TcpConnection::doReadSize()
{
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(&msgSize, sizeof(msgSize)),
                            [this, self](boost::system::error_code ec, std::size_t /*length*/)
    {
        if (!ec)
        {
            doReadBody(boost::endian::big_to_native(self->msgSize));
        }
        else
        {
            asioTcpServer->processDisconnect();
        }
    });
}

void TcpConnection::doReadBody(uint32_t size)
{
    data.resize(size, ' ');
    auto self(shared_from_this());

    boost::asio::async_read(socket_,
                            boost::asio::buffer(data.data(), size),
                            [this, self, size](boost::system::error_code ec, std::size_t /*length*/)
    {
        if (!ec)
        {
            processData();
            doReadSize();
        }
        else
        {
            asioTcpServer->processDisconnect();
        }
    });
}

void TcpConnection::sendCmd(const std::string &cmd)
{
    uint32_t size = cmd.size();

    size = boost::endian::native_to_big(size);

    try
    {
        boost::asio::write(socket_, boost::asio::buffer(&size, sizeof(size)));
        boost::asio::write(socket_, boost::asio::buffer(cmd, cmd.size()));
    }
    catch (std::exception& e)
    {
        asioTcpServer->processDisconnect();
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

void TcpConnection::processData()
{
    asioTcpServer->processData(data);
}

void AsioTcpServer::do_accept()
{
    acceptor_.async_accept(socket_,
                           [this](boost::system::error_code ec)
    {
        if (!ec)
        {
            std::lock_guard<std::mutex> lock(mutex);
            connection = std::make_shared<TcpConnection>(std::move(socket_), this);
            connection->start();

            emit onNewConnection();
        }

        do_accept();
    });
}
