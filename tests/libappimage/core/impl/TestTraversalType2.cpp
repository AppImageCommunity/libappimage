// system
#include <fstream>

// library
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include <appimage/core/Exceptions.h>
#include <core/impl/TraversalType2.h>


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

    ASSERT_EQ(traversal.getEntryName(), std::string());
    ASSERT_NO_THROW(traversal.next());

    std::vector<std::pair<std::string, entry::Type>> expectedEntries = {
        std::make_pair(".DirIcon", entry::LINK),
        std::make_pair("AppRun", entry::REGULAR),
        std::make_pair("echo.desktop", entry::LINK),
        std::make_pair("usr", entry::DIR),
        std::make_pair("usr/bin", entry::DIR),
        std::make_pair("usr/bin/echo", entry::REGULAR),
        std::make_pair("usr/bin", entry::DIR),
        std::make_pair("usr/share", entry::DIR),
        std::make_pair("usr/share/applications", entry::DIR),
        std::make_pair("usr/share/applications/echo.desktop", entry::REGULAR),
        std::make_pair("usr/share/applications", entry::DIR),
        std::make_pair("usr/share", entry::DIR),
        std::make_pair("usr", entry::DIR),
        std::make_pair("utilities-terminal.svg", entry::REGULAR),
    };

    while (!traversal.isCompleted()) {
        auto entry = std::make_pair(traversal.getEntryName(), traversal.getEntryType());

        auto itr = std::find(expectedEntries.begin(), expectedEntries.end(), entry);
        ASSERT_NE(itr, expectedEntries.end());

        expectedEntries.erase(itr);

        ASSERT_NO_THROW(traversal.next());
    }

    ASSERT_TRUE(expectedEntries.empty());
}

TEST_F(TestTraversalType2, extract) {
    auto tmpPath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

    while (!traversal.isCompleted()) {

        // Extract Synlink
        if (traversal.getEntryName() == ".DirIcon") {
            traversal.extract(tmpPath.string());

            ASSERT_TRUE(boost::filesystem::is_symlink(tmpPath));

            auto synlinkTarget = boost::filesystem::read_symlink(tmpPath);
            ASSERT_EQ(synlinkTarget, boost::filesystem::path("utilities-terminal.svg"));

            boost::filesystem::remove_all(tmpPath);
        }

        // Extract Dir
        if (traversal.getEntryName() == "usr") {
            traversal.extract(tmpPath.string());

            ASSERT_TRUE(boost::filesystem::is_directory(tmpPath));
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
        if (traversal.getEntryName() == "echo.desktop") {
            std::string content{std::istreambuf_iterator<char>(traversal.read()),
                                std::istreambuf_iterator<char>()};

            ASSERT_EQ(expected, content);
            break;
        }

        traversal.next();
    }
}
