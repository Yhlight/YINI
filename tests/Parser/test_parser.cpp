#include "Parser.h"
#include "Value.h"
#include <cassert>
#include <iostream>

using namespace yini;

void test_simple_section()
{
    std::cout << "Testing simple section..." << std::endl;
    
    std::string source = R"(
[Config]
key1 = 123
key2 = "value"
key3 = true
    )";
    
    Parser parser(source);
    assert(parser.parse());
    
    const auto& sections = parser.getSections();
    assert(sections.find("Config") != sections.end());
    
    const auto& config = sections.at("Config");
    assert(config.entries.find("key1") != config.entries.end());
    assert(config.entries.at("key1")->isInteger());
    assert(config.entries.at("key1")->asInteger() == 123);
    
    assert(config.entries.at("key2")->isString());
    assert(config.entries.at("key2")->asString() == "value");
    
    assert(config.entries.at("key3")->isBoolean());
    assert(config.entries.at("key3")->asBoolean() == true);
    
    std::cout << "✓ Simple section test passed" << std::endl;
}

void test_arrays()
{
    std::cout << "Testing arrays..." << std::endl;
    
    std::string source = R"(
[Config]
arr = [1, 2, 3]
    )";
    
    Parser parser(source);
    assert(parser.parse());
    
    const auto& sections = parser.getSections();
    const auto& config = sections.at("Config");
    
    assert(config.entries.at("arr")->isArray());
    auto arr = config.entries.at("arr")->asArray();
    assert(arr.size() == 3);
    assert(arr[0]->asInteger() == 1);
    assert(arr[1]->asInteger() == 2);
    assert(arr[2]->asInteger() == 3);
    
    std::cout << "✓ Arrays test passed" << std::endl;
}

void test_inheritance()
{
    std::cout << "Testing inheritance..." << std::endl;
    
    std::string source = R"(
[Base]
key1 = 100
key2 = 200

[Derived] : Base
key2 = 300
key3 = 400
    )";
    
    Parser parser(source);
    assert(parser.parse());
    
    const auto& sections = parser.getSections();
    const auto& derived = sections.at("Derived");
    
    // key1 should be inherited
    assert(derived.entries.find("key1") != derived.entries.end());
    assert(derived.entries.at("key1")->asInteger() == 100);
    
    // key2 should be overridden
    assert(derived.entries.at("key2")->asInteger() == 300);
    
    // key3 is new
    assert(derived.entries.at("key3")->asInteger() == 400);
    
    std::cout << "✓ Inheritance test passed" << std::endl;
}

void test_quick_register()
{
    std::cout << "Testing quick register..." << std::endl;
    
    std::string source = R"(
[Registry]
+= "value1"
+= "value2"
+= "value3"
    )";
    
    Parser parser(source);
    bool success = parser.parse();
    if (!success)
    {
        std::cerr << "Parse error: " << parser.getLastError() << std::endl;
    }
    assert(success);
    
    const auto& sections = parser.getSections();
    const auto& registry = sections.at("Registry");
    
    assert(registry.entries.size() == 3);
    assert(registry.entries.at("0")->asString() == "value1");
    assert(registry.entries.at("1")->asString() == "value2");
    assert(registry.entries.at("2")->asString() == "value3");
    
    std::cout << "✓ Quick register test passed" << std::endl;
}

void test_arithmetic()
{
    std::cout << "Testing arithmetic..." << std::endl;
    
    std::string source = R"(
[Math]
add = 1 + 2
multiply = 3 * 4
complex = 1 + 2 * 3
    )";
    
    Parser parser(source);
    assert(parser.parse());
    
    const auto& sections = parser.getSections();
    const auto& math = sections.at("Math");
    
    assert(math.entries.at("add")->asInteger() == 3);
    assert(math.entries.at("multiply")->asInteger() == 12);
    assert(math.entries.at("complex")->asInteger() == 7); // 1 + (2 * 3)
    
    std::cout << "✓ Arithmetic test passed" << std::endl;
}

