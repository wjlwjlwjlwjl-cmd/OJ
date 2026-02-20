#include <iostream>
#include "../comm/httplib.h"

using namespace httplib;

int main()
{
    Server server;
    //get all questions
    server.Get("/all_questions", [](const Request& req, Response& resp){
        resp.set_content("get all of the question", "text/plain; charset=utf-8");
    });
    //get the content by the number
    server.Get(R"(/question/(\d+))", [](const Request& req, Response& resp){
        std::string number = req.matches[1];
        resp.set_content("question" + number, "text/plain; charset=utf-8");
    });
    //commit user's code, and using the judge part
    server.Get(R"(/judge/(\d+))", [](const Request& req, Response& resp){
        std::string number = req.matches[1];
        resp.set_content("judge question" + number, "text/plain; charset=utf-8");
    });

    server.set_base_dir("./wwwroot");

    server.listen("0.0.0.0", 8080);

    return 0;
}
