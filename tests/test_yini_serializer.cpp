#include <gtest/gtest.h>
#include "YINI/Parser.hpp"
#include "YINI/YiniSerializer.hpp"
#include <string>
#include <algorithm>

// Helper function to normalize strings for comparison by removing whitespace
static std::string normalize(const std::string& str) {
    std::string normalized;
    std::remove_copy_if(str.begin(), str.end(), std::back_inserter(normalized), ::isspace);
    return normalized;
}

TEST(YiniSerializerTest, RoundTripTest)
{
    const std::string input = R"([Core]
name = "YINI"
version = 1.0
enabled = true

[Data]
values = [1, 2, 3]
my_pair = {key: "value"}
my_map = {{a: 1, b: false}}

)";

    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    parser.parse();

    std::string serialized_output = YINI::YiniSerializer::serialize(doc);

    EXPECT_EQ(normalize(serialized_output), normalize(input));
}