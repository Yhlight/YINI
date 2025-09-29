#include "YINI/JsonSerializer.hpp"
#include <iomanip>
#include <sstream>
#include <variant>

namespace
{ // Anonymous namespace for helpers

void serializeValue(std::stringstream &ss, const YINI::YiniValue &value);

void serializeArray(std::stringstream &ss, const YINI::YiniArray &array)
{
  ss << "[";
  for (size_t i = 0; i < array.elements.size(); ++i)
  {
    serializeValue(ss, array.elements[i]);
    if (i < array.elements.size() - 1)
    {
      ss << ",";
    }
  }
  ss << "]";
}

void serializeMap(std::stringstream &ss, const YINI::YiniMap &map)
{
  ss << "{";
  size_t count = 0;
  for (const auto &[key, value] : map.elements)
  {
    ss << "\"" << key << "\":";
    serializeValue(ss, value);
    if (count < map.elements.size() - 1)
    {
      ss << ",";
    }
    count++;
  }
  ss << "}";
}

void serializeValue(std::stringstream &ss, const YINI::YiniValue &value)
{
  if (std::holds_alternative<std::string>(value.data))
  {
    ss << std::quoted(std::get<std::string>(value.data));
  }
  else if (std::holds_alternative<int>(value.data))
  {
    ss << std::get<int>(value.data);
  }
  else if (std::holds_alternative<double>(value.data))
  {
    ss << std::get<double>(value.data);
  }
  else if (std::holds_alternative<bool>(value.data))
  {
    ss << (std::get<bool>(value.data) ? "true" : "false");
  }
  else if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value.data))
  {
    const auto &arr_ptr =
        std::get<std::unique_ptr<YINI::YiniArray>>(value.data);
    if (arr_ptr)
    {
      serializeArray(ss, *arr_ptr);
    }
    else
    {
      ss << "null";
    }
  }
  else if (std::holds_alternative<std::unique_ptr<YINI::YiniList>>(value.data))
  {
    const auto &list_ptr =
        std::get<std::unique_ptr<YINI::YiniList>>(value.data);
    ss << "{\"__type__\":\"List\",\"value\":";
    if (list_ptr)
    {
      // Since YiniList and YiniArray have the same structure, we can reuse serializeArray
      // by casting. This is a bit of a hack but avoids code duplication.
      serializeArray(ss, *reinterpret_cast<const YINI::YiniArray *>(list_ptr.get()));
    }
    else
    {
      ss << "null";
    }
    ss << "}";
  }
  else if (std::holds_alternative<std::unique_ptr<YINI::YiniSet>>(value.data))
  {
    const auto &set_ptr =
        std::get<std::unique_ptr<YINI::YiniSet>>(value.data);
    ss << "{\"__type__\":\"Set\",\"value\":";
    if (set_ptr)
    {
      serializeArray(ss, *reinterpret_cast<const YINI::YiniArray *>(set_ptr.get()));
    }
    else
    {
      ss << "null";
    }
    ss << "}";
  }
  else if (std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value.data))
  {
    const auto &map_ptr = std::get<std::unique_ptr<YINI::YiniMap>>(value.data);
    ss << "{\"__type__\":\"Map\",\"value\":";
    if (map_ptr)
    {
      serializeMap(ss, *map_ptr);
    }
    else
    {
      ss << "null";
    }
    ss << "}";
  }
  else if (std::holds_alternative<std::unique_ptr<YINI::YiniDynaValue>>(
               value.data))
  {
    const auto &dyna_ptr =
        std::get<std::unique_ptr<YINI::YiniDynaValue>>(value.data);
    ss << "{\"__type__\":\"Dyna\",\"value\":";
    if (dyna_ptr)
    {
      serializeValue(ss, dyna_ptr->value);
    }
    else
    {
      ss << "null";
    }
    ss << "}";
  }
  else if (std::holds_alternative<std::unique_ptr<YINI::YiniCoord>>(value.data))
  {
    const auto &ptr = std::get<std::unique_ptr<YINI::YiniCoord>>(value.data);
    ss << "{\"__type__\":\"Coord\",\"value\":";
    if (ptr)
    {
      ss << "{\"x\":" << ptr->x << ",\"y\":" << ptr->y << ",\"z\":" << ptr->z
         << ",\"is_3d\":" << (ptr->is_3d ? "true" : "false") << "}";
    }
    else
    {
      ss << "null";
    }
    ss << "}";
  }
  else if (std::holds_alternative<std::unique_ptr<YINI::YiniColor>>(value.data))
  {
    const auto &ptr = std::get<std::unique_ptr<YINI::YiniColor>>(value.data);
    ss << "{\"__type__\":\"Color\",\"value\":";
    if (ptr)
    {
      ss << "{\"r\":" << (int)ptr->r << ",\"g\":" << (int)ptr->g
         << ",\"b\":" << (int)ptr->b << "}";
    }
    else
    {
      ss << "null";
    }
    ss << "}";
  }
  else if (std::holds_alternative<std::unique_ptr<YINI::YiniPath>>(value.data))
  {
    const auto &ptr = std::get<std::unique_ptr<YINI::YiniPath>>(value.data);
    ss << "{\"__type__\":\"Path\",\"value\":";
    if (ptr)
    {
      ss << std::quoted(ptr->pathValue);
    }
    else
    {
      ss << "null";
    }
    ss << "}";
  }
}

} // end anonymous namespace

namespace YINI
{
std::string JsonSerializer::serialize(const YiniDocument &doc)
{
  std::stringstream ss;
  ss << "{";

  // Serialize Defines
  ss << "\"defines\":{";
  size_t define_count = 0;
  const auto &defines = doc.getDefines();
  for (const auto &[key, value] : defines)
  {
    ss << "\"" << key << "\":";
    serializeValue(ss, value);
    if (define_count < defines.size() - 1)
    {
      ss << ",";
    }
    define_count++;
  }
  ss << "},";

  // Serialize Sections
  ss << "\"sections\":{";
  const auto &sections = doc.getSections();
  for (size_t i = 0; i < sections.size(); ++i)
  {
    const auto &section = sections[i];
    ss << "\"" << section.name << "\":{";

    // Inherited Sections
    ss << "\"inherits\":[";
    for (size_t j = 0; j < section.inheritedSections.size(); ++j)
    {
      ss << "\"" << section.inheritedSections[j] << "\"";
      if (j < section.inheritedSections.size() - 1)
      {
        ss << ",";
      }
    }
    ss << "],";

    // Key-Value Pairs
    ss << "\"pairs\":{";
    for (size_t j = 0; j < section.pairs.size(); ++j)
    {
      const auto &pair = section.pairs[j];
      ss << "\"" << pair.key << "\":";
      serializeValue(ss, pair.value);
      if (j < section.pairs.size() - 1)
      {
        ss << ",";
      }
    }
    ss << "},";

    // Registration List
    ss << "\"register\":[";
    for (size_t j = 0; j < section.registrationList.size(); ++j)
    {
      serializeValue(ss, section.registrationList[j]);
      if (j < section.registrationList.size() - 1)
      {
        ss << ",";
      }
    }
    ss << "]";

    ss << "}"; // End of section object
    if (i < sections.size() - 1)
    {
      ss << ",";
    }
  }
  ss << "}"; // End of sections object

  ss << "}"; // End of root object
  return ss.str();
}
} // namespace YINI