## 添加用户注册功能

#### 要求

在之前实验的服务器客户端代码中增添用户注册的功能，在客户端为用户提供注册的选项，并将用户注册的账号、密码发送到服务器端，并保存在服务器的 MySQL 数据库中。

1. 在服务器本地 Mysql 中创建新数据库 ChatProject，库中有一张表叫 USER，该表中有账号 NAME 和密码 PASSWORD 两项属性。
2. 为客户端添加注册功能，让用户注册账号密码，客户端将注册信息发送到服务器端，注意：当用户注册时应输入两次密码，如果密码不一致需要重新输入。
3. 服务器端接收来自客户端的注册信息，将用户的账号、密码写入本地 Mysql 数据库。
4. 要面向对象编程，进行类封装。

#### 具体实现

首先在 MySQL 控制台创建数据库 ChatProject，如下：

```sql
create database ChatProject;
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304081434336.png)

接下来先用 use 命令切换到新建的数据库。

```sql
use ChatProject;
```

然后新建一张表格叫 USER 用来保存账号信息，表中有账号 NAME 和密码 PASSWORD 两项属性，都为 VARCHAR  可变长度字符串类型，且将账号 NAME 设为 PRIMARY KEY  主键，主键不允许重复保证了账号的唯一性，而且主键能自动建立索引加快查询速度。

```sql
CREATE TABLE USER(
    NAME VARCHAR(20) PRIMARY KEY,
    PASSWORD VARCHAR(20)
);
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304081434324.png)

建好表之后我们可以查看当前数据库中所有的表格。

```sql
show tables;
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304081434320.jpeg)

最后输入 `exit` 即可离开 MySQL 控制台回到终端。

创建好了表格之后，我们就可以开始为 Client 类和 Server 类添加注册账号的代码了。

首先修改 `client.h` 头文件，添加一个函数 HandleClient，该函数将在与服务器连接建立之后开始工作，与用户进行交互并处理各项事务。

```cpp
#ifndef CLIENT_H
#define CLIENT_H

#include "global.h"

class client{
    private:
        int server_port;
        string server_ip;
        int sock;
    public:
        client(int port,string ip);
        ~client();
        void run();
        static void SendMsg(int conn);
        static void RecvMsg(int conn);
        void HandleClient(int conn);
};
#endif
```

然后修改 `client.cpp` 文件中的 run 函数，连接完服务器之后就调用 HandleClient，并将文件描述符作为参数传入，如下：

```cpp
void client::run(){

    //定义sockfd
    sock = socket(AF_INET,SOCK_STREAM, 0);

    //定义sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);  //服务器端口
    servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());  //服务器ip

    //连接服务器，成功返回0，错误返回-1
    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    cout<<"连接服务器成功\n";

    HandleClient(sock);

    return;
}
```

接下来设计 `client.cpp` 文件中的 HandleClient  函数，开头先打印一段信息指示用户操作，然后开始处理事务，如果用户输入 2  就进入注册模块，注册时输入两次密码，密码不一致就要重新输入。最后将注册的账号和密码格式化成“name:xxxpass:yyy”（xxx  代表用户名，yyy 代表密码）发送到服务器端，格式化之后可以方便服务器提取出用户名和密码。

```cpp
void client::HandleClient(int conn){
    int choice;
    string name,pass,pass1;
    //bool if_login=false;//记录是否登录成功

    cout<<" ------------------\n";
    cout<<"|                  |\n";
    cout<<"| 请输入你要的选项:|\n";
    cout<<"|    0:退出        |\n";
    cout<<"|    1:登录        |\n";
    cout<<"|    2:注册        |\n";
    cout<<"|                  |\n";
    cout<<" ------------------ \n\n";

    //开始处理各种事务
    while(1){
        //if(if_login)
        //    break;
        cin>>choice;
        if(choice==0)
            break;
        //注册
        else if(choice==2){
            cout<<"注册的用户名:";
            cin>>name;
            while(1){
                cout<<"密码:";
                cin>>pass;
                cout<<"确认密码:";
                cin>>pass1;
                if(pass==pass1)
                    break;
                else
                    cout<<"两次密码不一致!\n\n";
            }
            name="name:"+name;
            pass="pass:"+pass;
            string str=name+pass;
            send(conn,str.c_str(),str.length(),0);
            cout<<"注册成功！\n";
            cout<<"\n继续输入你要的选项:";
        }
    }
}
```

接下来开始设计服务器端，因为服务器要连接 MySQL，所以我们先在 `global.h` 中添加 mysql 头文件，如下：

```cpp
#ifndef _GLOBAL_H
#define _GLOBAL_H

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
#include <mysql/mysql.h>    //新添加
using namespace std;

