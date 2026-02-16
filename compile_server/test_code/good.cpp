#include <iostream>
#include <sys/resource.h>
#include <sys/time.h>

struct A
{
    int a;
    int b;
};

int main()
{
    std::cout << "this is a file used for test" << std::endl;
    //std::cout << "trying to apply a lot of memory" << std::endl;;
    //int *pa = new int[1024 * 1024 * 1024];
    //A* pa = nullptr;
    //std::cout << pa->b << std::endl;
    std::cerr << "test the cerr" << std::endl;
    return 0;
}
