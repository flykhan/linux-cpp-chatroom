#include "server.h"

// 在定义时，sock_arr 被初始化为一个大小为 1000 的容器，并使用 false 作为默认元素值(表示未建立连接)。这意味着，当程序开始运行时，sock_arr 中包含了 1000 个初始值为 false 的元素，可以用于记录当前所有连接套接字的状态。
// 这个静态成员变量的作用可能是用于记录服务器上所有连接套接字的状态。具体来说，当一个新的连接套接字建立时，程序会向 sock_arr 中添加一个新的 bool 元素，表示该套接字处于“已连接”状态。当该套接字关闭时，程序会将相应的 bool 元素设置为 false，从而更新套接字的状态。
vector<bool> server::sock_arr(1000, false);

// 构造函数
server::server(int port, string ip) : server_port(port), server_ip(ip) {}

// 析构函数
server::~server()
{
    for (int i = 0; i < sock_arr.size(); i++) // 找到处于连接状态的客户端套接字并关闭它们
    {
        if (sock_arr[i])
            close(i);
    }
    close(server_sockfd); // 关闭服务器套接字
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
        sock_arr.push_back(conn); // 当一个新的连接套接字建立时，程序会将其添加到 sock_arr 中。这样做可以方便地管理多个连接套接字，并在需要时对它们进行遍历、筛选等操作。
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
            sock_arr[conn] = false; // 将 sock_arr[conn] 的值设置为 false，表示这个套接字已经不再处于连接状态了
            break;
        }

        cout << "收到套接字描述符为" << conn << "发来的信息：" << buffer << endl;

        string str(buffer);       // 将 buffer 转换为 string 类型
        HandleRequest(conn, str); // 处理请求
    }
}

void server::HandleRequest(int conn, string str)
{
    char buffer[1000];
    string name, password; // 用户名和密码
    bool if_login = false; // 记录当前服务对象(当前联机的客户端)是否已经登录
    string login_name;     // 记录当前服务对象(当前联机的客户端)的用户名

    // 连接 MySQL 数据库
    MYSQL *connection = mysql_init(NULL); // 初始化数据库连接变量,返回一个MYSQL*类型的连接句柄,如果返回NULL,说明初始化失败
    mysql_real_connect(connection,
                       "172.20.193.132",
                       "test",
                       "123456",
                       "ChatProject",
                       0,
                       NULL,
                       CLIENT_MULTI_STATEMENTS); // 连接数据库:服务器地址，用户名，密码，数据库名，端口号，套接字，连接标志

    // 注册：name:用户名 password:密码
    if (str.find("name:") != str.npos)
    {
        int p1 = str.find("name:"), p2 = str.find("password:"); // 查找用户名和密码的起始位置
        name = str.substr(p1 + 5, p2 - 5);                      // 提取用户名:从第5个字符开始，长度为p2-5
        password = str.substr(p2 + 9, str.length() - p2 - 9);   // 提取密码:从第9个字符开始，长度为str.length()-p2-9
        string search = "INSERT INTO USER VALUES(\"";           // 定义 sql语句:插入数据:INSERT INTO 表名 VALUES(值1,值2,值3,...);
        search += name;
        search += "\",\"";
        search += password;
        search += "\");";
        cout << "sql 语句为:" << search << endl
             << endl;                            // 两个endl是为了换行
        mysql_query(connection, search.c_str()); // 执行sql语句:成功返回0,失败返回非0,参数1:连接句柄,参数2:sql语句;.c_str()将string转换为char* 类型：为什么要转换为char*类型呢？因为mysql_query()函数的第二个参数是char*类型的，所以要转换一下
    }

    // 登录：name:用户名 password:密码
    else if (str.find("login:") != str.npos) // str.npos 是 string 类的一个静态成员变量，值为 -1，表示查找失败,如果查找成功，返回查找到的第一个字符的位置
    {
        int p1 = str.find("login:"), p2 = str.find("password:"); // 查找用户名和密码的起始位置
        name = str.substr(p1 + 6, p2 - 6);                       // 提取用户名:从第6个字符开始，长度为p2-6
        password = str.substr(p2 + 9, str.length() - p2 - 9);    // 提取密码:从第9个字符开始，长度为str.length()-p2-9
        string search = "SELECT * FROM USER WHERE NAME=\"";      // 定义 sql语句:查询数据:SELECT * FROM 表名 WHERE 条件;
        search += name;
        search += "\";";
        cout << "sql 语句为:" << search << endl;                   // 后台输出 sql 语句
        auto search_res = mysql_query(connection, search.c_str()); // 执行 sql 语句:成功返回 0，失败返回非 0；auto 是 C++11 中的新特性，用于自动推断变量的类型，这里 search_res 的类型为 int
        auto result = mysql_store_result(connection);              // mysql_store_result() 函数用于获取结果集，返回值为 MYSQL_RES 类型的指针，该指针指向查询结果集，如果查询失败，则返回 NULL。
        int column = mysql_num_fields(result);                     // 获取结果集的列数：mysql_num_fields() 函数用于获取结果集中字段的个数
        int row = mysql_num_rows(result);                          // 获取结果集的行数：mysql_num_rows() 函数用于获取结果集中行的个数

        // 如果数据库中存在该用户
        if (search_res == 0 && row != 0)
        {
            cout << "查询成功\n"
                 << endl;
            auto info = mysql_fetch_row(result);                                     // 获取结果集中的一行数据：mysql_fetch_row() 函数用于从结果集中获取下一行，返回值为 MYSQL_ROW 类型的指针，该指针指向结果集中的一行数据，如果没有数据了，则返回 NULL。
            cout << "查询到的用户名为:" << info[0] << " 密码为:" << info[1] << endl; // 后台输出查询到的用户名和密码,info[0]是用户名,info[1]是密码
            // 如果密码正确
            if (info[1] == password)
            {
                cout << "登录密码正确\n\n";
                string str1 = "ok";
                if_login = true;                                // 标记当前服务对象(当前联机的客户端)已经登录
                login_name = name;                              // 记录当前服务对象(当前联机的客户端)的用户名
                send(conn, str1.c_str(), str1.length() + 1, 0); // 发送消息给客户端: +1是为了发送结束符
            }
            // 如果密码错误
            else
            {
                cout << "登录密码错误\n\n";
                char str1[100] = "password_wrong";
                send(conn, str1, strlen(str1), 0); // strlen() 不用 +1：strlen() 函数用于计算字符串的长度，不包括字符串结束符，所以不用 +1
            }
        }
        // 如果数据库不存在该用户
        else
        {
            cout << "查询失败:数据库中不存在该用户\n\n";
            char str1[100] = "not_have_this_user";
            send(conn, str1, strlen(str1), 0); // strlen 和 .length() 都是用来计算字符串长度的，但是 strlen() 是 C 语言中的函数，而 .length() 是 C++ 中的成员函数，所以 .length() 可以用于 string 类型的变量，而 strlen() 不能用于 string 类型的变量
        }
    }
}