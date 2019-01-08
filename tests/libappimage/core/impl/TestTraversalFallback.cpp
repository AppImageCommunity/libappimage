// library
#include <gtest/gtest.h>
#include <fstream>

// local
#include <appimage/core/Exceptions.h>
#include <core/impl/TraversalFallback.h>


using namespace appimage::core::impl;

class TestTraversalFallback : public ::testing::Test {
protected:
    TraversalFallback fallback;
};

TEST_F(TestTraversalFallback, next) {
    ASSERT_NO_THROW(fallback.next());
}

TEST_F(TestTraversalFallback, isCompleted) {
    ASSERT_TRUE(fallback.isCompleted());
}

TEST_F(TestTraversalFallback, getEntryName) {
    ASSERT_EQ(fallback.getEntryName(), std::string());
}

TEST_F(TestTraversalFallback, extract) {
    ASSERT_NO_THROW(fallback.extract(""));
}

TEST_F(TestTraversalFallback, read) {
    auto& stream = fallback.read();
    std::vector<char> content{std::istream_iterator<char>(stream), std::istream_iterator<char>()};
    ASSERT_TRUE(content.empty());
}
