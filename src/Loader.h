#ifndef YINI_LOADER_H
#define YINI_LOADER_H

#include "Parser/AST.h"
#include <string>
#include <vector>
#include <set>

namespace YINI
{

class Loader
{
public:
    Loader();
    Document load(const std::string& filepath, bool use_cache = true);
    void save_ymeta(const std::string& filepath, const Document& doc);


private:
    Document loadRecursive(const std::string& filepath, std::set<std::string>& visited_files);
    void mergeDocuments(Document& base, Document& to_merge);

    // Caching methods
    Document load_from_ymeta(const std::string& filepath);
};

} // namespace YINI

#endif // YINI_LOADER_H
