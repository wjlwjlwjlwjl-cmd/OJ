#include <iostream>
#include "Logger.hpp"
int main()
{
    init_logger();
    DEBUG("{}", "hello");

    std::cout << "1";
    return 0;
}
