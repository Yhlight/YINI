#include "YINI/YiniManager.hpp"
#include "YINI/JsonDeserializer.hpp"
#include "YINI/JsonSerializer.hpp"
#include "YINI/YiniSerializer.hpp"
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
    : yini_file_path(yini_file_path),
      ymeta_file_path(get_ymeta_path(yini_file_path)),
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
    if (fs::exists(ymeta_file_path) && fs::exists(yini_file_path))
    {
        auto ymeta_time = fs::last_write_time(ymeta_file_path);
        auto yini_time = fs::last_write_time(yini_file_path);

        if (ymeta_time >= yini_time)
        {
            std::string ymeta_content = read_file_content(ymeta_file_path);
            if (!ymeta_content.empty())
            {
                if (JsonDeserializer::deserialize(ymeta_content, document))
                {
                    return true;
                }
            }
        }
    }

    std::string yini_content = read_file_content(yini_file_path);
    if (yini_content.empty())
    {
        if (fs::exists(ymeta_file_path))
        {
            std::string ymeta_content = read_file_content(ymeta_file_path);
            if (!ymeta_content.empty() && JsonDeserializer::deserialize(ymeta_content, document))
            {
                return true;
            }
        }
        return false;
    }

    try
    {
        document = {};
        std::string base_path = ".";
        size_t last_slash_idx = yini_file_path.rfind('/');
        if (std::string::npos != last_slash_idx)
        {
            base_path = yini_file_path.substr(0, last_slash_idx);
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

    return save_document();
}

bool YiniManager::save_document()
{
    const int max_backups = 5;
    std::string oldest_backup = ymeta_file_path + ".bak" + std::to_string(max_backups);
    if (fs::exists(oldest_backup))
    {
        fs::remove(oldest_backup);
    }

    for (int i = max_backups - 1; i >= 1; --i)
    {
        std::string current_backup = ymeta_file_path + ".bak" + std::to_string(i);
        std::string next_backup = ymeta_file_path + ".bak" + std::to_string(i + 1);
        if (fs::exists(current_backup))
        {
            fs::rename(current_backup, next_backup);
        }
    }

    if (fs::exists(ymeta_file_path))
    {
        std::string first_backup = ymeta_file_path + ".bak1";
        fs::rename(ymeta_file_path, first_backup);
    }

    std::ofstream out_file(ymeta_file_path);
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
    // This function will now serialize the entire document back to the .yini file.
    // This is more robust than the previous line-by-line approach.
    std::string serialized_content = YiniSerializer::serialize(document);
    std::ofstream out_file(yini_file_path);
    if (out_file.is_open())
    {
        out_file << serialized_content;
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
        // If the existing value is a DynaValue, update its inner value instead of replacing it.
        if (std::holds_alternative<std::unique_ptr<YiniDynaValue>>(it->value.data))
        {
            auto &dyna_ptr = std::get<std::unique_ptr<YiniDynaValue>>(it->value.data);
            if (dyna_ptr)
            {
                // Assign the new primitive value to the inner value of the Dyna object.
                // This uses YiniValue's `operator=` which correctly handles deep copying.
                dyna_ptr->value = value;
            }
            else
            {
                it->value = value; // Fallback for safety
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