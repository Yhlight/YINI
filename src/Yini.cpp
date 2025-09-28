#include "YINI/Yini.h"
#include "YINI/Parser.hpp"

extern "C" {

struct YiniDocumentHandle
{
    YINI::YiniDocument doc;
};

YINI_API YiniDocumentHandle* yini_parse(const char* content)
{
    if (!content) return nullptr;
    auto* handle = new YiniDocumentHandle();
    YINI::Parser parser(content, handle->doc, ".");
    parser.parse();
    return handle;
}

YINI_API void yini_free_document(YiniDocumentHandle* handle)
{
    delete handle;
}

YINI_API int yini_get_section_count(YiniDocumentHandle* handle)
{
    if (!handle) return 0;
    return handle->doc.getSections().size();
}

YINI_API const char* yini_get_section_name(YiniDocumentHandle* handle, int section_index)
{
    if (!handle || section_index < 0 || section_index >= handle->doc.getSections().size())
    {
        return nullptr;
    }
    return handle->doc.getSections()[section_index].name.c_str();
}

}