#include <sys/resource.h>
#include <sys/time.h>
#include <iostream>

int main()
{
    //设置cpu运行时间限制
    //struct rlimit r;
    //r.rlim_cur = 1;
    //r.rlim_max = RLIM_INFINITY;
    //setrlimit(RLIMIT_CPU, &r);

    //设置最大内存限制
    struct rlimit r;
    r.rlim_cur = 1024 * 1024 * 40;
    r.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_AS, &r);
    int count = 0;
    while(1)
    {
        int *p = new int[1024 * 1024];
        std::cout << "memory: " << ++count * 4 << std::endl; 
    }
    return 0;
}