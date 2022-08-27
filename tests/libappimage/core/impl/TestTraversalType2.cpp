// system
#include <fstream>

// library
#include <gtest/gtest.h>
#include <filesystem>

// local
#include <appimage/core/exceptions.h>
#include <core/impl/TraversalType2.h>
#include "TemporaryDirectory.h"


using namespace appimage::core;
using namespace appimage::core::impl;

class TestTraversalType2 : public ::testing::Test {
protected:
    TraversalType2 traversal;
public:
    TestTraversalType2() : traversal(TEST_DATA_DIR "/Echo-x86_64.AppImage") {}
};

TEST_F(TestTraversalType2, traversal) {
    ASSERT_FALSE(traversal.isCompleted());

    std::vector<std::pair<std::string, PayloadEntryType>> expectedEntries = {
        std::make_pair(".DirIcon", PayloadEntryType::LINK),
        std::make_pair("AppRun", PayloadEntryType::REGULAR),
        std::make_pair("echo.desktop", PayloadEntryType::LINK),
        std::make_pair("usr", PayloadEntryType::DIR),
        std::make_pair("usr/bin", PayloadEntryType::DIR),
        std::make_pair("usr/bin/echo", PayloadEntryType::REGULAR),
        std::make_pair("usr/bin", PayloadEntryType::DIR),
        std::make_pair("usr/share", PayloadEntryType::DIR),
        std::make_pair("usr/share/applications", PayloadEntryType::DIR),
        std::make_pair("usr/share/applications/echo.desktop", PayloadEntryType::REGULAR),
        std::make_pair("usr/share/applications", PayloadEntryType::DIR),
        std::make_pair("usr/share", PayloadEntryType::DIR),
        std::make_pair("usr", PayloadEntryType::DIR),
        std::make_pair("utilities-terminal.svg", PayloadEntryType::REGULAR),
    };

    while (!traversal.isCompleted()) {
        auto entry = std::make_pair(traversal.getEntryPath(), traversal.getEntryType());

        auto itr = std::find(expectedEntries.begin(), expectedEntries.end(), entry);
        ASSERT_NE(itr, expectedEntries.end());

        expectedEntries.erase(itr);

        ASSERT_NO_THROW(traversal.next());
    }

    ASSERT_TRUE(expectedEntries.empty());
}

TEST_F(TestTraversalType2, extract) {
    const TemporaryDirectory tmpDir;
    const auto tmpFilePath = tmpDir.path() / "tempfile";

    while (!traversal.isCompleted()) {
        // extract symlink
        if (traversal.getEntryPath() == ".DirIcon") {
            traversal.extract(tmpFilePath);

            ASSERT_TRUE(std::filesystem::is_symlink(tmpFilePath));

            auto synlinkTarget = std::filesystem::read_symlink(tmpFilePath);
            ASSERT_EQ(synlinkTarget, std::filesystem::path("utilities-terminal.svg"));

            // must clean up on every iteration, the extract(...) call below should recreate it
            std::filesystem::remove_all(tmpFilePath);
        }

        // extract dir
        if (traversal.getEntryPath() == "usr") {
            traversal.extract(tmpFilePath);

            ASSERT_TRUE(std::filesystem::is_directory(tmpFilePath));
        }

        traversal.next();
    }
}

TEST_F(TestTraversalType2, read) {
    std::string expected = "[Desktop Entry]\n"
                           "Version=1.0\n"
                           "Type=Application\n"
                           "Name=Echo\n"
                           "Comment=Just echo.\n"
                           "Exec=echo %F\n"
                           "Icon=utilities-terminal\n";

    while (!traversal.isCompleted()) {
        if (traversal.getEntryPath() == "usr/share/applications/echo.desktop") {
            std::string content{std::istreambuf_iterator<char>(traversal.read()),
                                std::istreambuf_iterator<char>()};

            ASSERT_EQ(expected, content);
            break;
        }

        traversal.next();
    }
}


TEST_F(TestTraversalType2, getEntryLink) {
    while (!traversal.isCompleted()) {
        // Extract Synlink
        if (traversal.getEntryPath() == ".DirIcon")
            ASSERT_EQ(traversal.getEntryLinkTarget(), "utilities-terminal.svg");

        traversal.next();
    }
}
