#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <json.hpp>

#include "../../backend/src/server.h"
#include "../../backend/src/router.h"
#include "../../backend/src/middleware/session.h"
#include "../../backend/src/db/connection.h"
#include "../../backend/src/models/user.h"
#include "../../backend/src/models/problem.h"
#include "../../backend/src/judge_client.h"
#include "../../backend/src/utils/logger.h"
#include "../../backend/include/httplib.h"
#include "../../backend/include/json.hpp"

using json = nlohmann::json;

static int testUserId = 0;
static int testProblemId = 0;
static std::string sessionCookie;

class SubmissionControllerTest : public ::testing::Test {
protected:
    static Server* server;
    static std::thread* serverThread;
    static int port;

    static void SetUpTestSuite() {
        Logger::init(ERROR);

        DbConfig cfg;
        cfg.host     = "127.0.0.1";
        cfg.port     = 3306;
        cfg.user     = "oj";
        cfg.password = "oj_password";
        cfg.database = "oj";
        DbConnection::init(cfg);

        DbConnection::execute("DELETE FROM submissions");
        DbConnection::execute("DELETE FROM problems WHERE title LIKE 'Test%'");
        DbConnection::execute("DELETE FROM users WHERE username LIKE 'test_%'");

        auto user = UserModel::create("test_sub_ctrl", "pass123");
        ASSERT_TRUE(user.has_value());
        testUserId = user->id;

        auto problem = ProblemModel::create("Test Ctrl Problem",
                                             "Desc", "easy",
                                             1000, 256, "[]", testUserId);
        ASSERT_TRUE(problem.has_value());
        testProblemId = problem->id;

        port = 18090;
        SessionMiddleware::init(24);
        JudgeClient::setSyncMode(true);

        server = new Server();
        Router router(*server);
        router.setupRoutes();

        serverThread = new std::thread([&]() {
            server->listen("127.0.0.1", port);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        httplib::Client client("http://127.0.0.1:" + std::to_string(port));

        json loginBody;
        loginBody["username"] = "test_sub_ctrl";
        loginBody["password"] = "pass123";
        auto loginRes = client.Post("/api/auth/login",
                                     loginBody.dump(), "application/json");
        ASSERT_NE(loginRes, nullptr);
        ASSERT_EQ(loginRes->status, 200);

        auto cookies = loginRes->get_header_value("Set-Cookie");
        ASSERT_FALSE(cookies.empty());
        sessionCookie = cookies;
    }

    static void TearDownTestSuite() {
        if (server) {
            server->stop();
            server = nullptr;
        }
        if (serverThread && serverThread->joinable()) {
            serverThread->join();
            delete serverThread;
            serverThread = nullptr;
        }

        DbConnection::execute("DELETE FROM submissions");
        DbConnection::execute("DELETE FROM problems WHERE title LIKE 'Test%'");
        DbConnection::execute("DELETE FROM users WHERE username LIKE 'test_%'");
        DbConnection::close();
    }

    void TearDown() override {
        DbConnection::execute("DELETE FROM submissions");
    }

    std::string getCookie() const {
        size_t semi = sessionCookie.find(';');
        return sessionCookie.substr(0, semi);
    }
};

Server* SubmissionControllerTest::server = nullptr;
std::thread* SubmissionControllerTest::serverThread = nullptr;
int SubmissionControllerTest::port = 18090;

TEST_F(SubmissionControllerTest, SubmitValid_Returns202) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    json body;
    body["problem_id"] = testProblemId;
    body["code"] = "#include <iostream>\nint main(){std::cout<<\"ok\";return 0;}";
    body["language"] = "cpp";

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());
    auto res = client.Post("/api/submit", headers, body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 202);

    auto j = json::parse(res->body);
    EXPECT_TRUE(j.contains("id"));
    EXPECT_GT(j["id"].get<int>(), 0);
    EXPECT_EQ(j["status"], "pending");
}

TEST_F(SubmissionControllerTest, Submit_NotAuthenticated_Returns401) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    json body;
    body["problem_id"] = testProblemId;
    body["code"] = "int main(){return 0;}";

    auto res = client.Post("/api/submit", body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 401);
}

