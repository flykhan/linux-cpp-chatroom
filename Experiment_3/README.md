## 多线程客户端      

我们前面几次实验所使用的客户端都是单线程的，只具有发送功能，但一个具备聊天功能的客户端应该同时具备发送和接收信息的功能，因此我们需要使用多线程来实现客户端，一个线程可以接收消息并打印，另一个线程可以输入信息并发送。

#### 知识点

- 多线程客户端设计
- TCP 套接字网络编程
- C++ 11 的 thread 库使用
- 面向对象程序设计思想

#### 具体要求

把前面实验的客户端升级为多线程客户端（一个线程用于接收并打印信息、一个线程用于输入并发送信息），为前面实验的多线程服务器添加自动回复客户端的代码，用一个终端运行服务器程序，多个终端运行客户端程序，多个客户端都能发送信息送达服务器并收到服务器的应答，并将应答打印到客户端终端上，当用户在客户端输入 exit 时，要结束两个线程之后再结束客户端进程。

1. 编写一个客户端类 client ，有发送线程和接收线程，可以同时发送消息和接收消息。
2. 要编写多个源代码文件：client 头文件给出 client 类声明、`client.cpp` 给出类方法具体实现、`test_client.cpp` 中编写主函数创建 client 实例对象并测试。
3. 当用户在客户端输入 exit 时，要结束发送线程和接收线程之后才退出主线程。
4. 服务器程序要在实验 3 的基础上进行一定修改，能够回复消息。
5. 编写 Makefile 进行自动编译，使用 git 管理版本。

#### 设计思路

客户端应先 connect  服务器建立连接，成功连接之后就创建发送线程和接收线程，与服务器类的设计同理，我们需要将发送线程和接收线程的函数设为静态成员函数，发送线程和接收线程中都使用 while(1) 的循环结构，循环终止的条件是用户输入了 exit 或者对端关闭了连接。

#### 在 global.h 中写头文件

考虑到 client 类和 server 类会用到许多相同的头文件，因此我们没必要每次都重新写各种头文件，我们可以编写一个 `global.h`，在里面写上所有我们需要的头文件（甚至全局变量），让 `server.h` 和 `client.h` 都引入这个 `global.h` 即可，这样通过 `global.h` 就可以包含所有头文件，没那么容易乱。

`global.h` 文件内容如下所示：

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
using namespace std;

#endif
```

#### 具体实现

首先在 `client.h` 头文件中给出 client 类的成员变量和成员函数声明，该类有三个成员变量，同时有构造函数、析构函数、run 函数、发送线程函数、接收线程函数：

```cpp
#ifndef CLIENT_H
#define CLIENT_H

#include "global.h"

class client{
    private:
        int server_port;//服务器端口
        string server_ip;//服务器ip
        int sock;//与服务器建立连接的套接字描述符
    public:
        client(int port,string ip);
        ~client();
        void run();//启动客户端服务
        static void SendMsg(int conn);//发送线程
        static void RecvMsg(int conn);//接收线程
};
#endif
```

接下来在 `client.cpp` 给出具体的函数定义。构造函数负责初始化服务器 ip 和端口号，析构函数负责关闭套接字描述符：

```cpp
#include "client.h"

client::client(int port,string ip):server_port(port),server_ip(ip){}
client::~client(){
    close(sock);
}
```

run 函数负责建立与服务器的连接并且启动发送线程和接收线程：

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

    //创建发送线程和接收线程
    thread send_t(SendMsg,sock),recv_t(RecvMsg,sock);
    send_t.join();
    cout<<"发送线程已结束\n";
    recv_t.join();
    cout<<"接收线程已结束\n";
    return;
}
```

发送线程负责接收用户的输入并且 send 到服务器端，如果用户输入 exit 或者出现异常时将结束线程：

```cpp
//注意，前面不用加static！
void client::SendMsg(int conn){
    char sendbuf[100];
    while (1)
    {
        memset(sendbuf, 0, sizeof(sendbuf));
        cin>>sendbuf;
        int ret=send(conn, sendbuf, strlen(sendbuf),0); //发送
        //输入exit或者对端关闭时结束
        if(strcmp(sendbuf,"exit")==0||ret<=0)
            break;
    }
}
```

