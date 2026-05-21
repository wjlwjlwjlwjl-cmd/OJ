#ifndef SESSION_MIDDLEWARE_H
#define SESSION_MIDDLEWARE_H

#include <string>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <ctime>

#include "../models/user.h"

struct Session {
    std::string session_id;
    int user_id;
    std::string username;
    std::string role;
    time_t created_at;
    time_t expires_at;
};

class SessionMiddleware {
public:
    static void init(int expiry_hours = 24);
    static std::string createSession(const User& user);
    static bool getSession(const std::string& session_id, Session& session);
    static void destroySession(const std::string& session_id);
    static std::string generateSessionId();

    static std::optional<Session> authenticate(const std::string& cookie);

private:
    static std::unordered_map<std::string, Session> s_sessions;
    static std::mutex s_mutex;
    static int s_expiry_hours;
};

#endif
