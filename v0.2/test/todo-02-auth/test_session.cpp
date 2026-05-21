#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "../../backend/src/middleware/session.h"

class SessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        SessionMiddleware::init(24);
    }
};

TEST_F(SessionTest, CreateSession_ReturnsNonEmptyId) {
    User user;
    user.id = 1;
    user.username = "testuser";
    user.role = "user";

    std::string sid = SessionMiddleware::createSession(user);
    EXPECT_FALSE(sid.empty());
    EXPECT_EQ(sid.size(), 64);
}

TEST_F(SessionTest, CreateSession_IdIsAlphanumeric) {
    User user;
    user.id = 42;
    user.username = "alice";
    user.role = "admin";

    std::string sid = SessionMiddleware::createSession(user);
    for (char c : sid) {
        EXPECT_TRUE(isalnum(c)) << "Non-alphanumeric char in session id: " << c;
    }
}

TEST_F(SessionTest, GetSession_ValidId_ReturnsSession) {
    User user;
    user.id = 7;
    user.username = "bob";
    user.role = "user";

    std::string sid = SessionMiddleware::createSession(user);

    Session retrieved;
    bool found = SessionMiddleware::getSession(sid, retrieved);
    EXPECT_TRUE(found);
    EXPECT_EQ(retrieved.user_id, 7);
    EXPECT_EQ(retrieved.username, "bob");
    EXPECT_EQ(retrieved.role, "user");
}

TEST_F(SessionTest, GetSession_InvalidId_ReturnsFalse) {
    Session retrieved;
    bool found = SessionMiddleware::getSession("nonexistent_session_id", retrieved);
    EXPECT_FALSE(found);
}

TEST_F(SessionTest, DestroySession_RemovesSession) {
    User user;
    user.id = 3;
    user.username = "charlie";
    user.role = "user";

    std::string sid = SessionMiddleware::createSession(user);

    Session before;
    EXPECT_TRUE(SessionMiddleware::getSession(sid, before));

    SessionMiddleware::destroySession(sid);

    Session after;
    EXPECT_FALSE(SessionMiddleware::getSession(sid, after));
}

TEST_F(SessionTest, CreateSession_DifferentUsers_DifferentIds) {
    User u1; u1.id = 1; u1.username = "a"; u1.role = "user";
    User u2; u2.id = 2; u2.username = "b"; u2.role = "user";

    std::string sid1 = SessionMiddleware::createSession(u1);
    std::string sid2 = SessionMiddleware::createSession(u2);

    EXPECT_NE(sid1, sid2);
}

TEST_F(SessionTest, MultipleSessions_AllAccessible) {
    User u1; u1.id = 1; u1.username = "x"; u1.role = "user";
    User u2; u2.id = 2; u2.username = "y"; u2.role = "admin";

    std::string sid1 = SessionMiddleware::createSession(u1);
    std::string sid2 = SessionMiddleware::createSession(u2);

    Session s1, s2;
    EXPECT_TRUE(SessionMiddleware::getSession(sid1, s1));
    EXPECT_TRUE(SessionMiddleware::getSession(sid2, s2));
    EXPECT_EQ(s1.user_id, 1);
    EXPECT_EQ(s2.user_id, 2);
}

TEST_F(SessionTest, SessionExpiry_AfterExpiry_ReturnsFalse) {
    SessionMiddleware::init(0);

    User user;
    user.id = 99;
    user.username = "expiry";
    user.role = "user";

    std::string sid = SessionMiddleware::createSession(user);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    Session retrieved;
    bool found = SessionMiddleware::getSession(sid, retrieved);
    EXPECT_FALSE(found);

    SessionMiddleware::init(24);
}

TEST_F(SessionTest, Authenticate_ValidCookie_ReturnsSession) {
    User user;
    user.id = 5;
    user.username = "dave";
    user.role = "user";

    std::string sid = SessionMiddleware::createSession(user);
    std::string cookie = "session_id=" + sid;

    auto result = SessionMiddleware::authenticate(cookie);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->user_id, 5);
    EXPECT_EQ(result->username, "dave");
}

TEST_F(SessionTest, Authenticate_EmptyCookie_ReturnsNullopt) {
    auto result = SessionMiddleware::authenticate("");
    EXPECT_FALSE(result.has_value());
}

TEST_F(SessionTest, Authenticate_MalformedCookie_ReturnsNullopt) {
    auto result = SessionMiddleware::authenticate("not_a_cookie_format");
    EXPECT_FALSE(result.has_value());
}

TEST_F(SessionTest, Authenticate_CookieWithOtherFields_FindsSessionId) {
    User user;
    user.id = 8;
    user.username = "eve";
    user.role = "user";

    std::string sid = SessionMiddleware::createSession(user);
    std::string cookie = "other=value; session_id=" + sid + "; path=/";

    auto result = SessionMiddleware::authenticate(cookie);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->user_id, 8);
}

TEST_F(SessionTest, Authenticate_DestroyedSession_ReturnsNullopt) {
    User user;
    user.id = 10;
    user.username = "frank";
    user.role = "user";

    std::string sid = SessionMiddleware::createSession(user);
    SessionMiddleware::destroySession(sid);

    std::string cookie = "session_id=" + sid;
    auto result = SessionMiddleware::authenticate(cookie);
    EXPECT_FALSE(result.has_value());
}
