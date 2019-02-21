// system
#include <ostream>

// libraries
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
// local
#include "utils/Logger.h"

using namespace appimage::utils;

class TestLogger : public ::testing::Test {

protected:
    std::unique_ptr<std::stringstream> output;
    std::unique_ptr<Logger> logger;
    std::string logPrefix = "TestLogger";

    void SetUp() override {
        output.reset(new std::stringstream());
        logger.reset(new Logger(logPrefix, *output));
    }
};

TEST_F(TestLogger, debug) {
    logger->debug() << "hello" << std::endl;
    ASSERT_EQ(output->str(), logPrefix + ": DEBUG hello\n");
}

TEST_F(TestLogger, debugIgnored) {
    logger->setLoglevel(LogLevel::INFO);

    logger->debug() << "hello" << std::endl;
    ASSERT_EQ(output->str(), "");
}

TEST_F(TestLogger, info) {
    logger->info() << "hello" << std::endl;
    ASSERT_EQ(output->str(), logPrefix + ": INFO hello\n");
}

TEST_F(TestLogger, infoIgnored) {
    logger->setLoglevel(LogLevel::WARNING);

    logger->info() << "hello" << std::endl;
    ASSERT_EQ(output->str(), "");
}

TEST_F(TestLogger, warning) {
    logger->warning() << "hello" << std::endl;
    ASSERT_EQ(output->str(), logPrefix + ": WARNING hello\n");
}

TEST_F(TestLogger, warningIgnored) {
    logger->setLoglevel(LogLevel::ERROR);

    logger->warning() << "hello" << std::endl;
    ASSERT_EQ(output->str(), "");
}

TEST_F(TestLogger, error) {
    logger->error() << "hello" << std::endl;
    ASSERT_EQ(output->str(), logPrefix + ": ERROR hello\n");
}
