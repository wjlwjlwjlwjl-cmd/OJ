#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <fstream>

#include "../../judge/src/judge_server.h"
#include "../../judge/include/httplib.h"
#include "../../judge/include/json.hpp"

class IntegrationTest : public ::testing::Test {
protected:
    JudgeServer* server = nullptr;
    std::thread serverThread;
    int port = 19091;

    void SetUp() override {
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

    std::string url() const {
        return "http://127.0.0.1:" + std::to_string(port);
    }

    nlohmann::json judge(const nlohmann::json& body) {
        httplib::Client client(url());
        auto res = client.Post("/judge", body.dump(), "application/json");
        EXPECT_NE(res, nullptr);
        if (!res) return nullptr;
        EXPECT_EQ(res->status, 200);
        return nlohmann::json::parse(res->body);
    }
};

static std::string helloCode() {
    return R"(#include <iostream>
int main() { std::cout << "Hello"; return 0; })";
}

static std::string wrongCode() {
    return R"(#include <iostream>
int main() { std::cout << "World"; return 0; })";
}

static std::string compileErrorCode() {
    return R"(#include <iostream>
int main() { std::cout << "Hello" << std::endl; return 0; )";
}

static std::string infiniteLoopCode() {
    return "int main() { while(true) {} return 0; }";
}

static std::string largeAllocCode() {
    return R"(int main() {
    volatile int* arr = new int[50000000];
    arr[0] = 42;
    return 0;
})";
}

static std::string sumCode() {
    return R"(#include <iostream>
int main() { int a,b; std::cin >> a >> b; std::cout << a+b; return 0; })";
}

static std::string partialPassCode() {
    return R"(#include <iostream>
int main() { int a,b; std::cin >> a >> b;
    if (a == 10 && b == 20) std::cout << 999;
    else std::cout << a+b;
    return 0; })";
}

/* ───────── Full pipeline correctness tests ───────── */

TEST_F(IntegrationTest, FullPipelineAccepted) {
    nlohmann::json body;
    body["code"] = helloCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();
    nlohmann::json tc;
    tc["input"] = "";
    tc["output"] = "Hello";
    body["test_cases"].push_back(tc);

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "accepted");
    EXPECT_TRUE(j["test_results"][0]["passed"].get<bool>());
}

TEST_F(IntegrationTest, FullPipelineWrongAnswer) {
    nlohmann::json body;
    body["code"] = wrongCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();
    nlohmann::json tc;
    tc["input"] = "";
    tc["output"] = "Hello";
    body["test_cases"].push_back(tc);

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "wrong_answer");
    EXPECT_FALSE(j["test_results"][0]["passed"].get<bool>());
}

TEST_F(IntegrationTest, FullPipelineCompileError) {
    nlohmann::json body;
    body["code"] = compileErrorCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "compile_error");
    EXPECT_FALSE(j["message"].empty());
}

TEST_F(IntegrationTest, FullPipelineMultipleTestCases) {
    nlohmann::json body;
    body["code"] = sumCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();

    auto addTc = [&](const std::string& in, const std::string& out) {
        nlohmann::json tc;
        tc["input"] = in;
        tc["output"] = out;
        body["test_cases"].push_back(tc);
    };
    addTc("1 2\n", "3");
    addTc("10 20\n", "30");
    addTc("0 0\n", "0");

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "accepted");
    ASSERT_EQ(j["test_results"].size(), 3);
    EXPECT_TRUE(j["test_results"][0]["passed"].get<bool>());
    EXPECT_TRUE(j["test_results"][1]["passed"].get<bool>());
    EXPECT_TRUE(j["test_results"][2]["passed"].get<bool>());
}

TEST_F(IntegrationTest, FullPipelinePartialPass) {
    nlohmann::json body;
    body["code"] = partialPassCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();

    auto addTc = [&](const std::string& in, const std::string& out) {
        nlohmann::json tc;
        tc["input"] = in;
        tc["output"] = out;
        body["test_cases"].push_back(tc);
    };
    addTc("1 2\n", "3");
    addTc("10 20\n", "30");
    addTc("0 0\n", "0");

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "wrong_answer");
    ASSERT_EQ(j["test_results"].size(), 3);
    EXPECT_TRUE(j["test_results"][0]["passed"].get<bool>());
    EXPECT_FALSE(j["test_results"][1]["passed"].get<bool>());
    EXPECT_TRUE(j["test_results"][2]["passed"].get<bool>());
}

/* ───────── Security: timeout tests ───────── */

