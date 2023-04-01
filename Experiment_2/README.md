## 多线程并发服务器

###       进程和线程的基本概念

#### 进程

进程是程序的一次执行过程，是操作系统资源分配的基本单位。

比如我们在上一个实验中用 `./server` 命令运行服务器程序，就会产生一个进程，可以使用 `ps -ef|grep ./server` 命令查看相关进程快照，如下：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304011939279.png)

PID 为 120 即该进程的进程号为 120。

#### 线程

线程是任务调度和执行的基本单位，一个进程中可以有多个线程独立运行。线程没有自己独立的地址空间，会与其它属于同一进程的线程一起共享进程的资源，但是每个线程也会有自己的独立的栈和一组寄存器。在 Linux 当中，线程的实现比较特别，会把线程当做进程来实现，即将线程视为一个与其它进程共享资源的进程。

我们可以使用 ps -T -p 命令来查看一个进程的所有线程，如下（ 359 是进程号）：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304011939325.png)

##### 通俗理解线程进程和协程

![image-20230401194319882](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304011943949.png)

### C++ 11 的 thread 线程库

C++ 11 中提供了专门的线程库，可以很方便地进行调用。

#### 基本使用

需要引入头文件：

```cpp
#include<thread>
```

创建一个新线程来执行 run 函数：

```cpp
thread t(run);  //实例化一个线程对象t，让该线程执行run函数，构造对象后线程就开始执行了
```

假如说 run 函数需要传入参数 a 和 b，我们可以这样构造：

```cpp
thread t(run,a,b);  //实例化一个线程对象t，让该线程执行run函数，传入a和b作为run的参数
```

需要注意的是，传入的函数必须是**全局函数或者静态函数**，不能是类的普通成员函数。

join 函数会阻塞主线程，直到 join 函数的 thread 对象标识的线程执行完毕为止，join 函数使用方法如下：

```cpp
t.join(); //调用join后，主线程会一直阻塞，直到子线程的run函数执行完毕
```

但有时候我们需要主线程在继续完成其它的任务，而不是一直等待子线程结束，这时候我们可以使用 detach 函数。

detach 函数会让子线程变为分离状态，主线程不会再阻塞等待子线程结束，而是让系统在子线程结束时自动回收资源。使用的方法如下：

```cpp
t.detach();
```

#### 代码示例

接下来用一段代码来示范如何进行简单的多线程编程，我们构建两个线程，同时输出 1-10，如下：

新建一个文件名为 `test_thread.cpp`：

```cpp
#include <iostream>
#include <thread>
#include <unistd.h>
using namespace std;
void print(){
    for(int i=1;i<=10;i++){
        cout<<i<<endl;
        sleep(1);   //休眠1秒钟
    }
}
int main(){
    thread t1(print),t2(print);
    t1.join();
    t2.join();
    //也可以使用detach
    //t1.detach();
    //t2.detach();
}
```

然后使用 g++ 进行编译，需要**注意的是这里要用上 `-l` 来链接线程动态库**，如下：

```bash
g++ -o test_thread test_thread.cpp -lpthread
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304011946424.png)

接下来运行可执行文件，如下：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304011946374.png)

### 实战练习：多线程并发服务器

在实验二中，我们的服务器程序是单线程的，只能为单个客户端服务，因此我们需要将其升级为多线程服务器，为每个客户端连接都创建一个线程进行服务。

#### 具体要求

编写两个程序：一个多线程服务器、一个单线程客户端程序（可和上个实验一样），用一个终端运行服务器程序，多个终端运行客户端程序，要让所有客户端发送的信息都能在服务器终端上显示。

1. 编写一个服务器类 server，该类可以创建多个线程为多个客户端服务，接收所有客户端发送的消息并打印出来。
2. 要编写多个源代码文件：`server.h` 头文件给出 server 类声明、`server.cpp` 给出类方法具体实现、`test_server.cpp` 中编写主函数创建 server 实例对象并测试。
3. 客户端程序可继续使用上个实验的，不用做修改。
4. 编写 Makefile 进行自动编译，使用 git 管理版本。

#### 服务器设计思路

因为我们需要为每个客户端创建一个线程进行服务，所以我们要在每次 accept  取出新连接之后都创建一个线程，这个线程只负责服务这个新的连接，因此我们还要将这个连接对应的套接字描述符传入线程函数中。线程函数不断地调用  recv 接收信息并打印，直到收到客户端发来的 “exit” 或者 recv 返回值小于等于 0 为止。

#### 实现过程

首先我们需要编写 `server.h` 头文件，给出类的成员变量和成员函数声明：

```cpp
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
class server{
    private:
        int server_port;//服务器端口号
        int server_sockfd;//设为listen状态的套接字描述符
        string server_ip;//服务器ip
        vector<int> sock_arr;//保存所有套接字描述符
    public:
        server(int port,string ip);//构造函数
        ~server();//析构函数
        void run();//服务器开始服务
        static void RecvMsg(int conn);//子线程工作的静态函数
};

#endif
```

接下来我们需要在 `server.cpp` 文件中给出函数具体的定义：

```cpp
#include "server.h"

//构造函数
server::server(int port,string ip):server_port(port),server_ip(ip){}

//析构函数
server::~server(){
    for(auto conn:sock_arr)
        close(conn);
    close(server_sockfd);
}

