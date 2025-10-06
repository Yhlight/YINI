#ifndef YINI_TEST_FRAMEWORK_H
#define YINI_TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace yini_test
{

class TestResult
{
public:
    bool passed;
    std::string name;
    std::string error_message;
    
    TestResult(const std::string& n, bool p, const std::string& msg = "")
        : passed(p), name(n), error_message(msg) {}
};

class TestRunner
{
public:
    static TestRunner& instance()
    {
        static TestRunner runner;
        return runner;
    }
    
    void addTest(const std::string& name, std::function<void()> test_func)
    {
        tests_.push_back({name, test_func});
    }
    
    int run()
    {
        int passed = 0;
        int failed = 0;
        
        std::cout << "\n========================================\n";
        std::cout << "Running " << tests_.size() << " tests...\n";
        std::cout << "========================================\n\n";
        
        for (const auto& test : tests_)
        {
            try
            {
                test.second();
                std::cout << "✓ PASS: " << test.first << "\n";
                passed++;
            }
            catch (const std::exception& e)
            {
                std::cout << "✗ FAIL: " << test.first << "\n";
                std::cout << "  Error: " << e.what() << "\n";
                failed++;
            }
        }
        
        std::cout << "\n========================================\n";
        std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
        std::cout << "========================================\n\n";
        
        return failed > 0 ? 1 : 0;
    }
    
private:
    std::vector<std::pair<std::string, std::function<void()>>> tests_;
};

#define TEST(name) \
    void test_##name(); \
    struct TestRegistrar_##name { \
        TestRegistrar_##name() { \
            yini_test::TestRunner::instance().addTest(#name, test_##name); \
        } \
    }; \
    static TestRegistrar_##name registrar_##name; \
    void test_##name()

#define ASSERT(condition) \
    if (!(condition)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #condition); \
    }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #a + " == " + #b); \
    }

#define ASSERT_TRUE(condition) ASSERT(condition)
#define ASSERT_FALSE(condition) ASSERT(!(condition))

} // namespace yini_test

#endif // YINI_TEST_FRAMEWORK_H
