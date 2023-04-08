#include "client.h"

client::client(int port, string ip) : server_port(port), server_ip(ip) {} // 初始化服务器 ip 和端口号
client::~client()
{
    close(sock); // 关闭套接字描述符
}

void client::run() // run 函数负责建立与服务器的连接并且启动发送线程和接收线程
{
    // 定义 sockfd 套接字描述符
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // 定义套接字信息
    struct sockaddr_in servaddr; // 定义套接字地址结构
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);                  // 服务器端口
    servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str()); // 服务器 ip

    // 连接服务器，成功返回 0，错误返回 -1
    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    cout << "连接服务器成功\n";

    HandleClient(sock); // 处理客户端请求

    return;
}

void client::HandleClient(int conn)
{
    int choice;                              // 用户选项
    string name, password, password_confirm; // 用户名，密码，确认密码

    cout << " --------------------\n"; // 功能选择界面
    cout << "|                    |\n";
    cout << "|  请输入您的选择：   |\n";
    cout << "|     0:退出         |\n";
    cout << "|     1:登录         |\n";
    cout << "|     2:注册         |\n";
    cout << "|                    |\n";
    cout << " --------------------\n\n";

    while (1)
    {
        cin >> choice;
        if (choice == 0)
            break;  // 退出
        
        // 注册
        else if(choice ==2){
            cout<<"请输入注册用户名：";
            cin>>name;
            while(1){//循环输入密码，直到两次密码一致
                cout<<"密码：";
                cin>>password;
                cout<<"确认密码：";
                cin>>password_confirm;
                if(password == password_confirm)
                    break;//两次密码一致,跳出循环
                else
                    cout<<"两次密码不一致，请重新输入\n\n"<<endl;
            }
            name="name:"+name;
            password="password:"+password;
            string str=name+password;
            send(conn,str.c_str(),str.length(),0);//发送注册信息:用户名和密码:格式为name:xxxpassword:xxx
            cout<<"注册成功！"<<endl;
            cout<<"\n请继续选择:"<<endl;
        }

    }
}

// 向服务器发送消息
// 注意：前面不用再加static
void client::SendMsg(int conn)
{
    char sendbuf[100]; // 声明一个大小为100的字符数组sendbuf
    while (1)
    {
        memset(sendbuf, 0, sizeof(sendbuf));               // 每次循环中，它使用memset函数将sendbuf清零
        cin >> sendbuf;                                    // 从cin输入流中读取用户输入的信息，将其存放到sendbuf中。
        int ret = send(conn, sendbuf, strlen(sendbuf), 0); // send函数将sendbuf中的内容发送给服务器，其中conn参数表示连接套接字。如果发送成功，send函数返回发送的字节数
        if (strcmp(sendbuf, "exit") == 0 || ret <= 0)      // ret <= 0表示发送失败
            break;
    }
}

// 从服务器接收消息
void client::RecvMsg(int conn)
{
    // 接收缓冲区
    char buffer[1000];
    // 不断接收数据
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer), 0);
        // recv 返回值小于等于 0，退出
        if (len <= 0)
            break;
        cout << "收到服务器发来的信息：" << buffer << endl;
    }
}