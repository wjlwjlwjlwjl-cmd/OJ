#include <iostream>
#include "../comm/httplib.h"

#include "oj_control.hpp"

using namespace httplib;

int main()
{
    init_logger();
    std::mutex mtx;

    Server server;
    //get all questions
    server.Get("/all_questions", [](const Request& req, Response& resp){
        std::shared_ptr<odb::core::database> db(new odb::mysql::database("oj_client", 
        "123456", "oj", "172.17.0.1", 0, 0, "utf8"));
        ProblemControl pc(db);
        std::string html;
        pc.GetAllQuestion(html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    //get the content by the number
    server.Get(R"(/question/(\d+))", [](const Request& req, Response& resp){
        std::shared_ptr<odb::core::database> db(new odb::mysql::database("oj_client", 
        "123456", "oj", "172.17.0.1", 0, 0, "utf8"));
        ProblemControl pc(db);
        std::string number = req.matches[1];
        std::string html;
        pc.GetOneQuestion(stoi(number), html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    //commit user's code, and using the judge part
    server.Post(R"(/judge/(\d+))", [&mtx](const Request& req, Response& resp){
        mtx.lock();
        std::shared_ptr<odb::core::database> db(new odb::mysql::database("oj_client", 
        "123456", "oj", "172.17.0.1", 0, 0, "utf8"));
        ProblemControl pc(db);
        std::string number = req.matches[1];
        std::cerr << "judge service: " << number << std::endl;
        std::string html;
        pc.judge(std::stoi(number), req.body, html);
        resp.set_content(html, "application/json; charset=utf-8");
        std::cout << "judge finish!!!!!!!!" << std::endl;
        mtx.unlock();
    });

    server.set_base_dir("./wwwroot");

    httplib::ThreadPool pool(5);
    auto ret = server.new_task_queue();
    server.listen("0.0.0.0", 9003);

    return 0;
}