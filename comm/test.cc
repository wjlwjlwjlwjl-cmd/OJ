#include <iostream>
#include <unistd.h>
#include <sys/types.h>
int main()
{
    execlp("g++", "g++","test.cc", "-o", "test.exe", "--std=c++11", nullptr);
    return 0;
}
