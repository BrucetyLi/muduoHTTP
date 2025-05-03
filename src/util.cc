#include "util.h"
#include <dirent.h>
#include <iostream>

std::string resource = ".";
int hexToDec(char c){ //16进制字符转成10进制数
    if(c >= '0' && c<= '9'){
        return c - '0';
    }else if(c >= 'a' && c<='f'){
        return c - 'a' + 10;
    }else if (c >= 'A' && c <= 'F'){
        return c - 'A' + 10;
    }
    return 0;
}

void decodeUtf8(char* to, char* from){
    for(; *from != '\0'; ++to, ++from){
        if(from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])){ // isxdigit判断是不是16进制的字符 0-f
            *to = hexToDec(from[1]) * 16 + hexToDec(from[2]);
            from += 2;
        }else {
            *to = *from;
        }
    }
    *to = *from;
}

const char* getFileType(const char* name){
    const char* dot = strrchr(name, '.'); // 自右向左查找'.'字符，不存在返回NULL
    if(dot == NULL){
        return "text/plain; charset=UTF-8";
    }
    std::string type(dot);
    type = type.substr(1, type.size()-1);
    if(type == "html" || type == "htm"){
        return "text/html; charset=UTF-8";
    }else if(type == "jpg" || type == "jpeg"){
        return "image/jpeg";
    }else if(type == "png"){
        return "image/png";
    }else if(type == "gif"){
        return "image/gif";
    }else if(type == "css"){
        return "text/css";
    }else if(type == "js"){
        return "application/x-javascript";
    }else if(type == "ico"){
        return "image/x-icon";
    }else{
        return "text/plain; charset=UTF-8";
    }
}

int parseRequestLine(const char* line, const muduo::net::TcpConnectionPtr& conn)  //解析请求行
{
    char method[12];
    char path[1024];
    sscanf(line, "%[^ ] %[^ ]", method, path);
    if(strcasecmp(method,"get") != 0)
    {
        return -1;
    }
    decodeUtf8(path,path);

    struct stat st;
    std::string tmp = resource + std::string(path);
    char * file = (char*)tmp.c_str();
    int ret = stat(file, &st);
    if(ret == -1)
    {
        // 文件不存在 回复404
        sendHeadMsg(conn, 404, "Not Found", getFileType(".html"),-1);
        sendFile("./res/404.html",conn);
        return 0;
    }
    
    if(S_ISDIR(st.st_mode))
    {
        // 把目录的内容发送到客户端
        sendHeadMsg(conn, 200, "OK", getFileType(".html"), -1);
        sendDir(file,conn);
    }
    else
    {
        // 把文件的内容发送到客户端
        sendHeadMsg(conn, 200, "OK", getFileType(file), st.st_size);
        sendFile(file,conn);
    }

    return 0;
}

int sendFile(const char* fileName,const muduo::net::TcpConnectionPtr& conn)
{
    int fd = open(fileName, O_RDONLY);
    assert(fd >0);
#if 1
    while(1)
    {
        char buf[1024];
        int len = read(fd, buf,sizeof(buf));
        if(len >0)
        {
            conn->send(buf, len);
        }
        else if(len==0)
        {
            break;
        }
        else
        {
            perror("read");
        }
    }
#else
#endif
    close(fd);
    return 0;
}


int sendHeadMsg(const muduo::net::TcpConnectionPtr& conn, int status, const char* descr, const char* type, int length)
{
    char buf[4096] = {0};
    sprintf(buf, "http/1.1 %d %s\r\n", status, descr);
    sprintf(buf+strlen(buf), "content-type: %s\r\n", type);
    sprintf(buf+strlen(buf), "content-length: %d\r\n\r\n", length);
    conn->send(buf,strlen(buf));
    return 0;
}

int sendDir(const char* fileName,const muduo::net::TcpConnectionPtr& conn)
{
    char buf[4096] = {0};
    sprintf(buf, "<html><head><meta charset=\"UTF-8\"><title>%s</title></head><body><table>", fileName);
    struct dirent** namelist;
    int num = scandir(fileName, &namelist,nullptr,alphasort);
    for(int i = 0; i< num; i++)
    {
        char* name = namelist[i]->d_name; // namelist指向的是一个指针数组
        struct stat st;
        char subPath[1024] = {0};
        sprintf(subPath, "%s/%s", fileName, name);
        stat(subPath, &st);
        if(S_ISDIR(st.st_mode))
        {
            // std::cout << "mulu: " << subPath << std::endl;
            // a标签支持跳转 <a href="">name</a>
            sprintf(buf+strlen(buf),
            "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", 
            name,  name, st.st_size);
        }
        else
        {
            // std::cout << "wenjian: " << subPath << std::endl;
            sprintf(buf + strlen(buf), 
            "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
             name, name, st.st_size);
        }
        conn->send(buf,strlen(buf));
        memset(buf,0,sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    conn->send(buf,strlen(buf));
    free(namelist);
    return 0;
}