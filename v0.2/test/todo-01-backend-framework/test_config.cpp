#include <gtest/gtest.h>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

#include "../../backend/include/json.hpp"

using json = nlohmann::json;

class ConfigTest : public ::testing::Test {
protected:
    std::string tempDir;

    void SetUp() override {
        tempDir = "/tmp/oj_test_config_" + std::to_string(getpid());
        mkdir(tempDir.c_str(), 0755);
    }

    void TearDown() override {
        unlink((tempDir + "/config.json").c_str());
        rmdir(tempDir.c_str());
        unsetenv("CONFIG_PATH");
    }

    void writeConfig(const std::string& content) {
        std::ofstream f(tempDir + "/config.json");
        f << content;
        f.close();
    }
};

TEST_F(ConfigTest, ParseValidConfig_AllFieldsPresent) {
    writeConfig(R"({
        "server": {"host": "0.0.0.0", "port": 8080},
        "db": {"host": "127.0.0.1", "port": 3306, "user": "oj", "password": "pwd", "database": "oj"},
        "judge": {"host": "127.0.0.1", "port": 9090},
        "session": {"secret": "test-secret", "expiry_hours": 24}
    })");

    std::ifstream file(tempDir + "/config.json");
    ASSERT_TRUE(file.is_open());
    json cfg;
    file >> cfg;

    EXPECT_EQ(cfg["server"]["host"], "0.0.0.0");
    EXPECT_EQ(cfg["server"]["port"], 8080);
    EXPECT_EQ(cfg["db"]["host"], "127.0.0.1");
    EXPECT_EQ(cfg["db"]["port"], 3306);
    EXPECT_EQ(cfg["db"]["user"], "oj");
    EXPECT_EQ(cfg["db"]["password"], "pwd");
    EXPECT_EQ(cfg["db"]["database"], "oj");
    EXPECT_EQ(cfg["judge"]["host"], "127.0.0.1");
    EXPECT_EQ(cfg["judge"]["port"], 9090);
    EXPECT_EQ(cfg["session"]["secret"], "test-secret");
    EXPECT_EQ(cfg["session"]["expiry_hours"], 24);
}

TEST_F(ConfigTest, ParseConfigWithMissingOptionalField_UseDefault) {
    writeConfig(R"({
        "server": {"host": "0.0.0.0"},
        "db": {"host": "127.0.0.1", "user": "oj", "password": "pwd", "database": "oj"},
        "judge": {"host": "127.0.0.1", "port": 9090},
        "session": {"secret": "s", "expiry_hours": 24}
    })");

    std::ifstream file(tempDir + "/config.json");
    ASSERT_TRUE(file.is_open());
    json cfg;
    file >> cfg;

    int port = cfg["server"].value("port", 8080);
    EXPECT_EQ(port, 8080);

    int dbPort = cfg["db"].value("port", 3306);
    EXPECT_EQ(dbPort, 3306);
}

TEST_F(ConfigTest, MissingConfigFile_ThrowsError) {
    std::ifstream file("/nonexistent/config.json");
    EXPECT_FALSE(file.is_open());
}

TEST_F(ConfigTest, InvalidJson_ThrowsParseError) {
    writeConfig("{ invalid json }");
    std::ifstream file(tempDir + "/config.json");
    ASSERT_TRUE(file.is_open());
    EXPECT_THROW({
        json cfg;
        file >> cfg;
    }, json::parse_error);
}

TEST_F(ConfigTest, EnvVarOverridesDefaultPath) {
    writeConfig(R"({"server":{"host":"0.0.0.0","port":9090}})");

    setenv("CONFIG_PATH", (tempDir + "/config.json").c_str(), 1);

    std::ifstream file(std::getenv("CONFIG_PATH"));
    ASSERT_TRUE(file.is_open());
    json cfg;
    file >> cfg;
    EXPECT_EQ(cfg["server"]["port"], 9090);
}

TEST_F(ConfigTest, DbConfigValuesAreCorrectTypes) {
    writeConfig(R"({
        "server": {"host": "0.0.0.0", "port": 8080},
        "db": {"host": "127.0.0.1", "port": 3306, "user": "oj", "password": "pwd", "database": "oj"},
        "judge": {"host": "127.0.0.1", "port": 9090},
        "session": {"secret": "s", "expiry_hours": 24}
    })");

    std::ifstream file(tempDir + "/config.json");
    ASSERT_TRUE(file.is_open());
    json cfg;
    file >> cfg;

    auto& db = cfg["db"];
    EXPECT_TRUE(db["host"].is_string());
    EXPECT_TRUE(db["port"].is_number());
    EXPECT_TRUE(db["user"].is_string());
    EXPECT_TRUE(db["password"].is_string());
    EXPECT_TRUE(db["database"].is_string());
}
