#include "../comm/httplib.h"
#include "compile_run.hpp"

using namespace httplib;

int main()
{
    init_logger();
    Server svr;
    svr.Post("/compile_run", [](const Request& req, Response& resp){
        std::string in_string = req.body;
        if(in_string.empty())
        {
            return;
        }
        std::string out_string;
        CompileAndRun::compile_and_run(in_string, out_string);
        resp.set_content(out_string, "application/json; charset=utf-8");
    });

    svr.set_base_dir(".");

    svr.listen("0.0.0.0", 8080);
    return 0;
}