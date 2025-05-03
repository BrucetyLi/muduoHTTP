#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <muduo/net/TcpConnection.h>

extern std::string resource;
int parseRequestLine(const char* line,const muduo::net::TcpConnectionPtr& conn);
int sendFile(const char* fileName,const muduo::net::TcpConnectionPtr& conn);
// 发送响应头 (状态行+响应头)
int sendHeadMsg(const muduo::net::TcpConnectionPtr& conn, int status, const char* descr, const char* type, int length);
// 根据文件类型设定不同的content-type
const char* getFileType(const std::string& filename);
int sendDir(const char* fileName, const muduo::net::TcpConnectionPtr& conn);
int hexToDec(char c);
void decodeUtf8(char* to, char* from);  // 主要是为了解码http请求行中文 的问题