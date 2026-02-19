#include <iostream>
#include "../comm/httplib.h"

#include "oj_control.hpp"
#include <mutex>

using namespace httplib;

int main()
{
    init_logger();
    const char* db_user     = "oj_client";
    const char* db_pass     = "123456";
    const char* db_name     = "oj";
    const char* db_host     = "172.17.0.1";
    std::mutex mtx;

    std::thread warmup_thread([=]() {
        try {
            std::shared_ptr<odb::core::database> db(new odb::mysql::database("oj_client", 
            "123456", "oj", "172.17.0.1", 0, 0, "utf8"));
            std::cout << "[预热] MySQL 连接预热成功" << std::endl;
        } catch (...) {
            std::cerr << "[预热] MySQL 暂时不可用（不影响服务启动）" << std::endl;
        }
    });
    warmup_thread.detach();

    Server server;
    server.new_task_queue = []() -> httplib::TaskQueue* {
        return new httplib::ThreadPool(10);
    };
    auto create_db = [=]() {
        std::shared_ptr<odb::core::database> db(new odb::mysql::database("oj_client", 
            "123456", "oj", "172.17.0.1", 0, 0, "utf8"));
        return db;
    };

    server.Get("/all_questions", [create_db](const Request& req, Response& resp){
        auto db = create_db();
        ProblemControl pc(db);
        std::string html;
        pc.GetAllQuestion(html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    server.Get(R"(/question/(\d+))", [create_db](const Request& req, Response& resp){
        auto db = create_db();
        ProblemControl pc(db);
        int num = std::stoi(req.matches[1]);
        std::string html;
        pc.GetOneQuestion(num, html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    server.Post(R"(/judge/(\d+))", [&mtx, create_db](const Request& req, Response& resp){
        std::lock_guard<std::mutex> lock(mtx);
        auto db = create_db();
        ProblemControl pc(db);
        int num = std::stoi(req.matches[1]);
        std::cerr << "judge service: " << num << std::endl;
        std::string html;
        pc.judge(num, req.body, html);
        resp.set_content(html, "application/json; charset=utf-8");
        std::cout << "judge finish!!!!!!!!" << std::endl;
    });

    server.set_base_dir("./wwwroot");

    server.listen("0.0.0.0", 9003, 128);

    return 0;
}