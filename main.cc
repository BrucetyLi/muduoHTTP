#include <iostream>
#include <httpServer.hpp>

int main(int argc, char* argv[])
{
    if(argc <3)
    {
        std::cout << "./muduohttp port path "<< std::endl; 
        return 0;
    }
    unsigned short port = atoi(argv[1]);
    resource = argv[2];
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1", port);
    httpServer httpserver(&loop,addr,"myHTTPserver");
    httpserver.setThreadNum(4);
    httpserver.start();
    loop.loop();
    return 0;
}