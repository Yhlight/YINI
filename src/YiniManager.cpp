#include "YINI/YiniManager.hpp"
#include "YINI/JsonDeserializer.hpp"
#include "YINI/JsonSerializer.hpp"
#include "YINI/Parser.hpp"
#include "YiniValueToString.hpp"
#include <cstdio>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace YINI
{
namespace fs = std::filesystem;

// Helper to read file content
static std::string read_file_content(const std::string &path)
{
  std::ifstream t(path);
  if (!t.is_open())
    return "";
  return std::string((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
}

static std::string get_ymeta_path(const std::string &yiniFilePath)
{
  std::string ymetaPath = yiniFilePath;
  size_t dotPos = ymetaPath.rfind(".yini");
  if (dotPos != std::string::npos)
  {
    ymetaPath.replace(dotPos, 5, ".ymeta");
  }
  else
  {
    ymetaPath += ".ymeta";
  }
  return ymetaPath;
}

YiniManager::YiniManager(const std::string &path)
    : yiniFilePath(path),
      ymetaFilePath(get_ymeta_path(path)), is_loaded(false)
{
  std::lock_guard<std::mutex> lock(managerMutex);
  is_loaded = load();
}

YiniManager::~YiniManager()
{
  std::lock_guard<std::mutex> lock(managerMutex);
  if (is_loaded)
  {
    writeBackDynaValues();
  }
}

YiniDocument YiniManager::getDocument() const
{
  std::lock_guard<std::mutex> lock(managerMutex);
  return document;
}

bool YiniManager::isLoaded() const
{
  std::lock_guard<std::mutex> lock(managerMutex);
  return is_loaded;
}

bool YiniManager::load()
{
  if (fs::exists(ymetaFilePath) && fs::exists(yiniFilePath))
  {
    auto ymeta_time = fs::last_write_time(ymetaFilePath);
    auto yini_time = fs::last_write_time(yiniFilePath);

    if (ymeta_time >= yini_time)
    {
      std::string ymetaContent = read_file_content(ymetaFilePath);
      if (!ymetaContent.empty())
      {
        if (JsonDeserializer::deserialize(ymetaContent, document))
        {
          return true;
        }
      }
    }
  }

  std::string yiniContent = read_file_content(yiniFilePath);
  if (yiniContent.empty())
  {
    if (fs::exists(ymetaFilePath)) {
        std::string ymetaContent = read_file_content(ymetaFilePath);
        if (!ymetaContent.empty() && JsonDeserializer::deserialize(ymetaContent, document)) {
            return true;
        }
    }
    return false;
  }

  try
  {
    document = YiniDocument{};
    std::string basePath = ".";
    size_t last_slash_idx = yiniFilePath.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
      basePath = yiniFilePath.substr(0, last_slash_idx);
    }

    Parser parser(yiniContent, document, basePath);
    parser.parse();
    document.resolveInheritance();
  }
  catch (...)
  {
    return false;
  }

  return save();
}

bool YiniManager::save()
{
  const int max_backups = 5;
  std::string oldest_backup = ymetaFilePath + ".bak" + std::to_string(max_backups);
  if (fs::exists(oldest_backup))
  {
      fs::remove(oldest_backup);
  }

  for (int i = max_backups - 1; i >= 1; --i)
  {
    std::string current_backup = ymetaFilePath + ".bak" + std::to_string(i);
    std::string next_backup = ymetaFilePath + ".bak" + std::to_string(i + 1);
    if(fs::exists(current_backup))
    {
        fs::rename(current_backup, next_backup);
    }
  }

  if (fs::exists(ymetaFilePath))
  {
    std::string first_backup = ymetaFilePath + ".bak1";
    fs::rename(ymetaFilePath, first_backup);
  }

  std::ofstream outFile(ymetaFilePath);
  if (!outFile.is_open())
  {
    return false;
  }
  std::string jsonContent = JsonSerializer::serialize(document);
  outFile << jsonContent;
  return true;
}

void YiniManager::writeBackDynaValues()
{
    std::ifstream inFile(yiniFilePath);
    if (!inFile.is_open()) return;

    std::stringstream new_content_ss;
    std::string line;
    YiniSection* currentSection = nullptr;

    while (std::getline(inFile, line))
    {
        std::string trimmed_line = line;
        trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t\n\r"));
        trimmed_line.erase(trimmed_line.find_last_not_of(" \t\n\r") + 1);

        if (trimmed_line.empty() || trimmed_line[0] == '/' || trimmed_line[0] == ';') {
            new_content_ss << line << '\n';
            continue;
        }

        if (trimmed_line.front() == '[' && trimmed_line.back() == ']') {
            std::string sectionName = trimmed_line.substr(1, trimmed_line.length() - 2);
            size_t colon_pos = sectionName.find(':');
            if (colon_pos != std::string::npos) {
                sectionName = sectionName.substr(0, colon_pos);
                sectionName.erase(sectionName.find_last_not_of(" \t") + 1);
            }
            currentSection = document.getOrCreateSection(sectionName);
            new_content_ss << line << '\n';
            continue;
        }

        size_t equals_pos = line.find('=');
        if (equals_pos != std::string::npos) {
            std::string key = line.substr(0, equals_pos);
            key.erase(key.find_last_not_of(" \t") + 1);
            key.erase(0, key.find_first_not_of(" \t"));

            if (currentSection) {
                 auto it = std::find_if(currentSection->pairs.begin(), currentSection->pairs.end(),
                                   [&](const auto& p) { return p.key == key; });

                if (it != currentSection->pairs.end() && std::holds_alternative<std::unique_ptr<YiniDynaValue>>(it->value.data)) {
                    std::string new_value_str = valueToString(it->value);
                    new_content_ss << key << " = " << new_value_str << '\n';
                    continue;
                }
            }
        }

        new_content_ss << line << '\n';
    }
    inFile.close();

    std::ofstream outFile(yiniFilePath);
    outFile << new_content_ss.str();
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
        auto& dyna_ptr = std::get<std::unique_ptr<YiniDynaValue>>(it->value.data);
        if (dyna_ptr) {
            // Assign the new primitive value to the inner value of the Dyna object.
            // This uses YiniValue's `operator=` which correctly handles deep copying.
            dyna_ptr->value = value;
        } else {
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

void YiniManager::setStringValue(const std::string &section,
                                 const std::string &key,
                                 const std::string &value)
{
  std::lock_guard<std::mutex> lock(managerMutex);
  YiniValue val;
  val.data = value;
  set_value_helper(document, section, key, val);
  save();
}

void YiniManager::setIntValue(const std::string &section,
                              const std::string &key, int value)
{
  std::lock_guard<std::mutex> lock(managerMutex);
  YiniValue val;
  val.data = value;
  set_value_helper(document, section, key, val);
  save();
}

void YiniManager::setDoubleValue(const std::string &section,
                                 const std::string &key, double value)
{
  std::lock_guard<std::mutex> lock(managerMutex);
  YiniValue val;
  val.data = value;
  set_value_helper(document, section, key, val);
  save();
}

void YiniManager::setBoolValue(const std::string &section,
                               const std::string &key, bool value)
{
  std::lock_guard<std::mutex> lock(managerMutex);
  YiniValue val;
  val.data = value;
  set_value_helper(document, section, key, val);
  save();
}
} // namespace YINI