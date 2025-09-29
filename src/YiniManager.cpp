#include "YINI/YiniManager.hpp"
#include "YINI/JsonDeserializer.hpp"
#include "YINI/JsonSerializer.hpp"
#include "YINI/Parser.hpp"
#include <cstdio> // For std::rename, std::remove
#include <fstream>
#include <string>

namespace YINI
{
static std::string read_file_content(const std::string &path)
{
  std::ifstream t(path);
  if (!t.is_open())
    return "";
  return std::string((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
}

static std::string get_ymeta_path(const std::string &yini_file_path)
{
  std::string ymeta_path = yini_file_path;
  size_t dotPos = ymeta_path.rfind(".yini");
  if (dotPos != std::string::npos)
  {
    ymeta_path.replace(dotPos, 5, ".ymeta");
  }
  else
  {
    ymeta_path += ".ymeta";
  }
  return ymeta_path;
}

YiniManager::YiniManager(const std::string &yini_file_path)
    : yiniFilePath(yini_file_path),
      ymetaFilePath(get_ymeta_path(yini_file_path)), is_loaded(false)
{
  is_loaded = load();
}

const YiniDocument &YiniManager::getDocument() const { return doc; }

bool YiniManager::isLoaded() const { return is_loaded; }

bool YiniManager::load()
{
  std::string ymetaContent = read_file_content(ymetaFilePath);
  if (!ymetaContent.empty())
  {
    YiniDocument tempDoc;
    if (JsonDeserializer::deserialize(ymetaContent, tempDoc))
    {
      doc = std::move(tempDoc);
      return true;
    }
  }

  std::string yiniContent = read_file_content(yiniFilePath);
  if (yiniContent.empty())
  {
    return false;
  }

  try
  {
    doc = {};
    std::string basePath = ".";
    size_t last_slash_idx = yiniFilePath.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
      basePath = yiniFilePath.substr(0, last_slash_idx);
    }

    Parser parser(yiniContent, doc, basePath);
    parser.parse();
    doc.resolveInheritance();
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
  std::string oldest_backup =
      ymetaFilePath + "." + std::to_string(max_backups);
  std::remove(oldest_backup.c_str());

  for (int i = max_backups - 1; i > 0; --i)
  {
    std::string current_backup = ymetaFilePath + "." + std::to_string(i);
    std::string next_backup = ymetaFilePath + "." + std::to_string(i + 1);
    std::rename(current_backup.c_str(), next_backup.c_str());
  }

  std::ifstream current_ymeta(ymetaFilePath.c_str());
  if (current_ymeta.good())
  {
    std::string first_backup = ymetaFilePath + ".1";
    std::rename(ymetaFilePath.c_str(), first_backup.c_str());
  }

  std::ofstream outFile(ymetaFilePath);
  if (!outFile.is_open())
  {
    return false;
  }
  std::string jsonContent = JsonSerializer::serialize(doc);
  outFile << jsonContent;
  return true;
}

static void set_value_helper(YiniDocument &document, const std::string &section_name,
                             const std::string &key, const YiniValue &value)
{
  auto *sec = document.findSection(section_name);
  if (sec)
  {
    auto it = std::find_if(sec->pairs.begin(), sec->pairs.end(),
                           [&](const auto &p) { return p.key == key; });

    if (it != sec->pairs.end() && it->is_dynamic)
    {
      it->value = value;
    }
  }
}

void YiniManager::setStringValue(const std::string &section,
                                 const std::string &key,
                                 const std::string &value)
{
  YiniValue val;
  val.data = value;
  set_value_helper(doc, section, key, val);
  save();
}

void YiniManager::setIntValue(const std::string &section,
                              const std::string &key, int value)
{
  YiniValue val;
  val.data = value;
  set_value_helper(doc, section, key, val);
  save();
}

void YiniManager::setDoubleValue(const std::string &section,
                                 const std::string &key, double value)
{
  YiniValue val;
  val.data = value;
  set_value_helper(doc, section, key, val);
  save();
}

void YiniManager::setBoolValue(const std::string &section,
                               const std::string &key, bool value)
{
  YiniValue val;
  val.data = value;
  set_value_helper(doc, section, key, val);
  save();
}
}