#include <gtest/gtest.h>
#include <string>
#include "common/thirdlib/inih/INIReader.h"

using namespace std;

class INIReaderStringTest : public ::testing::Test {
protected:
    void SetUp() override {
        ini_content = R"(
[base]
axis_num = 2
net_name = test_network
input_delay = 100

[axis_0]
cycle_time = 2000
axis_index = 0
is_axis_virtual = true
velocity_ramp_option = 1

[axis_1]
cycle_time = 1000
axis_index = 1
is_axis_virtual = false
velocity_ramp_option = 0
)";
    }
    
    string ini_content;
};

// Test string parsing with constructor flag
TEST_F(INIReaderStringTest, ConstructorWithStringFlag) {
    INIReader reader(ini_content, true);
    
    EXPECT_EQ(reader.ParseError(), 0) << "Failed to parse string content";
    
    // Test sections
    auto sections = reader.Sections();
    EXPECT_EQ(sections.size(), 3) << "Expected 3 sections";
    EXPECT_NE(sections.find("base"), sections.end()) << "Missing [base] section";
    EXPECT_NE(sections.find("axis_0"), sections.end()) << "Missing [axis_0] section";
    EXPECT_NE(sections.find("axis_1"), sections.end()) << "Missing [axis_1] section";
}

// Test reading integer values
TEST_F(INIReaderStringTest, GetIntegerValues) {
    INIReader reader(ini_content, true);
    EXPECT_EQ(reader.ParseError(), 0);
    
    EXPECT_EQ(reader.GetInteger("base", "axis_num", -1), 2);
    EXPECT_EQ(reader.GetInteger("base", "input_delay", -1), 100);
    EXPECT_EQ(reader.GetInteger("axis_0", "cycle_time", -1), 2000);
    EXPECT_EQ(reader.GetInteger("axis_0", "axis_index", -1), 0);
    EXPECT_EQ(reader.GetInteger("axis_1", "cycle_time", -1), 1000);
    EXPECT_EQ(reader.GetInteger("axis_1", "axis_index", -1), 1);
}

// Test reading string values
TEST_F(INIReaderStringTest, GetStringValues) {
    INIReader reader(ini_content, true);
    EXPECT_EQ(reader.ParseError(), 0);
    
    EXPECT_EQ(reader.Get("base", "net_name", "not_found"), "test_network");
    EXPECT_EQ(reader.Get("base", "nonexistent", "default"), "default");
}

// Test reading boolean values
TEST_F(INIReaderStringTest, GetBooleanValues) {
    INIReader reader(ini_content, true);
    EXPECT_EQ(reader.ParseError(), 0);
    
    EXPECT_TRUE(reader.GetBoolean("axis_0", "is_axis_virtual", false));
    EXPECT_FALSE(reader.GetBoolean("axis_1", "is_axis_virtual", true));
    EXPECT_TRUE(reader.GetBoolean("axis_0", "nonexistent", true));
    EXPECT_FALSE(reader.GetBoolean("axis_1", "nonexistent", false));
}

// Test FromString static method
TEST_F(INIReaderStringTest, FromStringMethod) {
    INIReader reader = INIReader::FromString(ini_content);
    
    EXPECT_EQ(reader.ParseError(), 0) << "FromString method failed";
    
    // Test that it works the same as constructor
    EXPECT_EQ(reader.GetInteger("axis_0", "axis_index", -1), 0);
    EXPECT_EQ(reader.GetInteger("axis_1", "axis_index", -1), 1);
}

// Test error handling with invalid content
TEST_F(INIReaderStringTest, InvalidContent) {
    string invalid_content = "[section\ninvalid line without closing bracket";
    INIReader reader(invalid_content, true);
    
    // Should have parse error due to malformed section
    EXPECT_NE(reader.ParseError(), 0) << "Should detect parse error in invalid content";
}

// Test empty content
TEST_F(INIReaderStringTest, EmptyContent) {
    string empty_content = "";
    INIReader reader(empty_content, true);
    
    EXPECT_EQ(reader.ParseError(), 0) << "Empty content should parse successfully";
    EXPECT_EQ(reader.Sections().size(), 0) << "Empty content should have no sections";
}

// Test content with only comments
TEST_F(INIReaderStringTest, CommentOnlyContent) {
    string comment_content = R"(
; This is a comment
# This is also a comment
; Another comment
)";
    INIReader reader(comment_content, true);
    
    EXPECT_EQ(reader.ParseError(), 0) << "Comment-only content should parse successfully";
    EXPECT_EQ(reader.Sections().size(), 0) << "Comment-only content should have no sections";
}

// Test multiline values (if supported)
TEST_F(INIReaderStringTest, MultilineValues) {
    string multiline_content = R"(
[test]
description = This is a long description
    that spans multiple lines
    and should be handled properly
)";
    INIReader reader(multiline_content, true);
    
    EXPECT_EQ(reader.ParseError(), 0) << "Multiline content should parse successfully";
    EXPECT_NE(reader.Sections().find("test"), reader.Sections().end()) << "Should find [test] section";
}