#endif
```

然后编写 `server.h`，添加静态函数 HandleRequest 的定义。

server.h:

```cpp
#ifndef SERVER_H
#define SERVER_H

#include "global.h"

class server{
    private:
        int server_port;
        int server_sockfd;
        string server_ip;
        static vector<bool> sock_arr;
    public:
        server(int port,string ip);
        ~server();
        void run();
        static void RecvMsg(int conn);
        static void HandleRequest(int conn,string str);
};
#endif
```

修改 `server.cpp` 中 RecvMsg 的代码，让其接收到信息之后调用 HandleRequest 函数，并传入文件描述符和接收到的字符串作为参数。

```cpp
void server::RecvMsg(int conn){
    //接收缓冲区
    char buffer[1000];
    //不断接收数据
    while(1)
    {
        memset(buffer,0,sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer),0);
        //客户端发送exit或者异常结束时，退出
        if(strcmp(buffer,"exit")==0 || len<=0){
            close(conn);
            sock_arr[conn]=false;
            break;
        }
        cout<<"收到套接字描述符为"<<conn<<"发来的信息："<<buffer<<endl;
        string str(buffer);
        HandleRequest(conn,str);
    }
}
```

接下来在 `server.cpp` 中给出 HandleRequest 具体实现，首先连接 MySQL  数据库，然后判断字符串是否含有“name:”，含有就说明是注册信息，通过字符串的 find、substr 函数提取出用户名和密码，然后组合成  SQL 插入语句并通过 mysql_query 执行该 SQL 语句即可。这里我们插入数据用的 SQL 语句为“INSERT INTO USER VALUES ("xxx","yyy");”（xxx 和 yyy 为具体的用户名、密码）。

```cpp
void server::HandleRequest(int conn,string str){
    char buffer[1000];
    string name,pass;
    // bool if_login=false;//记录当前服务对象是否成功登录
    //string login_name;//记录当前服务对象的名字
    //string target_name;//记录发送信息时目标用户的名字
    //int group_num;//记录群号


    //连接MYSQL数据库
    MYSQL *con=mysql_init(NULL);
    mysql_real_connect(con,"127.0.0.1","root","","ChatProject",0,NULL,CLIENT_MULTI_STATEMENTS);

    if(str.find("name:")!=str.npos){
        int p1=str.find("name:"),p2=str.find("pass:");
        name=str.substr(p1+5,p2-5);
        pass=str.substr(p2+5,str.length()-p2-4);
        string search="INSERT INTO USER VALUES (\"";
        search+=name;
        search+="\",\"";
        search+=pass;
        search+="\");";
        cout<<"sql语句:"<<search<<endl<<endl;
        mysql_query(con,search.c_str());
    }
}
```

最后修改 `makefile`，因为服务器要连接 MySQL，所以要加上 `-l` 来动态链接。

```bash
all: test_server.cpp server.cpp server.h test_client.cpp client.cpp client.h global.h
    g++ -o test_client test_client.cpp client.cpp -lpthread
    g++ -o test_server test_server.cpp server.cpp -lpthread -lmysqlclient
test_server: test_server.cpp server.cpp server.h global.h
    g++ -o test_server test_server.cpp server.cpp -lpthread -lmysqlclient
test_client: test_client.cpp client.cpp client.h global.h
    g++ -o test_client test_client.cpp client.cpp -lpthread
clean:
    rm test_server
    rm test_client
```

然后用 `make` 编译即可，接下来进行测试：

```bash
make
./test_server 
# 新开一个终端，执行如下命令
./test_client
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304081434342.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304081434349.png)

然后另开一个终端看看数据库中的 USER 表是否已经有用户数据：

```bash
mysql -u root
> use ChatProject;
> select * from USER;
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304081434326.png)

可见成功注册。

最后提交 git：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304081434410.png)