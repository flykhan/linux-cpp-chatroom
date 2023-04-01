#ifndef SERVER_H
#define SERVER_H
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

class server
{
private:
    int server_port;      // 服务器端口号
    int server_sockfd;    // 设为 listen 状态的套接字描述符
    string server_ip;     // 服务器 ip
    vector<int> sock_arr; // 保存所有套接字描述符
public:
    server(int port, string ip);   // 构造函数
    ~server();                     // 析构函数
    void run();                    // 服务器开始服务
    static void RecvMsg(int conn); // 子线程工作的静态函数：用于在子线程中处理客户端的数据收发，conn 参数是与客户端建立连接后返回的新套接字描述符，用于指定当前处理的客户端。
};

#endif