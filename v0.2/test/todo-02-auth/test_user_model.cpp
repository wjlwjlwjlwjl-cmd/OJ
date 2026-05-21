#include <gtest/gtest.h>
#include <mysql/mysql.h>

#include "../../backend/src/models/user.h"
#include "../../backend/src/db/connection.h"

class UserModelTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        DbConfig cfg;
        cfg.host     = "127.0.0.1";
        cfg.port     = 3306;
        cfg.user     = "oj";
        cfg.password = "oj_password";
        cfg.database = "oj";
        DbConnection::init(cfg);

        DbConnection::execute(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "username VARCHAR(64) NOT NULL UNIQUE,"
            "password VARCHAR(256) NOT NULL,"
            "role ENUM('admin','user') NOT NULL DEFAULT 'user',"
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP"
            ") ENGINE=InnoDB");
    }

    static void TearDownTestSuite() {
        DbConnection::close();
    }

    void TearDown() override {
        DbConnection::execute("DELETE FROM users WHERE username LIKE 'test_%'");
    }
};

TEST_F(UserModelTest, CreateUser_ValidCredentials_ReturnsUser) {
    auto user = UserModel::create("test_valid_user", "password123");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->username, "test_valid_user");
    EXPECT_EQ(user->role, "user");
    EXPECT_GT(user->id, 0);
}

TEST_F(UserModelTest, CreateUser_PasswordHash_ContainsSalt) {
    auto user = UserModel::create("test_salt_user", "mypassword");
    ASSERT_TRUE(user.has_value());

    auto fetched = UserModel::findByUsername("test_salt_user");
    ASSERT_TRUE(fetched.has_value());

    std::string stored = fetched->password;
    size_t sep = stored.find('$');
    EXPECT_NE(sep, std::string::npos);
    EXPECT_EQ(sep, 32);

    std::string salt = stored.substr(0, sep);
    std::string hash = stored.substr(sep + 1);
    EXPECT_EQ(salt.size(), 32);
    EXPECT_EQ(hash.size(), 64);
}

TEST_F(UserModelTest, CreateUser_EachUserGetsDifferentSalt) {
    auto user1 = UserModel::create("test_salt_a", "samepass");
    auto user2 = UserModel::create("test_salt_b", "samepass");
    ASSERT_TRUE(user1.has_value());
    ASSERT_TRUE(user2.has_value());

    auto f1 = UserModel::findByUsername("test_salt_a");
    auto f2 = UserModel::findByUsername("test_salt_b");
    ASSERT_TRUE(f1.has_value());
    ASSERT_TRUE(f2.has_value());

    EXPECT_NE(f1->password, f2->password);
}

TEST_F(UserModelTest, VerifyPassword_CorrectPassword_ReturnsTrue) {
    UserModel::create("test_verify_ok", "secret123");
    auto user = UserModel::findByUsername("test_verify_ok");
    ASSERT_TRUE(user.has_value());
    EXPECT_TRUE(UserModel::verifyPassword("secret123", user->password));
}

TEST_F(UserModelTest, VerifyPassword_WrongPassword_ReturnsFalse) {
    UserModel::create("test_verify_bad", "secret123");
    auto user = UserModel::findByUsername("test_verify_bad");
    ASSERT_TRUE(user.has_value());
    EXPECT_FALSE(UserModel::verifyPassword("wrongpass", user->password));
}

TEST_F(UserModelTest, VerifyPassword_LegacyUnsaltedHash_StillWorks) {
    DbConnection::execute(
        "INSERT INTO users (username, password) VALUES "
        "('test_legacy_user', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8')");

    auto user = UserModel::findByUsername("test_legacy_user");
    ASSERT_TRUE(user.has_value());
    EXPECT_TRUE(UserModel::verifyPassword("password", user->password));
}

TEST_F(UserModelTest, FindById_ExistingUser_ReturnsUser) {
    auto created = UserModel::create("test_find_id", "pass123");
    ASSERT_TRUE(created.has_value());

    auto found = UserModel::findById(created->id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->username, "test_find_id");
}

TEST_F(UserModelTest, FindById_NonExisting_ReturnsNullopt) {
    auto found = UserModel::findById(999999);
    EXPECT_FALSE(found.has_value());
}

TEST_F(UserModelTest, FindByUsername_NonExisting_ReturnsNullopt) {
    auto found = UserModel::findByUsername("test_nonexistent_user_xyz");
    EXPECT_FALSE(found.has_value());
}

TEST_F(UserModelTest, CreateUser_DuplicateUsername_ReturnsNullopt) {
    UserModel::create("test_dup_user", "pass123");
    auto duplicate = UserModel::create("test_dup_user", "otherpass");
    EXPECT_FALSE(duplicate.has_value());
}

TEST_F(UserModelTest, CreateUser_AdminRole_Works) {
    auto user = UserModel::create("test_admin_user", "adminpass", "admin");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->role, "admin");
}

TEST_F(UserModelTest, CreateUser_UsernameWithSpecialChars_Works) {
    auto user = UserModel::create("test_user_123", "pass123");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->username, "test_user_123");
}