//服务器开始服务
void server::run(){
    //定义sockfd
    server_sockfd = socket(AF_INET,SOCK_STREAM, 0);

    //定义sockaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;//TCP/IP协议族
    server_sockaddr.sin_port = htons(server_port);//server_port;//端口号
    server_sockaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());//ip地址，127.0.0.1是环回地址，相当于本机ip

    //bind，成功返回0，出错返回-1
    if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        perror("bind");//输出错误原因
        exit(1);//结束程序
    }

    //listen，成功返回0，出错返回-1
    if(listen(server_sockfd,20) == -1)
    {
        perror("listen");//输出错误原因
        exit(1);//结束程序
    }

    //客户端套接字
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    //不断取出新连接并创建子线程为其服务
    while(1){
        int conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
        if(conn<0)
        {
            perror("connect");//输出错误原因
            exit(1);//结束程序
        }
        cout<<"文件描述符为"<<conn<<"的客户端成功连接\n";
        sock_arr.push_back(conn);
        //创建线程
        thread t(server::RecvMsg,conn);
        t.detach();//置为分离状态，不能用join，join会导致主线程阻塞
    }
}

//子线程工作的静态函数
//注意，前面不用加static，否则会编译报错
void server::RecvMsg(int conn){
    //接收缓冲区
    char buffer[1000];
    //不断接收数据
    while(1)
    {
        memset(buffer,0,sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer),0);
        //客户端发送exit或者异常结束时，退出
        if(strcmp(buffer,"exit")==0 || len<=0)
            break;
        cout<<"收到套接字描述符为"<<conn<<"发来的信息："<<buffer<<endl;
    }
}
```

写好了类之后，我们需要编写主函数构建实例进行测试，于是便有 `test_server.cpp` 文件：

```cpp
#include"server.h"
int main(){
    server serv(8023,"127.0.0.1");//创建实例，传入端口号和ip作为构造函数参数
    serv.run();//启动服务
}
```

接下来我们需要编写 Makefile 进行自动编译。

`makefile` 内容如下：

```makefile
test_server: test_server.cpp server.cpp server.h
    g++ -o test_server test_server.cpp server.cpp -lpthread
clean:
    rm test_server
```

对于 makefile 文件中的内容，如果是直接粘贴的，需要调整一下格式。点击右下角的 “制表符长度:4”，然后选择“将缩进转换为制表符”：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010170.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010162.png)

以后对于粘贴的 makefile 文件都需要执行这个操作，后面的课程中不再赘述。

然后我们输入 make 进行编译：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010391.png)

接下来执行 `test_server`，如下：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010158.png)

然后打开多个新的终端运行客户端的 client 程序，并发送信息，如下：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010072.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010177.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010735.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010791.png)

可以看到通信成功。

最后我们将这次实验的项目文件提交到 git 的版本库，先设定账号标识，可以把下面的 xiaoming 改成自己的信息，如下：

```bash
git config --global user.email "xiaoming@qq.com"
git config --global user.name "xiaoming"
```

然后 git init 初始化仓库，用 git add 添加到暂存区，再 git commit 提交到版本库，如下：

```bash
git init
git add *
git commit -m " server class finished "
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012010849.png)

## 头文件说明

### #ifndef

`#ifndef` 是 C/C++ 的预编译指令之一，用于在编译时判断某个标识符是否已定义。

具体来说，`#ifndef` 表示如果某个标识符没有被定义，则执行代码块。可以与 `#define` 指令结合使用，来为该标识符定义一个值。

以下是一个简单的示例：

```
c++
#ifndef EXAMPLE_H
#define EXAMPLE_H

// 这里放置需要执行的代码块

#endif
```

在上述示例中，`EXAMPLE_H` 是一个用户自定义的标识符，它表示一个头文件的名称。当这个头文件被包含到其他源文件中时，预编译器首先会检查 `EXAMPLE_H` 是否已经被定义。如果没有被定义，则执行 `#define` 操作，将其定义为该头文件的名称；否则，跳过整个代码块。

使用 `ifndef` 可以帮助避免头文件的重复包含，提高程序的编译效率，并且可以防止多次定义同一个变量或函数等问题。

### #include " " 和 #include < > 的探究

在 C++ 中，用于包含头文件的指令是 `#include`。在使用 `#include` 指令时，可以使用尖括号 `< >` 或者双引号 `" "` 来引用头文件。

当使用尖括号 `< >` 时，编译器会在系统默认的头文件目录中查找该头文件；而当使用双引号 `" "` 时，编译器会先在当前源代码文件所在目录中查找该头文件，如果没有找到再到系统默认的头文件目录中查找。

因此，如果要引用系统库中的头文件，应该使用尖括号 `< >`；如果要引用自己编写的头文件，应该使用双引号 `" "`。

需要注意的是，如果在使用 `#include` 指令时使用了错误的符号，可能会导致编译器无法正确查找头文件，从而导致编译失败。因此，在编写代码时要注意使用正确的符号来引用头文件。

## server代码解释

### accept()

![image-20230401223027473](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012230588.png)

### 编译警告信息说明

![image-20230401234038072](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012340170.png)

### makefile 编译链接命令解释

![image-20230401234152788](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012341878.png)

![image-20230401234450436](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304012344501.png)