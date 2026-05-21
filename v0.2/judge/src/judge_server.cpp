#include "judge_server.h"
#include "runner.h"
#include <json.hpp>
#include <iostream>

using json = nlohmann::json;

JudgeServer::JudgeServer() : m_running(false) {
    setupRoutes();
}

void JudgeServer::setupRoutes() {
    m_server.Post("/judge", [this](const httplib::Request& req, httplib::Response& res) {
        handleJudge(req, res);
    });
}

void JudgeServer::handleJudge(const httplib::Request& req, httplib::Response& res) {
    json body;
    try {
        body = json::parse(req.body);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"status":"error","message":"Invalid JSON"})", "application/json");
        return;
    }

    std::string code = body.value("code", "");
    int time_limit_ms = body.value("time_limit_ms", 1000);
    int memory_limit_mb = body.value("memory_limit_mb", 256);

    if (code.empty()) {
        res.status = 400;
        res.set_content(R"({"status":"error","message":"Code is required"})", "application/json");
        return;
    }

    std::vector<TestCase> test_cases;
    if (body.contains("test_cases") && body["test_cases"].is_array()) {
        for (const auto& tc : body["test_cases"]) {
            TestCase t;
            t.input = tc.value("input", "");
            t.output = tc.value("output", "");
            test_cases.push_back(t);
        }
    }

    if (body.contains("test_cases_str")) {
        try {
            auto parsed = json::parse(body["test_cases_str"].get<std::string>());
            if (parsed.is_array()) {
                for (const auto& tc : parsed) {
                    TestCase t;
                    t.input = tc.value("input", "");
                    t.output = tc.value("output", "");
                    test_cases.push_back(t);
                }
            }
        } catch (...) {}
    }

    JudgeResult result = Runner::judge(code, test_cases, time_limit_ms, memory_limit_mb);

    json j;
    j["status"] = result.status;
    j["message"] = result.message;
    j["test_results"] = json::array();
    for (const auto& tr : result.test_results) {
        json jtr;
        jtr["index"] = tr.index;
        jtr["passed"] = tr.passed;
        jtr["your_output"] = tr.your_output;
        jtr["expected_output"] = tr.expected_output;
        jtr["status"] = tr.status;
        j["test_results"].push_back(jtr);
    }

    res.set_content(j.dump(), "application/json");
}

bool JudgeServer::listen(const std::string& host, int port) {
    std::cout << "[Judge] Server starting on " << host << ":" << port << std::endl;
    m_running = true;
    return m_server.listen(host.c_str(), port);
}

void JudgeServer::stop() {
    if (m_running) {
        m_server.stop();
        m_running = false;
        std::cout << "[Judge] Server stopped" << std::endl;
    }
}
