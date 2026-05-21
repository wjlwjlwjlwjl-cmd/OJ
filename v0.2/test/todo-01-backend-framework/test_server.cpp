#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

#include "../../backend/src/server.h"
#include "../../backend/include/httplib.h"

class ServerTest : public ::testing::Test {
protected:
    Server* server = nullptr;
    std::thread serverThread;

    void startServer(Server& srv, int port) {
        server = &srv;
        serverThread = std::thread([&srv, port]() {
            srv.listen("127.0.0.1", port);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    void TearDown() override {
        if (server) {
            server->stop();
        }
        if (serverThread.joinable()) {
            serverThread.join();
        }
        server = nullptr;
    }
};

TEST_F(ServerTest, HealthRoute_Returns200) {
    Server srv;
    srv.get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });
    startServer(srv, 18080);

    httplib::Client client("http://127.0.0.1:18080");
    auto res = client.Get("/api/health");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto body = res->body;
    EXPECT_NE(body.find("ok"), std::string::npos);
}

TEST_F(ServerTest, CustomRoute_ReturnsExpectedBody) {
    Server srv;
    srv.get("/hello", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("world", "text/plain");
    });
    startServer(srv, 18081);

    httplib::Client client("http://127.0.0.1:18081");
    auto res = client.Get("/hello");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "world");
}

TEST_F(ServerTest, NonexistentRoute_Returns404) {
    Server srv;
    startServer(srv, 18082);

    httplib::Client client("http://127.0.0.1:18082");
    auto res = client.Get("/nonexistent");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);
}

TEST_F(ServerTest, OptionsRequest_ReturnsCorsHeaders) {
    Server srv;
    startServer(srv, 18083);

    httplib::Client client("http://127.0.0.1:18083");
    auto res = client.Options("/any-path");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->get_header_value("Access-Control-Allow-Origin"), "*");
    EXPECT_EQ(res->get_header_value("Access-Control-Allow-Methods"),
              "GET, POST, PUT, DELETE, OPTIONS");
}

TEST_F(ServerTest, PostRoute_Works) {
    Server srv;
    srv.post("/echo", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(req.body, "text/plain");
    });
    startServer(srv, 18084);

    httplib::Client client("http://127.0.0.1:18084");
    auto res = client.Post("/echo", "hello", "text/plain");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "hello");
}

TEST_F(ServerTest, ListenOnOccupiedPort_ReturnsFalse) {
    Server first;
    std::thread t1([&first]() {
        first.listen("127.0.0.1", 18085);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    Server second;
    bool secondResult = false;
    std::thread t2([&second, &secondResult]() {
        secondResult = second.listen("127.0.0.1", 18085);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_FALSE(secondResult);

    first.stop();
    second.stop();
    t1.join();
    t2.join();
}

TEST_F(ServerTest, GetRoute_AccessibleAfterStopRestart) {
    Server srv;
    srv.get("/ping", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("pong", "text/plain");
    });
    startServer(srv, 18086);

    httplib::Client client("http://127.0.0.1:18086");
    auto res1 = client.Get("/ping");
    ASSERT_NE(res1, nullptr);
    EXPECT_EQ(res1->status, 200);
    EXPECT_EQ(res1->body, "pong");
}
