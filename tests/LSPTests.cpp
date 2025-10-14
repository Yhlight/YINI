#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <sstream>

// This is an unusual include, but it allows us to test the static
// functions inside main.cpp directly.
#include "../src/CLI/main.cpp"

TEST(LSPTests, ServerSendsDiagnosticsOnError)
{
    // Redirect cerr to a stringstream to capture the log output
    std::stringstream log_buffer;
    std::streambuf* old_cerr = std::cerr.rdbuf(log_buffer.rdbuf());

    std::string uri = "file:///test.yini";
    // This is an invalid YINI file because a value is missing after the equals sign.
    std::string invalid_text = "[MySection]\nkey = ";

    update_document_info(uri, invalid_text);

    // Restore cerr
    std::cerr.rdbuf(old_cerr);

    std::string logs = log_buffer.str();

    // Check that a textDocument/publishDiagnostics notification was sent in the logs
    ASSERT_NE(logs.find("textDocument/publishDiagnostics"), std::string::npos);
    // Check that the diagnostic message contains the expected parser error
    ASSERT_NE(logs.find("Expect expression."), std::string::npos);
}

TEST(LSPTests, ServerClearsDiagnosticsOnSuccess)
{
    // Redirect cerr to a stringstream to capture the log output
    std::stringstream log_buffer;
    std::streambuf* old_cerr = std::cerr.rdbuf(log_buffer.rdbuf());

    std::string uri = "file:///test.yini";
    std::string valid_text = "[MySection]\nkey = value";

    update_document_info(uri, valid_text);

    // Restore cerr
    std::cerr.rdbuf(old_cerr);

    std::string logs = log_buffer.str();

    // Check that a textDocument/publishDiagnostics notification was sent
    ASSERT_NE(logs.find("textDocument/publishDiagnostics"), std::string::npos);
    // Check that the diagnostics array is empty, indicating success
    ASSERT_NE(logs.find("\"diagnostics\":[]"), std::string::npos);
}

TEST(LSPTests, ServerSendsDiagnosticsForCircularInheritance)
{
    std::stringstream log_buffer;
    std::streambuf* old_cerr = std::cerr.rdbuf(log_buffer.rdbuf());

    std::string uri = "file:///test_circular.yini";
    std::string text = "[A]:B\n[B]:A";

    update_document_info(uri, text);

    std::cerr.rdbuf(old_cerr);
    std::string logs = log_buffer.str();

    ASSERT_NE(logs.find("textDocument/publishDiagnostics"), std::string::npos);
    ASSERT_NE(logs.find("Circular inheritance detected"), std::string::npos);
}

TEST(LSPTests, ServerSendsDiagnosticsForSchemaViolation)
{
    std::stringstream log_buffer;
    std::streambuf* old_cerr = std::cerr.rdbuf(log_buffer.rdbuf());

    std::string uri = "file:///test_schema.yini";
    std::string text = "[#schema]\n[MySection]\nmy_key = !, int, e\n\n[MySection]\n";

    update_document_info(uri, text);

    std::cerr.rdbuf(old_cerr);
    std::string logs = log_buffer.str();

    ASSERT_NE(logs.find("textDocument/publishDiagnostics"), std::string::npos);
    ASSERT_NE(logs.find("Missing required key"), std::string::npos);
}
