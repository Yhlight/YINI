#include "test_framework.h"
#include "../src/Core/Types.h"
#include "../src/Core/Value.h"

using namespace yini;
using namespace yini_test;

TEST(ValueTypeToString)
{
    ASSERT_EQ(value_type_to_string(ValueType::Integer), "int");
    ASSERT_EQ(value_type_to_string(ValueType::Float), "float");
    ASSERT_EQ(value_type_to_string(ValueType::Boolean), "bool");
    ASSERT_EQ(value_type_to_string(ValueType::String), "string");
}

TEST(ParseHexColor)
{
    auto color = parse_hex_color("#FF0000");
    ASSERT_TRUE(color.has_value());
    ASSERT_EQ(color->r, 255);
    ASSERT_EQ(color->g, 0);
    ASSERT_EQ(color->b, 0);
}

TEST(ParseHexColorInvalid)
{
    auto color = parse_hex_color("FF0000");  // Missing #
    ASSERT_FALSE(color.has_value());
    
    color = parse_hex_color("#FF00");  // Too short
    ASSERT_FALSE(color.has_value());
}

TEST(ValueInteger)
{
    Value v(123LL);
    ASSERT_TRUE(v.isInteger());
    ASSERT_EQ(v.asInteger(), 123);
    ASSERT_FALSE(v.isDynamic());
}

TEST(ValueFloat)
{
    Value v(3.14);
    ASSERT_TRUE(v.isFloat());
    ASSERT_EQ(v.asFloat(), 3.14);
}

TEST(ValueBoolean)
{
    Value v(true);
    ASSERT_TRUE(v.isBoolean());
    ASSERT_EQ(v.asBoolean(), true);
}

TEST(ValueString)
{
    Value v(std::string("hello"));
    ASSERT_TRUE(v.isString());
    ASSERT_EQ(v.asString(), "hello");
}

TEST(ValueColor)
{
    Color c(255, 128, 64);
    Value v(c);
    ASSERT_TRUE(v.isColor());
    Color result = v.asColor();
    ASSERT_EQ(result.r, 255);
    ASSERT_EQ(result.g, 128);
    ASSERT_EQ(result.b, 64);
}

TEST(ValueCoord2D)
{
    Coord coord(10.0, 20.0);
    Value v(coord);
    ASSERT_TRUE(v.isCoord());
    Coord result = v.asCoord();
    ASSERT_EQ(result.x, 10.0);
    ASSERT_EQ(result.y, 20.0);
    ASSERT_FALSE(result.z.has_value());
}

TEST(ValueCoord3D)
{
    Coord coord(10.0, 20.0, 30.0);
    Value v(coord);
    ASSERT_TRUE(v.isCoord());
    Coord result = v.asCoord();
    ASSERT_EQ(result.x, 10.0);
    ASSERT_EQ(result.y, 20.0);
    ASSERT_TRUE(result.z.has_value());
    ASSERT_EQ(result.z.value(), 30.0);
}

TEST(ValueDynamic)
{
    Value v(42LL);
    ASSERT_FALSE(v.isDynamic());
    v.setDynamic(true);
    ASSERT_TRUE(v.isDynamic());
}

int main()
{
    return TestRunner::instance().run();
}
