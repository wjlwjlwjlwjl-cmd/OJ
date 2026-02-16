#include "compiler.hpp"
#include "runner.hpp"

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
    Runner rn;
    if(!rn.run(filename, 6, 1024))
    {
        std::cout << "run success" << std::endl;
    }
    else
    {
        std::cout << "run end with error" << std::endl;
    }
    return 0;
}