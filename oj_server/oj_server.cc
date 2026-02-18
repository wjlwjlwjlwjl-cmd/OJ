#include <iostream>
#include "../comm/httplib.h"

#include "oj_control.hpp"

using namespace httplib;


int main()
{
    init_logger();
    std::shared_ptr<odb::core::database> db(new odb::mysql::database("oj_client", 
    "123456", "oj", "172.17.0.1", 0, 0, "utf8"));
    ProblemControl pc(db);

    Server server;
    //get all questions
    server.Get("/all_questions", [&pc](const Request& req, Response& resp){
        std::string html;
        pc.GetAllQuestion(html);
        DEBUG("{}", html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    //get the content by the number
    server.Get(R"(/question/(\d+))", [&pc](const Request& req, Response& resp){
        std::string number = req.matches[1];
        std::string html;
        pc.GetOneQuestion(stoi(number), html);
        DEBUG("{}", html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    //commit user's code, and using the judge part
    server.Get(R"(/judge/(\d+))", [&pc](const Request& req, Response& resp){
        std::string number = req.matches[1];
        std::string html;
        pc.GetOneQuestion(std::stoi(number), html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    server.set_base_dir("./wwwroot");

    server.listen("0.0.0.0", 9000);

    return 0;
}