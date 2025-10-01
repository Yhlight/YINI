#include "YINI/YiniManager.hpp"
#include "YINI/JsonDeserializer.hpp"
#include "YINI/JsonSerializer.hpp"
#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YiniValueToString.hpp"
#include <cstdio>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>

namespace YINI
{
namespace fs = std::filesystem;

// Helper to read file content
static std::string read_file_content(const std::string &path)
{
    std::ifstream t(path);
    if (!t.is_open())
    {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
}

static std::string get_ymeta_path(const std::string &yini_file_path)
{
    std::string ymeta_path = yini_file_path;
    size_t dot_pos = ymeta_path.rfind(".yini");
    if (dot_pos != std::string::npos)
    {
        ymeta_path.replace(dot_pos, 5, ".ymeta");
    }
    else
    {
        ymeta_path += ".ymeta";
    }
    return ymeta_path;
}

YiniManager::YiniManager(const std::string &yini_file_path)
    : m_yini_file_path(yini_file_path),
      m_ymeta_file_path(get_ymeta_path(yini_file_path)),
      m_is_loaded(false)
{
    m_is_loaded = load_document();
}

YiniManager::~YiniManager()
{
    if (m_is_loaded)
    {
        write_back_dyna_values();
    }
}

const YiniDocument &YiniManager::get_document() const
{
    return document;
}

bool YiniManager::is_loaded() const
{
    return m_is_loaded;
}

bool YiniManager::load_document()
{
    if (fs::exists(m_ymeta_file_path) && fs::exists(m_yini_file_path))
    {
        auto ymeta_time = fs::last_write_time(m_ymeta_file_path);
        auto yini_time = fs::last_write_time(m_yini_file_path);

        if (ymeta_time >= yini_time)
        {
            std::string ymeta_content = read_file_content(m_ymeta_file_path);
            if (!ymeta_content.empty())
            {
                if (JsonDeserializer::deserialize(ymeta_content, document))
                {
                    m_original_document = document; // Cache original state
                    return true;
                }
            }
        }
    }

    std::string yini_content = read_file_content(m_yini_file_path);
    if (yini_content.empty())
    {
        if (fs::exists(m_ymeta_file_path))
        {
            std::string ymeta_content = read_file_content(m_ymeta_file_path);
            if (!ymeta_content.empty() && JsonDeserializer::deserialize(ymeta_content, document))
            {
                m_original_document = document; // Cache original state
                return true;
            }
        }
        return false;
    }

    try
    {
        document = {};
        std::string base_path = ".";
        size_t last_slash_idx = m_yini_file_path.rfind('/');
        if (std::string::npos != last_slash_idx)
        {
            base_path = m_yini_file_path.substr(0, last_slash_idx);
        }

        Parser parser(yini_content, document, base_path);
        parser.parse();
        document.resolveInheritance();
    }
    catch (const YiniException &e)
    {
        std::cerr << "Failed to parse YINI file: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "An unexpected error occurred during parsing: " << e.what() << std::endl;
        return false;
    }

    m_original_document = document; // Cache original state
    return save_document();
}

bool YiniManager::save_document()
{
    const int max_backups = 5;
    std::string oldest_backup = m_ymeta_file_path + ".bak" + std::to_string(max_backups);
    if (fs::exists(oldest_backup))
    {
        fs::remove(oldest_backup);
    }

    for (int i = max_backups - 1; i >= 1; --i)
    {
        std::string current_backup = m_ymeta_file_path + ".bak" + std::to_string(i);
        std::string next_backup = m_ymeta_file_path + ".bak" + std::to_string(i + 1);
        if (fs::exists(current_backup))
        {
            fs::rename(current_backup, next_backup);
        }
    }

    if (fs::exists(m_ymeta_file_path))
    {
        std::string first_backup = m_ymeta_file_path + ".bak1";
        fs::rename(m_ymeta_file_path, first_backup);
    }

    std::ofstream out_file(m_ymeta_file_path);
    if (!out_file.is_open())
    {
        return false;
    }
    std::string json_content = JsonSerializer::serialize(document);
    out_file << json_content;
    return true;
}

void YiniManager::write_back_dyna_values()
{
    // This is the new, non-destructive implementation.
    std::map<std::string, std::map<std::string, YiniValue>> changed_values;

    // 1. Detect which Dyna() values have changed.
    for (const auto& section : document.getSections())
    {
        const auto* original_section = m_original_document.findSection(section.name);
        if (!original_section) continue;

        for (const auto& pair : section.pairs)
        {
            if (!std::holds_alternative<std::unique_ptr<YiniDynaValue>>(pair.value.data))
            {
                continue; // Not a dyna value
            }

            auto it = std::find_if(original_section->pairs.begin(), original_section->pairs.end(),
                                   [&](const YiniKeyValuePair& p) { return p.key == pair.key; });

            if (it != original_section->pairs.end())
            {
                if (pair.value != it->value)
                {
                    changed_values[section.name][pair.key] = pair.value;
                }
            }
        }
    }

    if (changed_values.empty())
    {
        return; // No changes to write back
    }

    // 2. Read the original file and update it line-by-line
    std::ifstream in_file(m_yini_file_path);
    if (!in_file.is_open()) return;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in_file, line))
    {
        lines.push_back(line);
    }
    in_file.close();