void test_color()
{
    std::cout << "Testing color..." << std::endl;
    
    std::string source = R"(
[Visual]
color1 = #FF0000
color2 = Color(255, 0, 0)
    )";
    
    Parser parser(source);
    bool success = parser.parse();
    if (!success)
    {
        std::cerr << "Parse error: " << parser.getLastError() << std::endl;
    }
    assert(success);
    
    const auto& sections = parser.getSections();
    const auto& visual = sections.at("Visual");
    
    assert(visual.entries.at("color1")->isColor());
    auto c1 = visual.entries.at("color1")->asColor();
    assert(c1.r == 255 && c1.g == 0 && c1.b == 0);
    
    assert(visual.entries.at("color2")->isColor());
    auto c2 = visual.entries.at("color2")->asColor();
    assert(c2.r == 255 && c2.g == 0 && c2.b == 0);
    
    std::cout << "✓ Color test passed" << std::endl;
}

void test_coord()
{
    std::cout << "Testing coordinates..." << std::endl;
    
    std::string source = R"(
[Position]
pos2d = Coord(10, 20)
pos3d = Coord(10, 20, 30)
    )";
    
    Parser parser(source);
    assert(parser.parse());
    
    const auto& sections = parser.getSections();
    const auto& position = sections.at("Position");
    
    assert(position.entries.at("pos2d")->isCoord());
    auto c2d = position.entries.at("pos2d")->asCoord();
    assert(c2d.x == 10 && c2d.y == 20 && !c2d.z.has_value());
    
    assert(position.entries.at("pos3d")->isCoord());
    auto c3d = position.entries.at("pos3d")->asCoord();
    assert(c3d.x == 10 && c3d.y == 20 && c3d.z.has_value() && c3d.z.value() == 30);
    
    std::cout << "✓ Coordinates test passed" << std::endl;
}

void test_defines()
{
    std::cout << "Testing defines..." << std::endl;
    
    std::string source = R"(
[#define]
width = 1920
height = 1080

[Config]
key1 = @width
    )";
    
    Parser parser(source);
    bool success = parser.parse();
    if (!success)
    {
        std::cerr << "Parse error: " << parser.getLastError() << std::endl;
    }
    assert(success);
    
    const auto& defines = parser.getDefines();
    assert(defines.find("width") != defines.end());
    assert(defines.at("width")->asInteger() == 1920);
    assert(defines.at("height")->asInteger() == 1080);
    
    std::cout << "✓ Defines test passed" << std::endl;
}

void test_includes()
{
    std::cout << "Testing includes..." << std::endl;
    
    std::string source = R"(
[#include]
+= "file1.yini"
+= "file2.yini"
    )";
    
    Parser parser(source);
    bool success = parser.parse();
    if (!success)
    {
        std::cerr << "Parse error: " << parser.getLastError() << std::endl;
    }
    assert(success);
    
    const auto& includes = parser.getIncludes();
    assert(includes.size() == 2);
    assert(includes[0] == "file1.yini");
    assert(includes[1] == "file2.yini");
    
    std::cout << "✓ Includes test passed" << std::endl;
}

void test_map()
{
    std::cout << "Testing map..." << std::endl;
    
    std::string source = R"(
[Config]
settings = {width: 1920, height: 1080}
    )";
    
    Parser parser(source);
    assert(parser.parse());
    
    const auto& sections = parser.getSections();
    const auto& config = sections.at("Config");
    
    assert(config.entries.at("settings")->isMap());
    auto map = config.entries.at("settings")->asMap();
    assert(map.size() == 2);
    assert(map.at("width")->asInteger() == 1920);
    assert(map.at("height")->asInteger() == 1080);
    
    std::cout << "✓ Map test passed" << std::endl;
}

void test_dynamic()
{
    std::cout << "Testing dynamic values..." << std::endl;
    
    std::string source = R"(
[Config]
dyna_value = Dyna(100)
    )";
    
    Parser parser(source);
    assert(parser.parse());
    
    const auto& sections = parser.getSections();
    const auto& config = sections.at("Config");
    
    assert(config.entries.at("dyna_value")->isDynamic());
    
    std::cout << "✓ Dynamic values test passed" << std::endl;
}

int main()
{
    std::cout << "Running Parser tests..." << std::endl;
    std::cout << "==========================================" << std::endl;
    
    try
    {
        test_simple_section();
        test_arrays();
        test_inheritance();
        test_quick_register();
        test_arithmetic();
        test_color();
        test_coord();
        test_defines();
        test_includes();
        test_map();
        test_dynamic();
        
        std::cout << "\n==========================================" << std::endl;
        std::cout << "All tests passed! ✓" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
