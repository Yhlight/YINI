#include "YINI/JsonDeserializer.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace YINI
{
// Forward declaration for recursive parsing
static bool parseJsonValue(const json &j, YiniValue &value);

static bool parseJsonArray(const json &j_array, YiniValue &value)
{
  auto yini_array = std::make_unique<YiniArray>();
  for (const auto &j_element : j_array)
  {
    YiniValue element_value;
    if (!parseJsonValue(j_element, element_value))
    {
      return false; // Propagate failure
    }
    yini_array->elements.push_back(std::move(element_value));
  }
  value.data = std::move(yini_array);
  return true;
}

static bool parseJsonList(const json &j_list, YiniValue &value)
{
  auto yini_list = std::make_unique<YiniList>();
  for (const auto &j_element : j_list)
  {
    YiniValue element_value;
    if (!parseJsonValue(j_element, element_value))
    {
      return false; // Propagate failure
    }
    yini_list->elements.push_back(std::move(element_value));
  }
  value.data = std::move(yini_list);
  return true;
}

static bool parseJsonSet(const json &j_set, YiniValue &value)
{
  auto yini_set = std::make_unique<YiniSet>();
  for (const auto &j_element : j_set)
  {
    YiniValue element_value;
    if (!parseJsonValue(j_element, element_value))
    {
      return false; // Propagate failure
    }
    yini_set->elements.insert(std::move(element_value));
  }
  value.data = std::move(yini_set);
  return true;
}

static bool parseJsonMap(const json &j_map, YiniValue &value)
{
  auto yini_map = std::make_unique<YiniMap>();
  if (!j_map.is_object())
    return false;

  for (auto it = j_map.begin(); it != j_map.end(); ++it)
  {
    YiniValue element_value;
    if (!parseJsonValue(it.value(), element_value))
    {
      return false;
    }
    yini_map->elements[it.key()] = std::move(element_value);
  }
  value.data = std::move(yini_map);
  return true;
}

static bool parseJsonDyna(const json &j_dyna, YiniValue &value)
{
  auto dyna_val = std::make_unique<YiniDynaValue>();
  if (!parseJsonValue(j_dyna, dyna_val->value))
  {
    return false;
  }
  value.data = std::move(dyna_val);
  return true;
}

static bool parseJsonCoord(const json &j_coord, YiniValue &value)
{
  auto coord = std::make_unique<YiniCoord>();
  coord->x = j_coord.value("x", 0.0);
  coord->y = j_coord.value("y", 0.0);
  coord->z = j_coord.value("z", 0.0);
  coord->is_3d = j_coord.value("is_3d", false);
  value.data = std::move(coord);
  return true;
}

static bool parseJsonColor(const json &j_color, YiniValue &value)
{
  auto color = std::make_unique<YiniColor>();
  color->r = j_color.value("r", 0);
  color->g = j_color.value("g", 0);
  color->b = j_color.value("b", 0);
  value.data = std::move(color);
  return true;
}

static bool parseJsonPath(const json &j_path, YiniValue &value)
{
  auto path = std::make_unique<YiniPath>();
  if (j_path.is_string())
  {
    path->path_value = j_path.get<std::string>();
  }
  value.data = std::move(path);
  return true;
}

static bool parseJsonValue(const json &j, YiniValue &value)
{
  if (j.is_object() && j.contains("__type__") && j.contains("value"))
  {
    const std::string type = j.at("__type__").get<std::string>();
    const json &j_val = j.at("value");

    if (type == "List")
      return parseJsonList(j_val, value);
    if (type == "Set")
      return parseJsonSet(j_val, value);
    if (type == "Map")
      return parseJsonMap(j_val, value);
    if (type == "Dyna")
      return parseJsonDyna(j_val, value);
    if (type == "Coord")
      return parseJsonCoord(j_val, value);
    if (type == "Color")
      return parseJsonColor(j_val, value);
    if (type == "Path")
      return parseJsonPath(j_val, value);
  }

  if (j.is_string())
  {
    value.data = j.get<std::string>();
  }
  else if (j.is_number_integer())
  {
    value.data = j.get<int>();
  }
  else if (j.is_number_float())
  {
    value.data = j.get<double>();
  }
  else if (j.is_boolean())
  {
    value.data = j.get<bool>();
  }
  else if (j.is_array())
  {
    return parseJsonArray(j, value);
  }
  else if (j.is_null())
  {
    // Do nothing, default YiniValue is fine
  }
  else
  {
    // Could be an object that's not a special type, treat as map
    if (j.is_object())
    {
      return parseJsonMap(j, value);
    }
  }
  return true;
}

bool JsonDeserializer::deserialize(const std::string &json_content,
                                   YiniDocument &doc)
{
  try
  {
    json j = json::parse(json_content);

    if (!j.is_object())
      return false;

    // Deserialize Defines
    if (j.contains("defines") && j.at("defines").is_object())
    {
      for (auto it = j.at("defines").begin(); it != j.at("defines").end(); ++it)
      {
        YiniValue val;
        if (parseJsonValue(it.value(), val))
        {
          doc.addDefine(it.key(), val);
        }
      }
    }

    // Deserialize Sections
    if (j.contains("sections") && j.at("sections").is_object())
    {
      for (auto it = j.at("sections").begin(); it != j.at("sections").end();
           ++it)
      {
        const std::string &section_name = it.key();
        const json &section_json = it.value();
        if (!section_json.is_object())
          continue;

        YiniSection *section = doc.getOrCreateSection(section_name);

        // Inherits
        if (section_json.contains("inherits") &&
            section_json.at("inherits").is_array())
        {
          for (const auto &inherit_json : section_json.at("inherits"))
          {
            if (inherit_json.is_string())
            {
              section->inheritedSections.push_back(
                  inherit_json.get<std::string>());
            }
          }
        }

        // Pairs
        if (section_json.contains("pairs") &&
            section_json.at("pairs").is_object())
        {
          for (auto pair_it = section_json.at("pairs").begin();
               pair_it != section_json.at("pairs").end(); ++pair_it)
          {
            YiniKeyValuePair pair;
            pair.key = pair_it.key();
            if (parseJsonValue(pair_it.value(), pair.value))
            {
              section->pairs.push_back(std::move(pair));
            }
          }
        }

        // Register
        if (section_json.contains("register") &&
            section_json.at("register").is_array())
        {
          for (const auto &reg_json : section_json.at("register"))
          {
            YiniValue val;
            if (parseJsonValue(reg_json, val))
            {
              section->registrationList.push_back(std::move(val));
            }
          }
        }
      }
    }
  }
  catch (const json::parse_error &e)
  {
    // In a real application, log the error e.what()
    return false;
  }

  return true;
}
} // namespace YINI