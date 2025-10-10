#pragma once

#include <cstdint>
#include <any>

namespace YINI
{

struct ResolvedColor
{
    uint8_t r, g, b;
};

struct ResolvedCoord
{
    std::any x, y, z;
};

} // namespace YINI