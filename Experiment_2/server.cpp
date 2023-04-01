#include "server.h"

// 构造函数
server::server(int port, string ip) : server_port(port), server_ip(ip) {}

// 析构函数
server::~server()
{
    for (auto conn : sock_arr)
        close(conn);
    close(server_sockfd);
}

// 服务器开始服务
void server::run()
{
    // 定义 sockfd
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 定义地址信息
    struct sockaddr_in server_sockaddr;                             // 套接字地址结构
    server_sockaddr.sin_family = AF_INET;                           // TCP/IP 协议族
    server_sockaddr.sin_port = htons(server_port);                  // server_port 端口号
    server_sockaddr.sin_addr.s_addr = inet_addr(server_ip.c_str()); // ip 地址，127.0.0.1 是环回地址，相当于本机 ip

    // bind，成功返回 0，出错返回 -1
    if (bind(server_sockfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1) // 服务端套接字绑定 ip 地址和 port 端口号
    {
        perror("bind"); // 输出错误原因
        exit(1);        // 结束程序
    }

    // listen，成功返回 0，出错返回 -1
    if (listen(server_sockfd, 20) == -1)
    {
        perror("listen"); // 输出错误原因
        exit(1);          // 结束程序
    }

    // 客户端套接字
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    // 不断取出新连接并创建子线程为其服务
    while (1)
    {
        int conn = accept(server_sockfd, (struct sockaddr *)&client_addr, &length); // server_sockfd 是服务器端监听套接字的文件描述符，client_addr 是一个结构体指针，存储了客户端的地址信息（IP 地址和端口号），length 是客户端地址结构体的长度。
        if (conn < 0)
        {
            perror("connect"); // 输出错误原因
            exit(1);           // 结束程序
        }

        cout << "文件描述符为" << conn << "的客户端成功连接\n";
        sock_arr.push_back(conn);
        // 创建线程
        thread t(server::RecvMsg, conn);
        t.detach(); // 置为分离状态，不能用 join ，join 会导致主线程阻塞
    }
}

// 定义子线程工作的静态函数：用于在子线程中处理客户端的数据收发，conn 参数是与客户端建立连接后返回的新套接字描述符，用于指定当前处理的客户端。
// 注意：头文件 server.h 中已经定义过静态函数时，在另一个文件或同一个文件中包含该头文件并使用该静态函数时，不需要再次添加 static 关键字。如果重复添加 static 关键字，则会导致编译错误。
void server::RecvMsg(int conn)
{
    // 接收缓冲区
    char buffer[1000];
    // 不断接收数据
    while (1)
    {
        memset(buffer, 0, sizeof(buffer)); // 在网络编程中，由于发送和接收的数据可能会出现不完整的情况，因此需要先将接收缓冲区清空，以避免旧数据对新数据造成干扰。在这段代码中，使用 memset() 函数将 buffer 清空，以确保该缓冲区为空，可以安全地存储新的接收数据。
        int len = recv(conn, buffer, sizeof(buffer), 0);
        // 客户端发送 exit 或者异常结束时，退出
        if (strcmp(buffer, "exit") == 0 || len <= 0)
        {
            cout << "套接字描述符为" << conn << "的客户端断开了连接" << endl;
            break;
        }

        cout << "收到套接字描述符为" << conn << "发来的信息：" << buffer << endl;
    }
}