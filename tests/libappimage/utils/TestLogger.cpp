// system
#include <ostream>

// libraries
#include <gtest/gtest.h>

// local
#include <appimage/utils/Logger.h>

using namespace appimage::utils;

TEST(TestLogger, instance) {
    ASSERT_TRUE(Logger::getInstance() != NULL);
}

TEST(TestLogger, setFunctions) {
    auto logger = Logger::getInstance();

    LogLevel levelSet;
    std::string messageSet;
    logger->setFunction([&levelSet, &messageSet](const LogLevel& level, const std::string& message) {
        levelSet = level;
        messageSet = message;
    });

    logger->log(LogLevel::ERROR, "Hello");

    ASSERT_EQ(levelSet, LogLevel::ERROR);
    ASSERT_EQ(messageSet, "Hello");
}