接收线程负责接收服务器发来的消息并且打印到终端，因为我们要保证用户输入 exit  之后结束发送线程和接收线程再退出子线程，发送线程的结束很容易，输入 exit 之后 break 即可，但是接收线程无法得知用户是否输入了  exit，因此我们需要进行以下处理：服务器收到 exit 之后断开与客户端的连接，使得客户端接收线程的 recv 返回值为 0，这时再  break 即可退出接收线程：

```cpp
//注意，前面不用加static！
void client::RecvMsg(int conn){
    //接收缓冲区
    char buffer[1000];
    //不断接收数据
    while(1)
    {
        memset(buffer,0,sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer),0);
        //recv返回值小于等于0，退出
        if(len<=0)
            break;
        cout<<"收到服务器发来的信息："<<buffer<<endl;
    }
}
```

因为我们为了让接收线程正常结束要保证服务器收到 exit 之后就断开与客户端的连接，因此要对之前的服务器代码进行修改，让服务器收到  exit 之后就立马 close 掉套接字描述符，为此我们需要对之前的数据结构进行修改，将 sock_arr 改为  vector<bool> 类型，初始化的时候就为其分配一定大小的空间，并全部置为 false 表示“未打开”。

更改后的 `server.h` 如下：

```cpp
#ifndef SERVER_H
#define SERVER_H

#include "global.h"

class server{
    private:
        int server_port;
        int server_sockfd;
        string server_ip;
        static vector<bool> sock_arr;//改为了静态成员变量，且类型变为vector<bool>
    public:
        server(int port,string ip);
        ~server();
        void run();
        static void RecvMsg(int conn);
};
#endif
```

在 `server.cpp` 中开头加入下面这句代码为 `sock_arr` 完成初始化：

```cpp
vector<bool> server::sock_arr(10000,false);    //将10000个位置都设为false，sock_arr[i]=false表示套接字描述符i未打开（因此不能关闭）
```

当然，具体的大小设为 10000 还是其它数字取决于系统能够打开的文件描述符数量，在 Linux 中我们可以使用 `ulimit -n` 命令来查看和修改文件描述符数量限制。

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304031032851.png)

接下来添加服务器收到 exit 关闭套接字描述符的代码，修改后的 `server::RecvMsg` 如下：

```cpp
//注意，前面不用加static！
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
        //回复客户端
        string ans="收到";
        int ret = send(conn,ans.c_str(),ans.length(),0);
        //服务器收到exit或者异常关闭套接字描述符
        if(ret<=0){
            close(conn);
            sock_arr[conn]=false;
            break;
        }
    }
}
```

最后，我们需要将 `server.cpp` 的析构函数改为如下形式，来关闭仍处于打开状态的套接字描述符：

```cpp
server::~server(){
    for(int i=0;i<sock_arr.size();i++){
        if(sock_arr[i])
            close(i);
    }
    close(server_sockfd);
}
```

至此，对服务器的修改也完成了，我们接下来编写一个 test_client.cpp 文件来测试客户端：

```cpp
#include"client.h"
int main(){
    client clnt(8023,"127.0.0.1");
    clnt.run();
}
```

接下来修改 `makefile`：

```makefile
all: test_server.cpp server.cpp server.h test_client.cpp client.cpp client.h global.h
    g++ -o test_client test_client.cpp client.cpp -lpthread
    g++ -o test_server test_server.cpp server.cpp -lpthread
test_server: test_server.cpp server.cpp server.h global.h
    g++ -o test_server test_server.cpp server.cpp -lpthread
test_client: test_client.cpp client.cpp client.h global.h
    g++ -o test_client test_client.cpp client.cpp -lpthread
clean:
    rm test_server
    rm test_client
```

接下来 `make` 并且进行测试：

```bash
make
./test_server
# 新开一个终端
./test_client
# 再新开一个终端
./test_client
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304031032867.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304031032904.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304031032805.png)

最后上传 git：

```bash
git add .
git commit -m 'client class finished'
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304031032759.jpeg)