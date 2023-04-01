#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;

void print()
{
    for (int i = 1; i <= 10; i++)
    {
        cout << i << endl;
        sleep(1); // 休眠 1 秒钟
    }
}

int main(){
    thread t1(print),t2(print);
    t1.join();
    t2.join();
    // 也可以使用 detach
    // t1.detach();
    // t1.detach();
}