#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

#include "../../judge/src/judge_server.h"
#include "../../judge/include/httplib.h"
#include "../../judge/include/json.hpp"

class JudgeServerTest : public ::testing::Test {
protected:
    JudgeServer* server = nullptr;
    std::thread serverThread;
    int port = 19090;

    void startServer() {
        server = new JudgeServer();
        serverThread = std::thread([this]() {
            server->listen("127.0.0.1", port);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    void TearDown() override {
        if (server) {
            server->stop();
        }
        if (serverThread.joinable()) {
            serverThread.join();
        }
        delete server;
        server = nullptr;
    }
};

TEST_F(JudgeServerTest, ValidRequestReturns200) {
    ASSERT_NO_FATAL_FAILURE(startServer());

    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    nlohmann::json body;
    body["code"] = R"(#include <iostream>
int main() { std::cout << "OK"; return 0; })";
    body["time_limit_ms"] = 1000;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();
    nlohmann::json tc;
    tc["input"] = "";
    tc["output"] = "OK";
    body["test_cases"].push_back(tc);

    auto res = client.Post("/judge", body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto j = nlohmann::json::parse(res->body);
    EXPECT_EQ(j["status"], "accepted");
    ASSERT_TRUE(j.contains("test_results"));
    ASSERT_TRUE(j["test_results"].is_array());
    ASSERT_EQ(j["test_results"].size(), 1);
    EXPECT_TRUE(j["test_results"][0]["passed"].get<bool>());
}

TEST_F(JudgeServerTest, MissingCodeReturns400) {
    ASSERT_NO_FATAL_FAILURE(startServer());

    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    nlohmann::json body;
    body["time_limit_ms"] = 1000;

    auto res = client.Post("/judge", body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(JudgeServerTest, InvalidJsonReturns400) {
    ASSERT_NO_FATAL_FAILURE(startServer());

    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    auto res = client.Post("/judge", "not valid json", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}
