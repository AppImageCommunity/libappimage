// system
#include <fstream>

// library
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

// local
#include <appimage/core/Exceptions.h>
#include <core/impl/TraversalType2.h>


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

    std::vector<std::string> expectedEntries = {
        ".DirIcon",
        "echo.desktop",
        "AppRun",
        "usr",
        "usr/bin",
        "usr/bin/echo",
        "usr/bin",
        "usr/share",
        "usr/share/applications",
        "usr/share/applications/echo.desktop",
        "usr/share/applications",
        "usr/share",
        "usr",
        "utilities-terminal.svg"
    };

    while (!traversal.isCompleted()) {
        const auto entryName = traversal.getEntryName();
        auto itr = std::find(expectedEntries.begin(), expectedEntries.end(), entryName);
        if (itr == expectedEntries.end())
            FAIL();
        else
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
