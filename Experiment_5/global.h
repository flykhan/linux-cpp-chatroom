#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <sys/types.h>   //基本系统数据类型
#include <sys/socket.h>  //socket:套接字函数和结构,bind,listen,accept,connect,send,recv,sendto,recvfrom,shutdown
#include <stdio.h>       //printf:标准输入输出函数
#include <netinet/in.h>  //sockaddr_in:套接字地址结构,AF_INET:地址族,htons:主机字节顺序转网络字节顺序,INADDR_ANY:任意地址
#include <arpa/inet.h>   //inet_addr:将点分十进制ip地址转换为网络字节顺序的二进制ip地址
#include <unistd.h>      //close:关闭文件描述符
#include <string.h>      //bzero:将字符串清零
#include <stdlib.h>      //exit(0);:正常退出
#include <fcntl.h>       //文件控制定义:open,read,write,close
#include <sys/shm.h>     //共享内存:shmget,shmat,shmdt,shmctl
#include <iostream>      //cout cin:标准输入输出
#include <thread>        //thread 头文件:用于创建线程
#include <vector>        //vector容器:动态数组,可以动态增加和删除元素
#include <mysql/mysql.h> // mysql 头文件, 需要安装 mysql-devel:用于连接 mysql 数据库

using namespace std; // 使用标准命名空间

#endif