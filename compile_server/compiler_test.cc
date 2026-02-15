#include "compiler.hpp"

int main()
{
    init_logger();
    Compiler cp;
    std::string filename = "good";
    if(cp.compile(filename))
    {
        std::cout << "compile success" << std::endl;
    }
    else
    {
        std::cout << "compile error" << std::endl;
    }

    filename = "bad";
    if(cp.compile(filename))
    {
        std::cout << "compile success" << std::endl;
    }
    else
    {
        std::cout << "compile error" << std::endl;
    }
    return 0;
}