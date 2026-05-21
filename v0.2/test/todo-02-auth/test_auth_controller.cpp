#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "../../backend/src/server.h"
#include "../../backend/src/router.h"
#include "../../backend/src/middleware/session.h"
#include "../../backend/src/db/connection.h"
#include "../../backend/include/httplib.h"
#include "../../backend/include/json.hpp"

using json = nlohmann::json;

class AuthControllerTest : public ::testing::Test {
protected:
    Server server;
    std::thread serverThread;
    httplib::Client* client;

    static void SetUpTestSuite() {
        DbConfig cfg;
        cfg.host     = "127.0.0.1";
        cfg.port     = 3306;
        cfg.user     = "oj";
        cfg.password = "oj_password";
        cfg.database = "oj";
        DbConnection::init(cfg);
        SessionMiddleware::init(24);
    }

    static void TearDownTestSuite() {
        DbConnection::execute("DELETE FROM users WHERE username LIKE 'testctrl_%'");
        DbConnection::close();
    }

    void SetUp() override {
        Router router(server);
        router.setupRoutes();

        serverThread = std::thread([this]() {
            server.listen("127.0.0.1", 19090);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        client = new httplib::Client("http://127.0.0.1:19090");
    }

    void TearDown() override {
        server.stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
        delete client;
    }

    std::string registerUser(const std::string& username, const std::string& password) {
        json j;
        j["username"] = username;
        j["password"] = password;
        auto res = client->Post("/api/auth/register", j.dump(), "application/json");
        if (!res || res->status != 201) return "";
        auto body = json::parse(res->body);
        return body.value("session_id", "");
    }
};

TEST_F(AuthControllerTest, Register_ValidCredentials_Returns201) {
    json j;
    j["username"] = "testctrl_valid";
    j["password"] = "pass123";
    auto res = client->Post("/api/auth/register", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 201);

    auto body = json::parse(res->body);
    EXPECT_EQ(body["username"], "testctrl_valid");
    EXPECT_EQ(body["role"], "user");
    EXPECT_TRUE(body.contains("session_id"));
    EXPECT_FALSE(body.contains("password"));
}

TEST_F(AuthControllerTest, Register_MissingFields_Returns400) {
    json j;
    j["username"] = "testctrl_missing";
    auto res = client->Post("/api/auth/register", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(AuthControllerTest, Register_EmptyFields_Returns400) {
    json j;
    j["username"] = "";
    j["password"] = "";
    auto res = client->Post("/api/auth/register", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(AuthControllerTest, Register_ShortPassword_Returns400) {
    json j;
    j["username"] = "testctrl_shortpw";
    j["password"] = "ab";
    auto res = client->Post("/api/auth/register", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(AuthControllerTest, Register_ShortUsername_Returns400) {
    json j;
    j["username"] = "ab";
    j["password"] = "password123";
    auto res = client->Post("/api/auth/register", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(AuthControllerTest, Register_SetsSessionCookie) {
    json j;
    j["username"] = "testctrl_cookie";
    j["password"] = "pass123";
    auto res = client->Post("/api/auth/register", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 201);

    std::string cookie = res->get_header_value("Set-Cookie");
    EXPECT_NE(cookie.find("session_id="), std::string::npos);
    EXPECT_NE(cookie.find("HttpOnly"), std::string::npos);
}

TEST_F(AuthControllerTest, Register_DuplicateUsername_Returns409) {
    registerUser("testctrl_dup", "pass123");

    json j;
    j["username"] = "testctrl_dup";
    j["password"] = "otherpass";
    auto res = client->Post("/api/auth/register", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 409);
}

TEST_F(AuthControllerTest, Login_ValidCredentials_Returns200) {
    registerUser("testctrl_loginok", "secret");

    json j;
    j["username"] = "testctrl_loginok";
    j["password"] = "secret";
    auto res = client->Post("/api/auth/login", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto body = json::parse(res->body);
    EXPECT_EQ(body["username"], "testctrl_loginok");
    EXPECT_TRUE(body.contains("session_id"));
    EXPECT_FALSE(body.contains("password"));
}

TEST_F(AuthControllerTest, Login_WrongPassword_Returns401) {
    registerUser("testctrl_loginbad", "secret");

    json j;
    j["username"] = "testctrl_loginbad";
    j["password"] = "wrongpass";
    auto res = client->Post("/api/auth/login", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 401);
}

TEST_F(AuthControllerTest, Login_NonexistentUser_Returns401) {
    json j;
    j["username"] = "testctrl_nobody";
    j["password"] = "pass";
    auto res = client->Post("/api/auth/login", j.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 401);
}

TEST_F(AuthControllerTest, Login_SetsSessionCookie) {
    registerUser("testctrl_logincookie", "secret");

    httplib::Headers headers;
    {
        json j;
        j["username"] = "testctrl_logincookie";
        j["password"] = "secret";
        auto res = client->Post("/api/auth/login", j.dump(), "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 200);

        std::string cookie = res->get_header_value("Set-Cookie");
        EXPECT_NE(cookie.find("session_id="), std::string::npos);
    }
}

TEST_F(AuthControllerTest, Me_WithValidCookie_ReturnsUserInfo) {
    json j;
    j["username"] = "testctrl_meok";
    j["password"] = "secret";
    auto regRes = client->Post("/api/auth/register", j.dump(), "application/json");
    ASSERT_NE(regRes, nullptr);
    ASSERT_EQ(regRes->status, 201);

    std::string cookie = regRes->get_header_value("Set-Cookie");

    httplib::Headers cookieHeaders = {{"Cookie", cookie}};
    auto meRes = client->Get("/api/auth/me", cookieHeaders);
    ASSERT_NE(meRes, nullptr);
    EXPECT_EQ(meRes->status, 200);

    auto body = json::parse(meRes->body);
    EXPECT_EQ(body["username"], "testctrl_meok");
    EXPECT_EQ(body["role"], "user");
    EXPECT_TRUE(body.contains("id"));
}

TEST_F(AuthControllerTest, Me_WithoutCookie_Returns401) {
    auto res = client->Get("/api/auth/me");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 401);
}

TEST_F(AuthControllerTest, Me_WithInvalidCookie_Returns401) {
    httplib::Headers headers = {{"Cookie", "session_id=invalid_session_id"}};
    auto res = client->Get("/api/auth/me", headers);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 401);
}

TEST_F(AuthControllerTest, Logout_WithValidCookie_Returns200) {
    json j;
    j["username"] = "testctrl_logout";
    j["password"] = "secret";
    auto regRes = client->Post("/api/auth/register", j.dump(), "application/json");
    ASSERT_NE(regRes, nullptr);
    std::string cookie = regRes->get_header_value("Set-Cookie");

    httplib::Headers cookieHeaders = {{"Cookie", cookie}};
    auto logoutRes = client->Post("/api/auth/logout", cookieHeaders, "", "text/plain");
    ASSERT_NE(logoutRes, nullptr);
    EXPECT_EQ(logoutRes->status, 200);
}

TEST_F(AuthControllerTest, Me_AfterLogout_Returns401) {
    json j;
    j["username"] = "testctrl_logoutme";
    j["password"] = "secret";
    auto regRes = client->Post("/api/auth/register", j.dump(), "application/json");
    ASSERT_NE(regRes, nullptr);
    std::string cookie = regRes->get_header_value("Set-Cookie");

    httplib::Headers cookieHeaders = {{"Cookie", cookie}};

    client->Post("/api/auth/logout", cookieHeaders, "", "text/plain");

    auto meRes = client->Get("/api/auth/me", cookieHeaders);
    ASSERT_NE(meRes, nullptr);
    EXPECT_EQ(meRes->status, 401);
}

TEST_F(AuthControllerTest, HealthCheck_AlwaysOk) {
    auto res = client->Get("/api/health");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
}
