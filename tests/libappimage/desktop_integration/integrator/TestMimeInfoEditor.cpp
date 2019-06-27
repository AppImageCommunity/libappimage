//system
#include <sstream>

// library
#include <gtest/gtest.h>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

// local
#include <appimage/core/exceptions.h>
#include "utils/hashlib.h"
#include "integrator/MimeInfoEditor.h"

using namespace appimage::desktop_integration::integrator;
using namespace appimage::utils;

class MimeInfoEditorTests : public ::testing::Test {
protected:
    std::stringstream mimeInfo;
    std::stringstream mimeInfoWithIconEntry;
    std::stringstream expectedMimeInfo;
protected:

    void SetUp() override {
        mimeInfo << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">\n"
                    "  <mime-type type=\"application/x-starbright-file\">\n"
                    "    <sub-class-of type=\"application/xml\"/>\n"
                    "    <comment>Starbright File</comment>\n"
                    "    <glob pattern=\"*.starb\"/>\n"
                    "  </mime-type>\n"
                    "</mime-info>";

        mimeInfoWithIconEntry
            << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">\n"
               "  <mime-type type=\"application/x-starbright-file\">\n"
               "    <sub-class-of type=\"application/xml\"/>\n"
               "    <comment>Starbright File</comment>\n"
               "    <glob pattern=\"*.starb\"/>\n"
               "    <icon name=\"application-x-starbright-file\"/>\n"
               "  </mime-type>\n"
               "</mime-info>";

        expectedMimeInfo
            << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">\n"
               "  <mime-type type=\"application/x-starbright-file\">\n"
               "    <sub-class-of type=\"application/xml\"/>\n"
               "    <comment>Starbright File</comment>\n"
               "    <glob pattern=\"*.starb\"/>\n"
               "    <icon name=\"application-x-starbright-file-appimaged-d41d8cd98f00b204e9800998ecf8427e\"/>\n"
               "  </mime-type>\n"
               "</mime-info>";
    }
};

TEST_F(MimeInfoEditorTests, setIcon) {
    MimeInfoEditor editor(mimeInfo.str());
    editor.setDeployId("appimaged-d41d8cd98f00b204e9800998ecf8427e");
    std::string result = editor.edit();


    using boost::property_tree::ptree;

    std::stringstream resultStream(result);

    // populate tree structure pt
    ptree resultPt, expectedPt;
    read_xml(resultStream, resultPt, boost::property_tree::xml_parser::trim_whitespace);
    read_xml(expectedMimeInfo, expectedPt, boost::property_tree::xml_parser::trim_whitespace);


    ASSERT_EQ(resultPt, expectedPt);
}

TEST_F(MimeInfoEditorTests, updateIcon) {
    MimeInfoEditor editor(mimeInfoWithIconEntry.str());
    editor.setDeployId("appimaged-d41d8cd98f00b204e9800998ecf8427e");
    std::string result = editor.edit();


    using boost::property_tree::ptree;

    std::stringstream resultStream(result);

    // populate tree structure pt
    ptree resultPt, expectedPt;
    read_xml(resultStream, resultPt, boost::property_tree::xml_parser::trim_whitespace);
    read_xml(expectedMimeInfo, expectedPt, boost::property_tree::xml_parser::trim_whitespace);


    ASSERT_EQ(resultPt, expectedPt);
}

TEST_F(MimeInfoEditorTests, getIconNamesFromMimeTypeType) {
    MimeInfoEditor editor(mimeInfo.str());
    std::list<std::string> iconNames = editor.getMimeTypeIconNames();
    std::list<std::string> expectedIconNames = {"application-x-starbright-file"};

    ASSERT_EQ(iconNames, expectedIconNames);
}

TEST_F(MimeInfoEditorTests, getIconNamesFromMimeTypeIconName) {
    MimeInfoEditor editor(mimeInfoWithIconEntry.str());
    std::list<std::string> iconNames = editor.getMimeTypeIconNames();
    std::list<std::string> expectedIconNames = {"application-x-starbright-file"};

    ASSERT_EQ(iconNames, expectedIconNames);
}
