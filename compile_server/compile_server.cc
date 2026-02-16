#include "compile_run.hpp"

int main()
{
    init_logger();
    int i = 4;
    while(i--)
    {
        std::cout << "******************test" << i << "******************" << std::endl;
        std::string in_string;
        std::string out_string;
        Json::Value in_json;
        if(i == 3)
            in_json["code"] = "#include <iostream>\n int main()\n{ std::cout << 1 << std::endl;\n return 0;\n}";
        else if(i == 2)
            in_json["code"] = "#inclde <iostream>\n int main()\n{ std::cout << 1 << std::endl;\n return 0;\n}";
        else if(i == 1) 
            in_json["code"] = "#include <iostream>\n struct A\n{\nint a;\n};\nint main()\n{ A* pa = nullptr;\nstd::cout << pa->a << std::endl;\n return 0;\n}";
        else if(i == 0)
            in_json["code"] = "#include <iostream>\n int main()\n{\nstd::cerr << 1 << std::endl;\n return 0;\n}";

        in_json["input"] = "null";
        in_json["cpu_limit"] = 2;
        in_json["mem_limit"] = 1024 * 8;
        Json::StyledWriter writer;
        in_string = writer.write(in_json);
        
        CompileAndRun::compile_and_run(in_string, out_string);
        DEBUG("result: {}", out_string);
    }
    return 0;
}