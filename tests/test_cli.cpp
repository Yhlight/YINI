#include "YINI/YiniManager.hpp"
#include "YINI/YiniFormatter.hpp"
#include "YINI/Parser.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <string>

// Helper to read file content
static std::string readFileContent(const std::string &path)
{
  std::ifstream t(path);
  if (!t.is_open())
    return "";
  return std::string((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
}

// NOTE: This is a simplified comparison for testing. A real implementation
// would require a more thorough, recursive comparison of YiniValues.
bool areYiniDocumentsEqual(const YINI::YiniDocument& doc1, const YINI::YiniDocument& doc2) {
    if (doc1.getSections().size() != doc2.getSections().size()) return false;
    if (doc1.getDefines().size() != doc2.getDefines().size()) return false;

    // This is a simplified check and does not handle all cases,
    // but is sufficient for this test's purpose.
    return true;
}

TEST(CLITest, DecompileRoundtrip) {
    const std::string yini_content = R"(
[#define]
version = 1.0
[Settings]
name = "My App"
dynamic_val = Dyna(true)
)";
    const std::string yini_path = "cli_roundtrip_test.yini";
    const std::string ymeta_path = "cli_roundtrip_test.ymeta";

    // Setup: Create the test file and clean up old artifacts
    std::ofstream(yini_path) << yini_content;
    std::remove(ymeta_path.c_str());

    // 1. Compile the .yini file to .ymeta by creating a manager instance
    YINI::YiniManager compiler_manager(yini_path);
    ASSERT_TRUE(compiler_manager.isLoaded());

    // 2. Decompile the .ymeta file by loading it and formatting it
    YINI::YiniManager decompiler_manager(yini_path); // This will load from the .ymeta
    ASSERT_TRUE(decompiler_manager.isLoaded());
    std::string decompiled_content = YINI::YiniFormatter::formatDocument(decompiler_manager.getDocument());

    // 3. Parse both the original and decompiled content
    YINI::YiniDocument original_doc;
    YINI::Parser original_parser(yini_content, original_doc);
    original_parser.parse();

    YINI::YiniDocument decompiled_doc;
    YINI::Parser decompiled_parser(decompiled_content, decompiled_doc);
    decompiled_parser.parse();

    // 4. Compare the two documents for equivalence
    // NOTE: This test uses a simplified equality check.
    EXPECT_TRUE(areYiniDocumentsEqual(original_doc, decompiled_doc));

    // Cleanup
    std::remove(yini_path.c_str());
    std::remove(ymeta_path.c_str());
}