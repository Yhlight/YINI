#pragma once

#include <any>
#include <map>
#include <string>
#include <vector>

namespace YINI
{

class YmetaManager
{
  public:
    YmetaManager();

    void load(const std::string &yini_filepath);
    void save(const std::string &yini_filepath);

    bool has_value(const std::string &key);
    std::any get_value(const std::string &key);
    void set_value(const std::string &key, std::any value);

  private:
    std::string get_ymeta_path(const std::string &yini_filepath);

    std::map<std::string, std::any> m_dynamic_values;
    std::map<std::string, std::vector<std::any>> m_backup_values;
};

} // namespace YINI