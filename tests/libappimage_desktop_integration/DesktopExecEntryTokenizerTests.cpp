// system
#include <sstream>

// library headers
#include <gtest/gtest.h>


// local
#include <utils/DesktopExecEntryTokenizer.h>

TEST(DesktopExecEntryTokenizerTest, iterateSimpleEntry) {
    appimage::utils::DesktopExecEntryTokenizer tokenizer("echo %F");
    ASSERT_TRUE(tokenizer.next());

    ASSERT_EQ(tokenizer.section(), "echo");
    ASSERT_EQ(tokenizer.sectionBegin(), 0);
    ASSERT_EQ(tokenizer.sectionSize(), 4);

    ASSERT_TRUE(tokenizer.next());

    ASSERT_EQ(tokenizer.section(), "%F");
    ASSERT_EQ(tokenizer.sectionBegin(), 5);
    ASSERT_EQ(tokenizer.sectionSize(), 2);

    ASSERT_FALSE(tokenizer.next());
}

TEST(DesktopExecEntryTokenizerTest, iterateQuotedEntry) {
    appimage::utils::DesktopExecEntryTokenizer tokenizer("\"/opt/custom apps/app\" %F --force=\"Rouge \\$1\"");

    ASSERT_TRUE(tokenizer.next());

    ASSERT_EQ(tokenizer.section(), "\"/opt/custom apps/app\"");
    ASSERT_EQ(tokenizer.sectionBegin(), 0);
    ASSERT_EQ(tokenizer.sectionSize(), 22);

    ASSERT_TRUE(tokenizer.next());

    ASSERT_EQ(tokenizer.section(), "%F");
    ASSERT_EQ(tokenizer.sectionBegin(), 23);
    ASSERT_EQ(tokenizer.sectionSize(), 2);

    ASSERT_TRUE(tokenizer.next());

    ASSERT_EQ(tokenizer.section(), "--force=\"Rouge \\$1\"");
    ASSERT_EQ(tokenizer.sectionBegin(), 26);
    ASSERT_EQ(tokenizer.sectionSize(), 19);

    ASSERT_FALSE(tokenizer.next());
}
