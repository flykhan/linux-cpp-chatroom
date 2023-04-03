#ifndef SERVER_H
#define SERVER_H

#include "global.h"

class server
{
private:
    int server_port;              // 服务器端口号
    int server_sockfd;            // 设为 listen 状态的套接字描述符
    string server_ip;             // 服务器 ip
    static vector<bool> sock_arr; // 当一个新的连接套接字建立时，程序会向sock_arr中添加一个新的bool元素，表示该套接字处于“已连接”状态。当该套接字关闭时，程序会将相应的bool元素设置为false，从而更新套接字的状
public:
    server(int port, string ip);   // 构造函数
    ~server();                     // 析构函数
    void run();                    // 服务器开始服务
    static void RecvMsg(int conn); // 子线程工作的静态函数：用于在子线程中处理客户端的数据收发，conn 参数是与客户端建立连接后返回的新套接字描述符，用于指定当前处理的客户端。
};

#endif