    std::string current_section_name;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::string trimmed_line = lines[i];
        trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t"));
        trimmed_line.erase(trimmed_line.find_last_not_of(" \t") + 1);

        if (!trimmed_line.empty() && trimmed_line.front() == '[' && trimmed_line.back() == ']')
        {
            current_section_name = trimmed_line.substr(1, trimmed_line.length() - 2);
            size_t colon_pos = current_section_name.find(':');
            if (colon_pos != std::string::npos)
            {
                current_section_name = current_section_name.substr(0, colon_pos);
                current_section_name.erase(current_section_name.find_last_not_of(" \t") + 1);
            }
        }

        if (!current_section_name.empty() && changed_values.count(current_section_name))
        {
            size_t equals_pos = lines[i].find('=');
            if (equals_pos != std::string::npos)
            {
                std::string key = lines[i].substr(0, equals_pos);
                key.erase(key.find_last_not_of(" \t") + 1);
                key.erase(0, key.find_first_not_of(" \t"));

                if (changed_values[current_section_name].count(key))
                {
                    std::string indent = lines[i].substr(0, lines[i].find_first_not_of(" \t"));
                    YiniValue new_val = changed_values[current_section_name][key];
                    lines[i] = indent + key + " = " + valueToString(new_val);
                }
            }
        }
    }

    // 3. Write the modified content back to the file
    std::ofstream out_file(m_yini_file_path);
    if (out_file.is_open())
    {
        for (size_t i = 0; i < lines.size(); ++i)
        {
            out_file << lines[i] << (i == lines.size() - 1 ? "" : "\n");
        }
    }
}

static void set_value_helper(YiniDocument &doc, const std::string &section_name,
                             const std::string &key, const YiniValue &value)
{
    auto *sec = doc.getOrCreateSection(section_name);
    auto it = std::find_if(sec->pairs.begin(), sec->pairs.end(),
                           [&](const auto &p) { return p.key == key; });

    if (it != sec->pairs.end())
    {
        if (std::holds_alternative<std::unique_ptr<YiniDynaValue>>(it->value.data))
        {
            auto &dyna_ptr = std::get<std::unique_ptr<YiniDynaValue>>(it->value.data);
            if (dyna_ptr)
            {
                dyna_ptr->value = value;
            }
            else
            {
                it->value = value;
            }
        }
        else
        {
            it->value = value;
        }
    }
    else
    {
        YiniKeyValuePair new_pair;
        new_pair.key = key;
        new_pair.value = value;
        sec->pairs.push_back(std::move(new_pair));
    }
}

void YiniManager::set_string_value(const std::string &section,
                                   const std::string &key,
                                   const std::string &value)
{
    YiniValue val;
    val.data = value;
    set_value_helper(document, section, key, val);
    save_document();
}

void YiniManager::set_int_value(const std::string &section,
                                const std::string &key, int value)
{
    YiniValue val;
    val.data = value;
    set_value_helper(document, section, key, val);
    save_document();
}

void YiniManager::set_double_value(const std::string &section,
                                   const std::string &key, double value)
{
    YiniValue val;
    val.data = value;
    set_value_helper(document, section, key, val);
    save_document();
}

void YiniManager::set_bool_value(const std::string &section,
                                 const std::string &key, bool value)
{
    YiniValue val;
    val.data = value;
    set_value_helper(document, section, key, val);
    save_document();
}
} // namespace YINI