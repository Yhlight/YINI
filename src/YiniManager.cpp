#include "YINI/YiniManager.hpp"
#include "YINI/JsonDeserializer.hpp"
#include "YINI/JsonSerializer.hpp"
#include "YINI/Parser.hpp"
#include <cstdio> // For std::rename, std::remove
#include <fstream>
#include <string>
#include <filesystem>

namespace YINI
{
namespace fs = std::filesystem;

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
  is_loaded = load();
}

const YiniDocument &YiniManager::getDocument() const { return document; }

bool YiniManager::isLoaded() const { return is_loaded; }

bool YiniManager::load()
{
  // Staleness Check: Only load from .ymeta if it exists and is newer than .yini
  if (fs::exists(ymetaFilePath) && fs::exists(yiniFilePath))
  {
    auto ymeta_time = fs::last_write_time(ymetaFilePath);
    auto yini_time = fs::last_write_time(yiniFilePath);

    if (ymeta_time >= yini_time)
    {
      std::string ymetaContent = read_file_content(ymetaFilePath);
      if (!ymetaContent.empty())
      {
        YiniDocument tempDoc;
        if (JsonDeserializer::deserialize(ymetaContent, tempDoc))
        {
          document = std::move(tempDoc);
          return true;
        }
      }
    }
  }

  // If ymeta is stale or doesn't exist, parse the .yini file
  std::string yiniContent = read_file_content(yiniFilePath);
  if (yiniContent.empty())
  {
    // As a last resort, if .yini is missing but .ymeta exists, load from cache
    if (fs::exists(ymetaFilePath)) {
        std::string ymetaContent = read_file_content(ymetaFilePath);
        if (!ymetaContent.empty()) {
            if (JsonDeserializer::deserialize(ymetaContent, document)) {
                return true;
            }
        }
    }
    return false;
  }

  try
  {
    document = {};
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

  return save(); // Create the .ymeta cache after a successful parse
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

static void set_value_helper(YiniDocument &doc, const std::string &section_name,
                             const std::string &key, const YiniValue &value)
{
  auto *sec = doc.getOrCreateSection(section_name);
  auto it = std::find_if(sec->pairs.begin(), sec->pairs.end(),
                         [&](const auto &p) { return p.key == key; });
  if (it != sec->pairs.end())
  {
    it->value = value;
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
  YiniValue val;
  val.data = value;
  set_value_helper(document, section, key, val);
  save();
}

void YiniManager::setIntValue(const std::string &section,
                              const std::string &key, int value)
{
  YiniValue val;
  val.data = value;
  set_value_helper(document, section, key, val);
  save();
}

void YiniManager::setDoubleValue(const std::string &section,
                                 const std::string &key, double value)
{
  YiniValue val;
  val.data = value;
  set_value_helper(document, section, key, val);
  save();
}

void YiniManager::setBoolValue(const std::string &section,
                               const std::string &key, bool value)
{
  YiniValue val;
  val.data = value;
  set_value_helper(document, section, key, val);
  save();
}
} // namespace YINI