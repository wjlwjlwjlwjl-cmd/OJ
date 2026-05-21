#include "../comm/httplib.h"
#include "compile_run.hpp"
#include <gflags/gflags.h>

DEFINE_int32(port, 9000, "PORT");

using namespace httplib;

int main(int argc, char* argv[])
{
    init_logger();
    google::ParseCommandLineFlags(&argc,  &argv, true);
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
        std::cout << "******finish the work******" << std::endl;
    });

    svr.set_base_dir(".");

    std::cout << "running on " << FLAGS_port << std::endl;
    svr.listen("0.0.0.0", FLAGS_port);
    return 0;
}