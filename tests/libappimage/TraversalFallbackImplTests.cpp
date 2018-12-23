// library
#include <gtest/gtest.h>
#include <fstream>

// local
#include <appimage/core/Exceptions.h>
#include <core/impl/TraversalFallback.h>


using namespace appimage::core::impl;

class TraversalFallbackImplTests : public ::testing::Test {
protected:
    TraversalFallback fallback;
};

TEST_F(TraversalFallbackImplTests, next) {
    ASSERT_NO_THROW(fallback.next());
}

TEST_F(TraversalFallbackImplTests, isCompleted) {
    ASSERT_TRUE(fallback.isCompleted());
}

TEST_F(TraversalFallbackImplTests, getEntryName) {
    ASSERT_EQ(fallback.getEntryName(), std::string());
}

TEST_F(TraversalFallbackImplTests, extract) {
    ASSERT_NO_THROW(fallback.extract(""));
}

TEST_F(TraversalFallbackImplTests, read) {
    auto& stream = fallback.read();
    std::vector<char> content{std::istream_iterator<char>(stream), std::istream_iterator<char>()};
    ASSERT_TRUE(content.empty());
}
