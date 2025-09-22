#include "Loader.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Json.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

namespace YINI
{

Loader::Loader() {}

Document Loader::load(const std::string& filepath, bool use_cache)
{
    std::string ymeta_path = filepath;
    size_t dot_pos = ymeta_path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        ymeta_path.replace(dot_pos, ymeta_path.length(), ".ymeta");
    } else {
        ymeta_path += ".ymeta";
    }

    // Caching is not fully implemented yet.
    // For now, always load from source but save the cache.

    std::set<std::string> visited_files;
    Document doc = loadRecursive(filepath, visited_files);
    save_ymeta(ymeta_path, doc);
    return doc;
}

void Loader::save_ymeta(const std::string& filepath, const Document& doc) {
    std::ofstream out(filepath);
    if (!out.is_open()) {
        std::cerr << "Warning: Could not open .ymeta file for writing: " << filepath << std::endl;
        return;
    }
    out << Json::to_json(doc);
}

Document Loader::load_from_ymeta(const std::string& filepath) {
    // Not implemented yet
    throw std::runtime_error("Loading from .ymeta is not implemented yet.");
}


Document Loader::loadRecursive(const std::string& filepath, std::set<std::string>& visited_files)
{
    if (visited_files.count(filepath))
    {
        std::cerr << "Warning: Circular include detected for file: " << filepath << std::endl;
        return Document();
    }
    visited_files.insert(filepath);

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    Document doc = parser.parse();

    for (const auto& include_path : doc.includes)
    {
        size_t last_slash = filepath.find_last_of("/\\");
        std::string dir = (last_slash == std::string::npos) ? "" : filepath.substr(0, last_slash + 1);
        Document included_doc = loadRecursive(dir + include_path, visited_files);
        mergeDocuments(doc, included_doc);
    }

    visited_files.erase(filepath);
    return doc;
}

void Loader::mergeDocuments(Document& base, Document& to_merge)
{
    for (auto& def : to_merge.defines)
    {
        base.defines[def.first] = std::move(def.second);
    }

    for (auto& merge_sec : to_merge.sections)
    {
        bool section_found = false;
        for (auto& base_sec : base.sections)
        {
            if (base_sec.name == merge_sec.name)
            {
                section_found = true;
                for (auto& merge_pair : merge_sec.pairs)
                {
                    bool pair_found = false;
                    for (auto& base_pair : base_sec.pairs)
                    {
                        if (base_pair.key == merge_pair.key)
                        {
                            pair_found = true;
                            base_pair.value = std::move(merge_pair.value);
                            break;
                        }
                    }
                    if (!pair_found)
                    {
                        base_sec.pairs.push_back(std::move(merge_pair));
                    }
                }
                for (auto& anon_val : merge_sec.anonymous_values)
                {
                    base_sec.anonymous_values.push_back(std::move(anon_val));
                }
                break;
            }
        }
        if (!section_found)
        {
            base.sections.push_back(std::move(merge_sec));
        }
    }
}


} // namespace YINI
