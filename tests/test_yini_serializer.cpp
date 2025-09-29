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

TEST(YiniSerializerTest, ComprehensiveRoundTripTest)
{
    const std::string input = R"([#define]
base_color = Color(255, 0, 0)

[Core]
name = "YINI"
version = 1.0
enabled = true
data = [1, 2, {key: "nested"}]

[CustomTypes]
player_pos = Coord(10, 20)
enemy_color = @base_color
asset = Path(items/sword.png)
dynamic_health = Dyna(100)

[Data]
my_pair = {level: 5}
my_map = {{a: 1, b: false, c: [1,2]}}

)";

    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    parser.parse();

    // The parser resolves macros, so we create an expected output
    // where the macro is replaced by its value.
    const std::string expected_output = R"([#define]
base_color=Color(255,0,0)
[Core]
name="YINI"
version=1.0
enabled=true
data=[1, 2, {key:"nested"}]
[CustomTypes]
player_pos=Coord(10,20)
enemy_color=Color(255,0,0)
asset=Path(items/sword.png)
dynamic_health=Dyna(100)
[Data]
my_pair={level:5}
my_map={{a:1,b:false,c:[1,2]}}
)";

    std::string serialized_output = YINI::YiniSerializer::serialize(doc);

    EXPECT_EQ(normalize(serialized_output), normalize(expected_output));
}