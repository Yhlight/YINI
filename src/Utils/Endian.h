#pragma once
#include <cstdint>
#include <algorithm>

// Define macros for endianness detection
#if defined(__linux__) || defined(__APPLE__)
    #include <endian.h>
    #define YINI_LITTLE_ENDIAN __LITTLE_ENDIAN
    #define YINI_BIG_ENDIAN    __BIG_ENDIAN
    #define YINI_BYTE_ORDER    __BYTE_ORDER
#elif defined(_WIN32)
    #define YINI_LITTLE_ENDIAN 1234
    #define YINI_BIG_ENDIAN    4321
    #define YINI_BYTE_ORDER    YINI_LITTLE_ENDIAN // Windows is always little-endian
#else
    #warning "Unsupported platform: Endianness detection might not work."
    // Fallback for other platforms
    #if __has_include(<endian.h>)
        #include <endian.h>
        #define YINI_LITTLE_ENDIAN __LITTLE_ENDIAN
        #define YINI_BIG_ENDIAN    __BIG_ENDIAN
        #define YINI_BYTE_ORDER    __BYTE_ORDER
    #else
        // A simple compile-time check
        #if ((*(uint16_t *)"\0\1") >> 8) == 0
            #define YINI_BYTE_ORDER YINI_LITTLE_ENDIAN
        #else
            #define YINI_BYTE_ORDER YINI_BIG_ENDIAN
        #endif
    #endif
#endif

namespace YINI
{
namespace Utils
{
    // --- Byte Swap Functions ---
    // Use built-ins if available for performance
    #if defined(__GNUC__) || defined(__clang__)
        inline uint16_t bswap16(uint16_t x) { return __builtin_bswap16(x); }
        inline uint32_t bswap32(uint32_t x) { return __builtin_bswap32(x); }
        inline uint64_t bswap64(uint64_t x) { return __builtin_bswap64(x); }
    #elif defined(_MSC_VER)
        #include <stdlib.h>
        inline uint16_t bswap16(uint16_t x) { return _byteswap_ushort(x); }
        inline uint32_t bswap32(uint32_t x) { return _byteswap_ulong(x); }
        inline uint64_t bswap64(uint64_t x) { return _byteswap_uint64(x); }
    #else
        // Generic fallback implementation
        inline uint16_t bswap16(uint16_t x) {
            return (x >> 8) | (x << 8);
        }
        inline uint32_t bswap32(uint32_t x) {
            return (bswap16(x) << 16) | bswap16(x >> 16);
        }
        inline uint64_t bswap64(uint64_t x) {
            return ((uint64_t)bswap32(x) << 32) | bswap32(x >> 32);
        }
    #endif

    // --- Host to Little-Endian ---
    #if YINI_BYTE_ORDER == YINI_LITTLE_ENDIAN
        inline uint16_t htole16(uint16_t x) { return x; }
        inline uint32_t htole32(uint32_t x) { return x; }
        inline uint64_t htole64(uint64_t x) { return x; }
    #else // Big Endian
        inline uint16_t htole16(uint16_t x) { return bswap16(x); }
        inline uint32_t htole32(uint32_t x) { return bswap32(x); }
        inline uint64_t htole64(uint64_t x) { return bswap64(x); }
    #endif

    // --- Little-Endian to Host ---
    // The le*toh functions are the same operation as htole*.
    // We use macros to create aliases to avoid compiler redefinition errors
    // with identical inline functions.
    #ifdef le16toh
        #undef le16toh
    #endif
    #define le16toh htole16
    #ifdef le32toh
        #undef le32toh
    #endif
    #define le32toh htole32
    #ifdef le64toh
        #undef le64toh
    #endif
    #define le64toh htole64

} // namespace Utils
} // namespace YINI
