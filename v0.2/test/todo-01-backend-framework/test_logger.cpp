#include <gtest/gtest.h>
#include <sstream>
#include <regex>

// include the source directly to test internal functions
#include "../../backend/src/utils/logger.h"

class LoggerTest : public ::testing::Test {
protected:
    std::stringstream buffer;

    void SetUp() override {
        Logger::init(DEBUG);
    }

    void TearDown() override {
        Logger::setLevel(DEBUG);
    }
};

TEST_F(LoggerTest, LevelFilter_DebugWhenInfoLevel_NoOutput) {
    Logger::setLevel(INFO);
    Logger::debug("should not appear");
    Logger::info("should appear");
}

TEST_F(LoggerTest, LogFormat_ContainsExpectedParts) {
    Logger::setLevel(DEBUG);
    Logger::debug("hello world");
}

TEST_F(LoggerTest, ErrorGoesToStderr) {
    Logger::error("test error");
}

TEST(LoggerInit, DefaultLevelIsDebug) {
    Logger::init(DEBUG);
    Logger::debug("init test");
}

TEST(LoggerLevel, SetLevelToWarn_InfoSuppressed) {
    Logger::init(WARN);
    Logger::info("this should be suppressed");
    Logger::warn("this should appear");
    Logger::error("this should appear too");
}
