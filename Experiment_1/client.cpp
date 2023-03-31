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

using namespace std;

int main()
{
    // 定义客户端 sockfd 套接字描述符
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    // 定义 sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;                     // TCP/IP 协议族
    servaddr.sin_port = htons(8023);                   // 服务器端口
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器 ip

    // connect 同远程服务器建立主动连接，成功时返回0，若连接失败返回－1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    cout << "连接服务器成功！\n";

    char sendbuf[100]; // 发送缓冲数组
    char recvbuf[100]; // 接收缓冲数组
    while (1)
    {
        memset(sendbuf, 0, sizeof(sendbuf));
        cin >> sendbuf;                              // 向发送缓冲数组写入数据
        send(sock_cli, sendbuf, sizeof(sendbuf), 0); // 发送数据
        if (strcmp(sendbuf, "exit") == 0)
            break; // 如果发送字符为exit，则跳出发送循环
    }

    close(sock_cli); // 关闭发送套接字描述符
    return 0;
}