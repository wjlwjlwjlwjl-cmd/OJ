#include <gtest/gtest.h>
#include <mysql/mysql.h>

#include "../../backend/src/db/connection.h"

class DbTest : public ::testing::Test {
protected:
    DbConfig validConfig;

    void SetUp() override {
        validConfig.host     = "127.0.0.1";
        validConfig.port     = 3306;
        validConfig.user     = "oj";
        validConfig.password = "oj_password";
        validConfig.database = "oj";
    }

    void TearDown() override {
        DbConnection::close();
    }
};

TEST_F(DbTest, InitWithValidConfig_ReturnsTrue) {
    bool ok = DbConnection::init(validConfig);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(DbConnection::isConnected());
}

TEST_F(DbTest, InitWithInvalidPassword_ReturnsFalse) {
    DbConfig bad = validConfig;
    bad.password = "wrong_password";
    bool ok = DbConnection::init(bad);
    EXPECT_FALSE(ok);
}

TEST_F(DbTest, InitWithInvalidHost_ReturnsFalse) {
    DbConfig bad = validConfig;
    bad.host = "192.0.2.1";
    bool ok = DbConnection::init(bad);
    EXPECT_FALSE(ok);
}

TEST_F(DbTest, DoubleInit_Reconnects) {
    ASSERT_TRUE(DbConnection::init(validConfig));
    ASSERT_TRUE(DbConnection::isConnected());

    DbConnection::close();
    EXPECT_FALSE(DbConnection::isConnected());

    ASSERT_TRUE(DbConnection::init(validConfig));
    EXPECT_TRUE(DbConnection::isConnected());
}

TEST_F(DbTest, Execute_ValidSql_ReturnsTrue) {
    ASSERT_TRUE(DbConnection::init(validConfig));
    EXPECT_TRUE(DbConnection::execute("SELECT 1"));
}

TEST_F(DbTest, Execute_InvalidSql_ReturnsFalse) {
    ASSERT_TRUE(DbConnection::init(validConfig));
    EXPECT_FALSE(DbConnection::execute("SELECT * FROM nonexistent_table"));
}

TEST_F(DbTest, Query_ValidSql_ReturnsResult) {
    ASSERT_TRUE(DbConnection::init(validConfig));
    MYSQL_RES* res = DbConnection::query("SELECT 1 as a");
    ASSERT_NE(res, nullptr);

    MYSQL_ROW row = mysql_fetch_row(res);
    ASSERT_NE(row, nullptr);
    EXPECT_STREQ(row[0], "1");

    mysql_free_result(res);
}

TEST_F(DbTest, Query_InvalidSql_ReturnsNull) {
    ASSERT_TRUE(DbConnection::init(validConfig));
    EXPECT_EQ(DbConnection::query("SELECT * FROM nonexistent_table"), nullptr);
}

TEST_F(DbTest, Escape_SingleQuote) {
    ASSERT_TRUE(DbConnection::init(validConfig));
    std::string escaped = DbConnection::escape("O'Brien");
    EXPECT_EQ(escaped, "O\\'Brien");
}

TEST_F(DbTest, Escape_SqlInjection) {
    ASSERT_TRUE(DbConnection::init(validConfig));
    std::string malicious = "foo'; DROP TABLE users; --";
    std::string escaped = DbConnection::escape(malicious);
    EXPECT_NE(escaped.find("\\'"), std::string::npos);
}

TEST_F(DbTest, QueryWithoutInit_ReturnsNull) {
    DbConnection::close();
    EXPECT_EQ(DbConnection::query("SELECT 1"), nullptr);
}

TEST_F(DbTest, ExecuteWithoutInit_ReturnsFalse) {
    DbConnection::close();
    EXPECT_FALSE(DbConnection::execute("SELECT 1"));
}
