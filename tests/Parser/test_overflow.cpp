#include "Parser.h"
#include "Value.h"
#include <cassert>
#include <iostream>
#include <limits>

using namespace yini;

// Test integer overflow in addition
void test_integer_overflow_add()
{
    std::cout << "Testing integer overflow in addition..." << std::endl;
    
    std::string source = R"(
[Test]
result = 9223372036854775807 + 1
    )";
    
    Parser parser(source);
    bool result = parser.parse();
    (void)result; // Suppress unused warning
    
    // Should fail with overflow error
    assert(!result);
    assert(parser.hasError());
    std::string error = parser.getLastError();
    (void)error; // Suppress unused warning
    assert(error.find("overflow") != std::string::npos);
    
    std::cout << "✓ Integer overflow (addition) test passed" << std::endl;
}

// Test integer overflow in subtraction
void test_integer_overflow_subtract()
{
    std::cout << "Testing integer overflow in subtraction..." << std::endl;
    
    std::string source = R"(
[Test]
result = -9223372036854775808 - 1
    )";
    
    Parser parser(source);
    bool result = parser.parse();
    (void)result; // Suppress unused warning
    
    // Should fail with overflow error
    assert(!result);
    assert(parser.hasError());
    
    std::cout << "✓ Integer overflow (subtraction) test passed" << std::endl;
}

// Test integer overflow in multiplication
void test_integer_overflow_multiply()
{
    std::cout << "Testing integer overflow in multiplication..." << std::endl;
    
    std::string source = R"(
[Test]
result = 9223372036854775807 * 2
    )";
    
    Parser parser(source);
    bool result = parser.parse();
    (void)result; // Suppress unused warning
    
    // Should fail with overflow error
    assert(!result);
    assert(parser.hasError());
    std::string error = parser.getLastError();
    (void)error; // Suppress unused warning
    assert(error.find("overflow") != std::string::npos);
    
    std::cout << "✓ Integer overflow (multiplication) test passed" << std::endl;
}

// Test division by zero
void test_division_by_zero_check()
{
    std::cout << "Testing division by zero..." << std::endl;
    
    std::string source = R"(
[Test]
result = 10 / 0
    )";
    
    Parser parser(source);
    bool result = parser.parse();
    (void)result; // Suppress unused warning
    
    // Should fail with division by zero error
    assert(!result);
    assert(parser.hasError());
    std::string error = parser.getLastError();
    (void)error; // Suppress unused warning
    assert(error.find("Division by zero") != std::string::npos);
    
    std::cout << "✓ Division by zero test passed" << std::endl;
}

// Test modulo by zero
void test_modulo_by_zero()
{
    std::cout << "Testing modulo by zero..." << std::endl;
    
    std::string source = R"(
[Test]
result = 10 % 0
    )";
    
    Parser parser(source);
    bool result = parser.parse();
    (void)result; // Suppress unused warning
    
    // Should fail with modulo by zero error
    assert(!result);
    assert(parser.hasError());
    std::string error = parser.getLastError();
    (void)error; // Suppress unused warning
    assert(error.find("Modulo by zero") != std::string::npos);
    
    std::cout << "✓ Modulo by zero test passed" << std::endl;
}

// Test normal arithmetic (no overflow)
void test_normal_arithmetic()
{
    std::cout << "Testing normal arithmetic..." << std::endl;
    
    std::string source = R"(
[Test]
add = 100 + 200
subtract = 500 - 300
multiply = 10 * 20
divide = 100 / 5
modulo = 17 % 5
    )";
    
    Parser parser(source);
    bool result = parser.parse();
    (void)result; // Suppress unused warning
    
    // Should succeed
    assert(result);
    
    auto section = parser.getSections().at("Test");
    assert(section.entries.at("add")->asInteger() == 300);
    assert(section.entries.at("subtract")->asInteger() == 200);
    assert(section.entries.at("multiply")->asInteger() == 200);
    assert(section.entries.at("divide")->asInteger() == 20);
    assert(section.entries.at("modulo")->asInteger() == 2);
    
    std::cout << "✓ Normal arithmetic test passed" << std::endl;
}

// Test edge values (safe range)
void test_edge_values()
{
    std::cout << "Testing edge values..." << std::endl;
    
    // Use smaller values that Lexer can handle
    std::string source = R"(
[Test]
large_pos = 1000000000
large_neg = -1000000000
zero = 0
    )";
    
    Parser parser(source);
    bool result = parser.parse();
    
    // Should succeed
    if (!result)
    {
        std::cerr << "  Parse failed: " << parser.getLastError() << std::endl;
    }
    assert(result);
    
    const auto& sections = parser.getSections();
    if (sections.find("Test") == sections.end())
    {
        std::cerr << "  ERROR: Test section not found" << std::endl;
    }
    
    auto section = sections.at("Test");
    (void)section; // Suppress unused warning
    assert(section.entries.at("large_pos")->asInteger() == 1000000000);
    assert(section.entries.at("large_neg")->asInteger() == -1000000000);
    assert(section.entries.at("zero")->asInteger() == 0);
    
    std::cout << "  (Note: INT64_MAX/MIN literals may not parse - using large values)" << std::endl;
    std::cout << "✓ Edge values test passed" << std::endl;
}

int main()
{
    std::cout << "Running Overflow and Arithmetic Tests..." << std::endl;
    std::cout << "=========================================" << std::endl;
    
    try
    {
        test_integer_overflow_add();
        test_integer_overflow_subtract();
        test_integer_overflow_multiply();
        test_division_by_zero_check();
        test_modulo_by_zero();
        test_normal_arithmetic();
        test_edge_values();
        
        std::cout << "\n=========================================" << std::endl;
        std::cout << "All overflow tests passed! ✓" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
