// system
#include <ostream>

// libraries
#include <gtest/gtest.h>

// local
#include "utils/Logger.h"

using namespace appimage::utils;

TEST(TestLogger, instance) {
    ASSERT_TRUE(Logger::getInstance() != NULL);
}

TEST(TestLogger, setCallback) {
    auto logger = Logger::getInstance();

    LogLevel levelSet;
    std::string messageSet;
    logger->setCallback([&levelSet, &messageSet](const LogLevel& level, const std::string& message) {
        levelSet = level;
        messageSet = message;
    });

    logger->log(LogLevel::ERROR, "Hello");

    ASSERT_EQ(levelSet, LogLevel::ERROR);
    ASSERT_EQ(messageSet, "Hello");
}