TEST_F(IntegrationTest, FullPipelineTimeout) {
    nlohmann::json body;
    body["code"] = infiniteLoopCode();
    body["time_limit_ms"] = 500;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();
    nlohmann::json tc;
    tc["input"] = "";
    tc["output"] = "";
    body["test_cases"].push_back(tc);

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "time_limit");
    ASSERT_EQ(j["test_results"].size(), 1);
    EXPECT_EQ(j["test_results"][0]["status"], "time_limit");
}

TEST_F(IntegrationTest, TimeLimitPrecision) {
    nlohmann::json body;
    body["code"] = infiniteLoopCode();
    body["time_limit_ms"] = 100;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();
    nlohmann::json tc;
    tc["input"] = "";
    tc["output"] = "";
    body["test_cases"].push_back(tc);

    auto start = std::chrono::steady_clock::now();
    auto j = judge(body);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "time_limit");
    EXPECT_LT(elapsed, 5000) << "Should not wait excessively for a 100ms timeout";
}

/* ───────── Security: memory limit test ───────── */

TEST_F(IntegrationTest, FullPipelineMemoryLimit) {
    nlohmann::json body;
    body["code"] = largeAllocCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 50;
    body["test_cases"] = nlohmann::json::array();
    nlohmann::json tc;
    tc["input"] = "";
    tc["output"] = "";
    body["test_cases"].push_back(tc);

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    ASSERT_EQ(j["test_results"].size(), 1);
    EXPECT_TRUE(j["test_results"][0]["status"] == "runtime_error"
                || j["test_results"][0]["status"] == "memory_limit");
}

/* ───────── Input/Output handling ───────── */

TEST_F(IntegrationTest, JudgeServerInputOutput) {
    nlohmann::json body;
    body["code"] = sumCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();
    nlohmann::json tc;
    tc["input"] = "42 58\n";
    tc["output"] = "100";
    body["test_cases"].push_back(tc);

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "accepted");
    EXPECT_EQ(j["test_results"][0]["your_output"], "100");
}

/* ───────── test_cases_str format ───────── */

TEST_F(IntegrationTest, JudgeServerTestCasesStr) {
    nlohmann::json body;
    body["code"] = sumCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 256;

    nlohmann::json cases = nlohmann::json::array();
    {
        nlohmann::json tc;
        tc["input"] = "1 2\n";
        tc["output"] = "3";
        cases.push_back(tc);
    }
    {
        nlohmann::json tc;
        tc["input"] = "4 5\n";
        tc["output"] = "9";
        cases.push_back(tc);
    }
    body["test_cases_str"] = cases.dump();

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "accepted");
    ASSERT_EQ(j["test_results"].size(), 2);
    EXPECT_TRUE(j["test_results"][0]["passed"].get<bool>());
    EXPECT_TRUE(j["test_results"][1]["passed"].get<bool>());
}

/* ───────── Error handling ───────── */

TEST_F(IntegrationTest, JudgeServerMissingCode) {
    httplib::Client client(url());
    nlohmann::json body;
    body["time_limit_ms"] = 1000;

    auto res = client.Post("/judge", body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(IntegrationTest, JudgeServerInvalidJson) {
    httplib::Client client(url());
    auto res = client.Post("/judge", "not valid json", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(IntegrationTest, JudgeServerEmptyCode) {
    nlohmann::json body;
    body["code"] = "";
    body["time_limit_ms"] = 1000;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();

    httplib::Client client(url());
    auto res = client.Post("/judge", body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

/* ───────── Zero limits ───────── */

TEST_F(IntegrationTest, ZeroTimeLimit) {
    nlohmann::json body;
    body["code"] = infiniteLoopCode();
    body["time_limit_ms"] = 0;
    body["memory_limit_mb"] = 256;
    body["test_cases"] = nlohmann::json::array();
    body["test_cases"].push_back({{"input", ""}, {"output", ""}});

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    EXPECT_EQ(j["status"], "time_limit");
}

TEST_F(IntegrationTest, ZeroMemoryLimit) {
    nlohmann::json body;
    body["code"] = largeAllocCode();
    body["time_limit_ms"] = 2000;
    body["memory_limit_mb"] = 0;
    body["test_cases"] = nlohmann::json::array();
    body["test_cases"].push_back({{"input", ""}, {"output", ""}});

    auto j = judge(body);
    ASSERT_NE(j, nullptr);
    ASSERT_EQ(j["test_results"].size(), 1);
    EXPECT_EQ(j["test_results"][0]["status"], "runtime_error");
}
