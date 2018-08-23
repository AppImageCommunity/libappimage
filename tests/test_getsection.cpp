// system headers
#include <cmath>

// library headers
#include <gtest/gtest.h>

// local headers
#include <appimage/appimage.h>
#include "fixtures.h"

extern "C" {
    #include "getsection.h"
}


using namespace std;


// most simple derivative class for better naming of the tests in this file
class GetSectionCTest : public AppImageKitTest {};



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
