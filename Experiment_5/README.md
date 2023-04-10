## 添加用户登录功能

#### 要求

在前面一次实验的服务器客户端代码基础上增添用户登录的功能，在客户端为用户提供登录的选项，客户端将登录信息发送到服务器端，并等待服务器告知登录是否成功。服务器接收客户端的登录信息，并在本地 Mysql 数据库查询该登录信息的账号是否存在，以及密码是否匹配，返回查询结果给客户端。

1. 为客户端添加登录功能，让用户输入账号密码，客户端将登录信息发送到服务器端，并等待服务器返回结果。当收到服务器表示登录成功的信息之后，客户端清空终端并输出新的选项（0：退出，1：发起私聊，2：发起群聊）给用户选择。
2. 为服务器添加登录功能，服务器端接收来自客户端的登录信息，并在本地 Mysql 数据库查询该登录信息的账号是否存在，以及密码是否匹配，返回查询结果给客户端。
3. 要面向对象编程，进行类封装。

#### 实现过程

首先修改客户端，当用户在客户端选择“登录”选项后，我们让其输入用户名和密码，然后对其输入的用户名和密码进行格式化，格式化成“loginxxxpass:yyy”的形式（xxx 为具体的用户名，yyy  为具体的密码），然后发送到服务器，服务器会进行相应的处理后返回结果。客户端接收服务器返回的信息，如果为“ok”说明登录成功，否则说明可能是用户名或者密码错误，需要重新输入登录信息。

具体实现的时候我们在之前客户端代码的基础上新增 choice==1 的分支，在其中完成登录的业务代码。如果登录成功，我们把 if_login 置为 true，随后清空终端，并输出新的选项信息。

修改后 `client.cpp` 的 HandleClient 函数:

```cpp
void client::HandleClient(int conn){
    int choice;
    string name,pass,pass1;
    bool if_login=false;//记录是否登录成功
    string login_name;//记录成功登录的用户名

    cout<<" ------------------\n";
    cout<<"|                  |\n";
    cout<<"| 请输入你要的选项:|\n";
    cout<<"|    0:退出        |\n";
    cout<<"|    1:登录        |\n";
    cout<<"|    2:注册        |\n";
    cout<<"|                  |\n";
    cout<<" ------------------ \n\n";

    //开始处理注册、登录事件
    while(1){
        if(if_login)
           break;
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
        //登录
        else if(choice==1&&!if_login){
            while(1){
                cout<<"用户名:";
                cin>>name;
                cout<<"密码:";
                cin>>pass;
                //格式化
                string str="login"+name;
                str+="pass:";
                str+=pass;
                send(sock,str.c_str(),str.length(),0);//发送登录信息
                char buffer[1000];
                memset(buffer,0,sizeof(buffer));
                recv(sock,buffer,sizeof(buffer),0);//接收响应
                string recv_str(buffer);
                //登录成功
                if(recv_str.substr(0,2)=="ok"){
                    if_login=true;
                    login_name=name;
                    cout<<"登录成功\n\n";
                    break;
                }
                //登录失败
                else
                    cout<<"密码或用户名错误！\n\n";
            }
        }
    }
    //登录成功
    if(if_login){
        system("clear");//清空终端d
        cout<<"        欢迎回来,"<<login_name<<endl;
        cout<<" -------------------------------------------\n";
        cout<<"|                                           |\n";
        cout<<"|          请选择你要的选项：               |\n";
        cout<<"|              0:退出                       |\n";
        cout<<"|              1:发起单独聊天               |\n";
        cout<<"|              2:发起群聊                   |\n";
        cout<<"|                                           |\n";
        cout<<" ------------------------------------------- \n\n";
    }
}
```

接下来修改服务器端的代码，当服务器收到客户端发来的字符串之后，首先查找该串中是否包含 `name:` 或者`login`，若含前者则进入注册的分支，若含后者则进入登录的分支，执行相应的业务代码。

当进入到登录的分支时，首先提取出 `login` 后紧跟的用户名，以及提取出 `pass:` 后紧跟的密码。然后构建一个 SQL 查询字符串 `SELECT \* FROM USER WHERE NAME=xxx;`，并执行该 SQL 从数据库中查找出该用户名及其对应的密码，如果返回的行数为 0 说明用户名不存在，直接返回 `wrong` 信息给客户端，如果行数不为 0 则将该密码和用户输入的密码进行比对，若一致则发送 `ok` 给客户端，否则说明密码错误，发送 `wrong` 给客户端。

完整的登录过程如下：

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304092025659.jpeg)

修改后 `server.cpp` 的 HandleRequest 函数:

```cpp
void server::HandleRequest(int conn,string str){
    char buffer[1000];
    string name,pass;
    bool if_login=false;//记录当前服务对象是否成功登录
    string login_name;//记录当前服务对象的名字
    //string target_name;//记录发送信息时目标用户的名字
    //int group_num;//记录群号


    //连接MYSQL数据库
    MYSQL *con=mysql_init(NULL);
    mysql_real_connect(con,"127.0.0.1","root","","ChatProject",0,NULL,CLIENT_MULTI_STATEMENTS);

    //注册
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
    //登录
    else if(str.find("login")!=str.npos){
        int p1=str.find("login"),p2=str.find("pass:");
        name=str.substr(p1+5,p2-5);
        pass=str.substr(p2+5,str.length()-p2-4);
        string search="SELECT * FROM USER WHERE NAME=\"";
        search+=name;
        search+="\";";
        cout<<"sql语句:"<<search<<endl;
        auto search_res=mysql_query(con,search.c_str());
        auto result=mysql_store_result(con);
        int col=mysql_num_fields(result);//获取列数
        int row=mysql_num_rows(result);//获取行数
        //查询到用户名
        if(search_res==0&&row!=0){
            cout<<"查询成功\n";
            auto info=mysql_fetch_row(result);//获取一行的信息
            cout<<"查询到用户名:"<<info[0]<<" 密码:"<<info[1]<<endl;
            //密码正确
            if(info[1]==pass){
                cout<<"登录密码正确\n\n";
                string str1="ok";
                if_login=true;
                login_name=name;//记录下当前登录的用户名
                send(conn,str1.c_str(),str1.length()+1,0);
            }
            //密码错误
            else{
                cout<<"登录密码错误\n\n";
                char str1[100]="wrong";
                send(conn,str1,strlen(str1),0);
            }
        }
        //没找到用户名
        else{
            cout<<"查询失败\n\n";
            char str1[100]="wrong";
            send(conn,str1,strlen(str1),0);
        }
    }
}
```

接下来使用 make 进行编译，然后测试：

```bash
sudo service mysql start
make
./test_server
# 新开一个终端，执行如下命令
./test_client
```

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304092025641.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304092025790.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304092025821.png)

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304092025779.png)

可见没有问题，最后上传 git 即可。

![图片描述](https://typoraflykhan.oss-cn-beijing.aliyuncs.com/202304092025867.png)