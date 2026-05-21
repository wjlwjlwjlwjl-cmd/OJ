#include "auth_controller.h"
#include "../models/user.h"
#include "../middleware/session.h"
#include "../utils/logger.h"
#include <json.hpp>

using json = nlohmann::json;

static json userToJson(const User& user) {
    json j;
    j["id"] = user.id;
    j["username"] = user.username;
    j["role"] = user.role;
    j["created_at"] = user.created_at;
    return j;
}

static void setSessionCookie(httplib::Response& res, const std::string& session_id) {
    std::string cookie = "session_id=" + session_id +
                         "; HttpOnly; Path=/; Max-Age=86400";
    res.set_header("Set-Cookie", cookie);
}

static void clearSessionCookie(httplib::Response& res) {
    std::string cookie = "session_id=; HttpOnly; Path=/; Max-Age=0";
    res.set_header("Set-Cookie", cookie);
}

void AuthController::handleRegister(const httplib::Request& req, httplib::Response& res) {
    json body;
    try {
        body = json::parse(req.body);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
        return;
    }

    std::string username = body.value("username", "");
    std::string password = body.value("password", "");

    if (username.empty() || password.empty()) {
        res.status = 400;
        res.set_content(R"({"error":"Username and password are required"})", "application/json");
        return;
    }

    if (username.size() < 3 || username.size() > 64) {
        res.status = 400;
        res.set_content(R"({"error":"Username must be 3-64 characters"})", "application/json");
        return;
    }

    if (password.size() < 6) {
        res.status = 400;
        res.set_content(R"({"error":"Password must be at least 6 characters"})", "application/json");
        return;
    }

    auto existing = UserModel::findByUsername(username);
    if (existing.has_value()) {
        res.status = 409;
        res.set_content(R"({"error":"Username already exists"})", "application/json");
        return;
    }

    auto user = UserModel::create(username, password);
    if (!user.has_value()) {
        res.status = 500;
        res.set_content(R"({"error":"Failed to create user"})", "application/json");
        return;
    }

    std::string session_id = SessionMiddleware::createSession(user.value());
    setSessionCookie(res, session_id);

    res.status = 201;
    json j = userToJson(user.value());
    j["session_id"] = session_id;
    res.set_content(j.dump(), "application/json");

    Logger::info("User registered: " + username);
}

void AuthController::handleLogin(const httplib::Request& req, httplib::Response& res) {
    json body;
    try {
        body = json::parse(req.body);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
        return;
    }

    std::string username = body.value("username", "");
    std::string password = body.value("password", "");

    if (username.empty() || password.empty()) {
        res.status = 400;
        res.set_content(R"({"error":"Username and password are required"})", "application/json");
        return;
    }

    auto user = UserModel::findByUsername(username);
    if (!user.has_value()) {
        res.status = 401;
        res.set_content(R"({"error":"Invalid username or password"})", "application/json");
        return;
    }

    if (!UserModel::verifyPassword(password, user.value().password)) {
        res.status = 401;
        res.set_content(R"({"error":"Invalid username or password"})", "application/json");
        return;
    }

    std::string session_id = SessionMiddleware::createSession(user.value());
    setSessionCookie(res, session_id);

    json j = userToJson(user.value());
    j["session_id"] = session_id;
    res.set_content(j.dump(), "application/json");

    Logger::info("User logged in: " + username);
}

void AuthController::handleLogout(const httplib::Request& req, httplib::Response& res) {
    std::string cookie = req.get_header_value("Cookie");
    auto session = SessionMiddleware::authenticate(cookie);
    if (session.has_value()) {
        std::string prefix = "session_id=";
        size_t pos = cookie.find(prefix);
        size_t start = pos + prefix.size();
        size_t end = cookie.find(';', start);
        std::string session_id = cookie.substr(start, end - start);
        SessionMiddleware::destroySession(session_id);
    }

    clearSessionCookie(res);
    res.set_content(R"({"message":"Logged out"})", "application/json");
    Logger::info("User logged out");
}

void AuthController::handleMe(const httplib::Request& req, httplib::Response& res) {
    std::string cookie = req.get_header_value("Cookie");
    auto session = SessionMiddleware::authenticate(cookie);

    if (!session.has_value()) {
        res.status = 401;
        res.set_content(R"({"error":"Not authenticated"})", "application/json");
        return;
    }

    auto user = UserModel::findById(session->user_id);
    if (!user.has_value()) {
        res.status = 401;
        res.set_content(R"({"error":"User not found"})", "application/json");
        return;
    }

    json j = userToJson(user.value());
    res.set_content(j.dump(), "application/json");
}
