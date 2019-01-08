// system
#include <fstream>

// library
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include <appimage/core/Exceptions.h>
#include <core/impl/TraversalType1.h>


using namespace appimage::core::impl;

class TestTraversalType1 : public ::testing::Test {
protected:
    TraversalType1 traversal;
public:
    TestTraversalType1() : traversal(TEST_DATA_DIR "/AppImageExtract_6-x86_64.AppImage") {}
};

TEST_F(TestTraversalType1, traversal) {
    ASSERT_FALSE(traversal.isCompleted());

    ASSERT_EQ(traversal.getEntryName(), std::string());
    ASSERT_NO_THROW(traversal.next());

    std::set<std::string> expectedEntries = {
        "AppRun",
        "AppRun",
        "AppImageExtract.desktop",
        ".DirIcon",
        "AppImageExtract.png",
        "usr/bin/appimageextract",
        "usr/lib/libisoburn.so.1",
        "usr/bin/xorriso",
        "usr/lib/libburn.so.4",
        "usr/lib/libisofs.so.6"
    };

    while (!traversal.isCompleted()) {
        const auto entryName = traversal.getEntryName();
        std::cerr << entryName << std::endl;

        if (expectedEntries.find(entryName) == expectedEntries.end())
            FAIL();
        else
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
            std::string content{ std::istreambuf_iterator<char>(traversal.read()),
                                 std::istreambuf_iterator<char>() };

            ASSERT_EQ(expected, content);
            break;
        }

        traversal.next();
    }
}
