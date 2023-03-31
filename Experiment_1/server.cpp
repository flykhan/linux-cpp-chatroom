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
    // 定义服务端 sockfd (套接字描述符)
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 定义服务端套接字 sockaddr_in (套接字地址结构)
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;                     // TCP/IP 协议族
    server_sockaddr.sin_port = htons(8023);                   // 端口号
    server_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // ip 地址，127.0.0.1 是回环地址，相当于本机 ip

    // bind (功能：为套接字绑定地址)， 绑定成功返回 0 ，绑定出错返回 -1
    if (bind(server_sockfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        perror("bind"); // 输出错误原因
        exit(1);        // 结束程序
    }

    // listen (将一个套接字置为监听模式，准备接收传入连接。用于服务器，指明某个套接字连接是被动的监听状态)
    // 成功返回 0 ，出错返回 -1
    if (listen(server_sockfd, 20) == -1)
    {
        perror("listen"); // 输出错误原因
        exit(1);          // 结束程序
    }

    // 客户端套接字
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    // accept (功能：从已完成连接队列中取出成功建立连接的套接字，返回成功连接的套接字描述符)
    // 成功返回非负描述符，出错返回 -1
    int conn = accept(server_sockfd, (struct sockaddr *)&client_addr, &length);
    if (conn < 0)
    {
        perror("connect"); // 输出错误原因
        exit(1);           // 结束程序
    }
    cout << "客户端成功连接\n";

    // 接收缓冲区:定义为 1000 字节大小，最多存储 999 个字符长度的字符串，最后一个字符要留给'\0'
    char buffer[1000];

    // 不断接收数据
    while (1)
    {
        // memset函数有三个参数：第一个参数是要被初始化的内存区域的起始地址，第二个参数是要设置的值，第三个参数是要初始化的字节数
        memset(buffer, 0, sizeof(buffer));
        // recv (功能：接收数据,返回实际接收的数据长度，出错时返回－1)
        int len = recv(conn, buffer, sizeof(buffer), 0);
        // 客户端发送 exit 或者异常结束时，退出
        if (strcmp(buffer, "exit") == 0 || len <= 0)
            break;
        cout << "收到客户端信息：" << buffer << endl;
    }

    // 套接字通常需要在完成数据传输等操作后及时关闭，以释放系统资源并避免出现一些问题，比如资源耗尽、端口占用等
    // conn和server_sockfd分别表示客户端和服务器端的套接字文件描述符，通过调用close函数来关闭它们
    close(conn);          // 关闭数据接受描述符
    close(server_sockfd); // 关闭套接字
    return 0;
}