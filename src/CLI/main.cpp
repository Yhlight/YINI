#include <iostream>
#include <vector>
#include <variant>
#include <fstream>
#include <sstream>
#include "../Lexer/Lexer.h"
#include "../Parser/Parser.h"
#include "../Processor/Processor.h"
#include "../Parser/AST.h"
#include "../YMeta/Serializer.h"
#include "../YMeta/Deserializer.h"

// AST printing functions (from previous steps)
void printValue(const YINI::Value& value, int indent);
void printMap(const YINI::Map& map, int indent) { /* ... */ }
void printArray(const YINI::Array& array, int indent) { /* ... */ }
void printValue(const YINI::Value& value, int indent) {
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, YINI::String>) std::cout << '"' << arg << '"';
        else if constexpr (std::is_same_v<T, YINI::Integer>) std::cout << arg;
        else if constexpr (std::is_same_v<T, YINI::Float>) std::cout << arg;
        else if constexpr (std::is_same_v<T, YINI::Boolean>) std::cout << (arg ? "true" : "false");
        else if constexpr (std::is_same_v<T, YINI::Coordinate>) { std::cout << "(" << arg.x << ", " << arg.y << (arg.has_z ? ", " : "") << (arg.has_z ? std::to_string(arg.z) : "") << ")"; }
        else if constexpr (std::is_same_v<T, YINI::Color>) { std::cout << "Color(r=" << arg.r << ", g=" << arg.g << ", b=" << arg.b << ")"; }
        else if constexpr (std::is_same_v<T, YINI::Map>) printMap(arg, indent);
        else if constexpr (std::is_same_v<T, YINI::Array>) printArray(arg, indent);
        else if constexpr (std::is_same_v<T, YINI::Macro>) std::cout << "@" << arg.name;
    }, value.data);
    std::cout << "\n";
}
void printAST(const YINI::YiniFile& yini_file)
{
    for (const auto& [name, section] : yini_file.sections)
    {
        if (section.is_define_section || section.is_include_section) continue;
        std::cout << "[" << name << "]" << std::endl;
        for (const auto& kvp : section.pairs)
        {
            std::cout << "  " << kvp.key << " = ";
            if (kvp.value) printValue(*kvp.value, 2);
            else std::cout << "null\n";
        }
    }
}


void usage()
{
    std::cout << "YINI CLI Tool\n"
              << "Usage:\n"
              << "  yini_cli check <file.yini>               - Check syntax and semantic correctness.\n"
              << "  yini_cli compile <file.yini> [file.ymeta] - Compile a .yini file.\n"
              << "  yini_cli decompile <file.ymeta>          - Decompile a .ymeta file to stdout.\n";
}

void checkFile(const std::string& filename)
{
    std::ifstream file_stream(filename);
    if (!file_stream) { std::cerr << "Error: Failed to open file: " << filename << std::endl; exit(1); }
    std::string content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
    YINI::Lexer lexer(content);
    YINI::Parser parser(lexer);
    auto ast = parser.parse();
    YINI::Processor processor(std::move(ast));
    processor.process();
    std::cout << filename << ": OK" << std::endl;
}

void compileFile(const std::string& input_filename, const std::string& output_filename)
{
    std::ifstream file_stream(input_filename);
    if (!file_stream) { std::cerr << "Error: Failed to open file: " << input_filename << std::endl; exit(1); }
    std::string content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());

    YINI::Lexer lexer(content);
    YINI::Parser parser(lexer);
    auto ast = parser.parse();
    YINI::Processor processor(std::move(ast));
    auto processed_ast = processor.process();

    std::ofstream out_stream(output_filename, std::ios::binary);
    if (!out_stream) { std::cerr << "Error: Failed to open output file: " << output_filename << std::endl; exit(1); }

    YINI::Serializer serializer(*processed_ast);
    serializer.serialize(out_stream);

    std::cout << "Compiled " << input_filename << " to " << output_filename << std::endl;
}

void decompileFile(const std::string& input_filename)
{
    std::ifstream in_stream(input_filename, std::ios::binary);
    if (!in_stream) { std::cerr << "Error: Failed to open file: " << input_filename << std::endl; exit(1); }

    YINI::Deserializer deserializer(in_stream);
    auto ast = deserializer.deserialize();

    printAST(*ast);
}

int main(int argc, char* argv[])
{
    if (argc < 3) { usage(); return 1; }

    std::string command = argv[1];
    std::string inputFile = argv[2];

    try {
        if (command == "check")
        {
            checkFile(inputFile);
        }
        else if (command == "compile")
        {
            std::string outputFile = (argc > 3) ? argv[3] : inputFile.substr(0, inputFile.find_last_of('.')) + ".ymeta";
            compileFile(inputFile, outputFile);
        }
        else if (command == "decompile")
        {
            decompileFile(inputFile);
        }
        else
        {
            std::cerr << "Unknown command: " << command << std::endl;
            usage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
