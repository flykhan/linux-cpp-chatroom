#include "server.h"

int main()
{
    server serv(8023, "127.0.0.1"); // 创建实例，传入端口号和 ip 作为构造函数参数
    serv.run();                     // 启动服务
}