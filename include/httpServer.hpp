#include <iostream>
#include <string>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <functional>

#include "util.h"

class httpServer
{
public:
    httpServer(muduo::net::EventLoop* loop,
        const muduo::net::InetAddress& listenAddr,
        const std::string& nameArg):_tcpServer(loop,listenAddr,nameArg),_loop(loop)
        {
            _tcpServer.setConnectionCallback(std::bind(&httpServer::ConnectionCallback, this  ,std::placeholders::_1));
            _tcpServer.setMessageCallback(std::bind(&httpServer::MessageCallback,this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
        }
    void setThreadNum(int num = 2)
    {
        _tcpServer.setThreadNum(num);
    }
    void start()
    {
        _tcpServer.start();
    }

private:
    void ConnectionCallback(const muduo::net::TcpConnectionPtr& conn)
    {
        if(!conn->connected())
        {
            conn->shutdown();
        }
    }
    
    void MessageCallback(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer*buffer,muduo::Timestamp time)
    {
        char* begin = (char *)buffer->peek();
        std::string str1(begin, buffer->readableBytes());
        int pos = str1.find("\r\n");
        if(pos != -1)
        {
            std::string str2 = str1.substr(0, pos);
            std::cout << str2 << std::endl;
            parseRequestLine(str2.c_str(), conn);
            buffer->retrieve(pos+2);
            conn->shutdown();
        }
    }
    muduo::net::TcpServer _tcpServer;
    muduo::net::EventLoop* _loop;
};
