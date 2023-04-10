#ifndef CLIENT_H
#define CLIENT_H

#include "global.h"

class client
{
private:
    int server_port;  // 服务器端口号
    string server_ip; // 服务器 ip
    int sock;         // 与服务器建立连接的套接字描述符
public:
    client(int port, string ip);
    ~client();
    void run();                    // 启动客户端服务
    static void SendMsg(int conn); // 发送线程, static 修饰的函数是类的静态成员函数，不需要创建对象就可以直接调用:所有对象共享一个函数
    static void RecvMsg(int conn); // 接收线程
    void HandleClient(int conn);   // 处理客户端请求，在与服务器连接建立之后开始工作，与用户进行交互并处理各项事务
};

#endif