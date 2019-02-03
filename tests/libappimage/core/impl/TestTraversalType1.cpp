// system
#include <fstream>

// library
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include <appimage/core/exceptions.h>
#include <core/impl/TraversalType1.h>


using namespace appimage::core;
using namespace appimage::core::impl;

class TestTraversalType1 : public ::testing::Test {
protected:
    TraversalType1 traversal;
public:
    TestTraversalType1() : traversal(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage") {}
};

TEST_F(TestTraversalType1, traversal) {
    ASSERT_FALSE(traversal.isCompleted());

    std::map<std::string, PayloadEntryType> expectedEntries = {
        std::make_pair("AppRun", PayloadEntryType::REGULAR),
        std::make_pair("AppImageExtract.desktop", PayloadEntryType::REGULAR),
        std::make_pair(".DirIcon", PayloadEntryType::REGULAR),
        std::make_pair("AppImageExtract.png", PayloadEntryType::LINK),
        std::make_pair("usr/bin/appimageextract", PayloadEntryType::REGULAR),
        std::make_pair("usr/lib/libisoburn.so.1", PayloadEntryType::REGULAR),
        std::make_pair("usr/bin/xorriso", PayloadEntryType::REGULAR),
        std::make_pair("usr/lib/libburn.so.4", PayloadEntryType::REGULAR),
        std::make_pair("usr/lib/libisofs.so.6", PayloadEntryType::REGULAR),
    };

    while (!traversal.isCompleted()) {
        const auto entryName = traversal.getEntryName();
        std::cerr << entryName << std::endl;
        auto itr = expectedEntries.find(entryName);
        ASSERT_NE(itr, expectedEntries.end());
        ASSERT_EQ(itr->second, traversal.getEntryType());

        expectedEntries.erase(entryName);

        ASSERT_NO_THROW(traversal.next());
    }

    ASSERT_TRUE(expectedEntries.empty());
}

TEST_F(TestTraversalType1, extract) {
    auto tmpPath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

    while (!traversal.isCompleted()) {
        if (traversal.getEntryName() == "AppImageExtract.desktop") {
            traversal.extract(tmpPath.string());
            ASSERT_TRUE(boost::filesystem::file_size(tmpPath) > 0);
            break;
        }

        traversal.next();
    }

    boost::filesystem::remove_all(tmpPath);
}

TEST_F(TestTraversalType1, read) {
    std::string expected = "[Desktop Entry]\n"
                           "Name=AppImageExtract\n"
                           "Exec=appimageextract\n"
                           "Icon=AppImageExtract\n"
                           "Terminal=true\n"
                           "Type=Application\n"
                           "Categories=Development;\n"
                           "Comment=Extract AppImage contents, part of AppImageKit\n"
                           "StartupNotify=true\n";

    while (!traversal.isCompleted()) {
        if (traversal.getEntryName() == "AppImageExtract.desktop") {
            std::string content{std::istreambuf_iterator<char>(traversal.read()),
                                std::istreambuf_iterator<char>()};

            ASSERT_EQ(expected, content);

            // Try re-read a given entry
            content = std::string{std::istreambuf_iterator<char>(traversal.read()),
                                  std::istreambuf_iterator<char>()};

            // As entries can be read only once an empty string is expected
            ASSERT_EQ("", content);
            break;
        }

        traversal.next();
    }
}

TEST_F(TestTraversalType1, getEntryLink) {
    auto tmpPath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

    while (!traversal.isCompleted()) {
        if (traversal.getEntryName() == "AppImageExtract.png") {
            ASSERT_EQ(traversal.getEntryLink(), ".DirIcon");
            break;
        }

        traversal.next();
    }

    boost::filesystem::remove_all(tmpPath);
}