TEST_F(SubmissionControllerTest, Submit_InvalidProblemId_Returns400) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    json body;
    body["problem_id"] = 0;
    body["code"] = "int main(){return 0;}";

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());
    auto res = client.Post("/api/submit", headers, body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(SubmissionControllerTest, Submit_EmptyCode_Returns400) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    json body;
    body["problem_id"] = testProblemId;
    body["code"] = "";

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());
    auto res = client.Post("/api/submit", headers, body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(SubmissionControllerTest, Submit_NonexistentProblem_Returns404) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    json body;
    body["problem_id"] = 999999;
    body["code"] = "int main(){return 0;}";

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());
    auto res = client.Post("/api/submit", headers, body.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);
}

TEST_F(SubmissionControllerTest, GetSubmission_ReturnsDetails) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    json submitBody;
    submitBody["problem_id"] = testProblemId;
    submitBody["code"] = "int main(){return 0;}";

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());
    auto submitRes = client.Post("/api/submit", headers,
                                  submitBody.dump(), "application/json");
    ASSERT_NE(submitRes, nullptr);
    EXPECT_EQ(submitRes->status, 202);

    auto submitJson = json::parse(submitRes->body);
    int subId = submitJson["id"];

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto getRes = client.Get("/api/submission/" + std::to_string(subId), headers);
    ASSERT_NE(getRes, nullptr);
    EXPECT_EQ(getRes->status, 200);

    auto getJson = json::parse(getRes->body);
    EXPECT_EQ(getJson["id"], subId);
    EXPECT_EQ(getJson["problem_id"], testProblemId);
    EXPECT_TRUE(getJson.contains("status"));
    EXPECT_TRUE(getJson.contains("submitted_at"));
}

TEST_F(SubmissionControllerTest, GetSubmission_NonExistent_Returns404) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());
    auto res = client.Get("/api/submission/999999", headers);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);
}

TEST_F(SubmissionControllerTest, GetSubmission_OtherUser_Returns403) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    json submitBody;
    submitBody["problem_id"] = testProblemId;
    submitBody["code"] = "int main(){return 0;}";

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());
    auto submitRes = client.Post("/api/submit", headers,
                                  submitBody.dump(), "application/json");
    ASSERT_NE(submitRes, nullptr);
    EXPECT_EQ(submitRes->status, 202);

    auto submitJson = json::parse(submitRes->body);
    int subId = submitJson["id"];

    json loginBody;
    loginBody["username"] = "test_sub_ctrl";
    loginBody["password"] = "pass123";
    auto loginRes = client.Post("/api/auth/login",
                                 loginBody.dump(), "application/json");
    ASSERT_NE(loginRes, nullptr);

    httplib::Headers otherHeaders;
    otherHeaders.emplace("Cookie", getCookie());
    auto res = client.Get("/api/submission/" + std::to_string(subId), otherHeaders);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
}

TEST_F(SubmissionControllerTest, ListSubmissions_ByProblem) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());

    for (int i = 0; i < 3; i++) {
        json body;
        body["problem_id"] = testProblemId;
        body["code"] = "int main(){return " + std::to_string(i) + ";}";
        client.Post("/api/submit", headers, body.dump(), "application/json");
    }

    auto res = client.Get("/api/submissions?problem_id="
                           + std::to_string(testProblemId), headers);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto j = json::parse(res->body);
    EXPECT_TRUE(j.contains("submissions"));
    EXPECT_TRUE(j["submissions"].is_array());
    EXPECT_GE((int)j["submissions"].size(), 3);
    EXPECT_TRUE(j.contains("total"));
}

TEST_F(SubmissionControllerTest, ListSubmissions_ByUser) {
    httplib::Client client("http://127.0.0.1:" + std::to_string(port));

    httplib::Headers headers;
    headers.emplace("Cookie", getCookie());

    auto res = client.Get("/api/submissions", headers);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto j = json::parse(res->body);
    EXPECT_TRUE(j.contains("submissions"));
    EXPECT_TRUE(j.contains("total"));
}
