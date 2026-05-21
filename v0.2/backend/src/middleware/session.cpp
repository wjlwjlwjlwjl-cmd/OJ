#include "session.h"
#include "../utils/logger.h"

#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

std::unordered_map<std::string, Session> SessionMiddleware::s_sessions;
std::mutex SessionMiddleware::s_mutex;
int SessionMiddleware::s_expiry_hours = 24;

void SessionMiddleware::init(int expiry_hours) {
    s_expiry_hours = expiry_hours;
    Logger::info("Session middleware initialized, expiry: " +
                 std::to_string(expiry_hours) + "h");
}

std::string SessionMiddleware::generateSessionId() {
    static const char chars[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);

    std::string id;
    id.reserve(64);
    for (int i = 0; i < 64; ++i) {
        id += chars[dis(gen)];
    }
    return id;
}

std::string SessionMiddleware::createSession(const User& user) {
    std::lock_guard<std::mutex> lock(s_mutex);

    Session session;
    session.session_id = generateSessionId();
    session.user_id = user.id;
    session.username = user.username;
    session.role = user.role;
    session.created_at = std::time(nullptr);
    session.expires_at = session.created_at + s_expiry_hours * 3600;

    s_sessions[session.session_id] = session;

    Logger::info("Session created for user: " + user.username +
                 " (id: " + session.session_id.substr(0, 8) + "...)");
    return session.session_id;
}

bool SessionMiddleware::getSession(const std::string& session_id, Session& session) {
    std::lock_guard<std::mutex> lock(s_mutex);

    auto it = s_sessions.find(session_id);
    if (it == s_sessions.end()) {
        return false;
    }

    if (std::time(nullptr) >= it->second.expires_at) {
        s_sessions.erase(it);
        Logger::debug("Session expired: " + session_id.substr(0, 8) + "...");
        return false;
    }

    session = it->second;
    return true;
}

void SessionMiddleware::destroySession(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(s_mutex);

    auto it = s_sessions.find(session_id);
    if (it != s_sessions.end()) {
        Logger::info("Session destroyed: " + session_id.substr(0, 8) + "...");
        s_sessions.erase(it);
    }
}

std::optional<Session> SessionMiddleware::authenticate(const std::string& cookie) {
    if (cookie.empty()) return std::nullopt;

    std::string prefix = "session_id=";
    size_t pos = cookie.find(prefix);
    if (pos == std::string::npos) return std::nullopt;

    size_t start = pos + prefix.size();
    size_t end = cookie.find(';', start);
    std::string session_id = cookie.substr(start, end - start);

    Session session;
    if (!getSession(session_id, session)) return std::nullopt;
    return session;
}
