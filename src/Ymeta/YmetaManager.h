#pragma once

#include "YiniTypes.h"
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
    YiniVariant get_value(const std::string &key);
    void set_value(const std::string &key, YiniVariant value);

  private:
    std::string get_ymeta_path(const std::string &yini_filepath);

    std::map<std::string, YiniVariant> m_dynamic_values;
    std::map<std::string, std::vector<YiniVariant>> m_backup_values;
};

} // namespace YINI
