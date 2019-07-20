#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QDataStream>
#include "d_format.hpp"
#include <QFile>
#include <QKeyEvent>
#include <QThread>
#include <boost/asio.hpp>
#include <thread>

class AsioTcpServer;

class TcpConnection: public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(boost::asio::ip::tcp::socket socket, AsioTcpServer *asioTcpServer)
        : socket_(std::move(socket)), asioTcpServer(asioTcpServer)
    {
    }

    ~TcpConnection()
    {
        LOG("~TcpConnection");
    }

    void start()
    {
        doReadSize();
    }

    void doReadSize();

    void doReadBody(uint32_t size);

    void sendCmd(const std::string &cmd);

    void processData();

    boost::asio::ip::tcp::socket socket_;
    uint32_t msgSize;
    std::string data;

    AsioTcpServer *asioTcpServer;
};

class AsioTcpServer : public QObject
{
    Q_OBJECT
public:
    AsioTcpServer(boost::asio::io_service& io_service,
                const boost::asio::ip::tcp::endpoint& endpoint)
            : acceptor_(io_service, endpoint),
              socket_(io_service)
    {
        do_accept();
    }

    ~AsioTcpServer()
    {
        LOG("~AsioTcpServer");
    }

    void processData(const std::string &data)
    {
        emit onNewData(data);
    }

    void processDisconnect()
    {
        connection.reset();
        emit onDisconnect();
    }

    void sendCmd(const std::string &cmd)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (connection)
        {
            connection->sendCmd(cmd);
        }
    }

private:
    void do_accept();

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;

    std::mutex mutex;
    std::shared_ptr<TcpConnection> connection;

signals:
    void onNewConnection();
    void onDisconnect();
    void onNewData(const std::string &data);
};


class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent);
    ~TcpServer();

    void start();
    void stop();
signals:
    void newObj(const Obj &obj);
    void onNewConnection();

public slots:
    void newConnection();
    void disconnected();
    void settingsChanged();
    void sendKeyEvent(const Obj &obj);
    void onNewData(const std::string &data);

private:
    QFile outputFile;
    //QThread thread;
    boost::asio::io_service io_service;
    std::thread thread;

    std::unique_ptr<AsioTcpServer> asioTcpServer;
};

#endif // TCPSERVER_H
