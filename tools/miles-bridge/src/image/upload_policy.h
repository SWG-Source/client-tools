#ifndef MILES_PRIVATE_UPLOAD_POLICY93_H
#define MILES_PRIVATE_UPLOAD_POLICY93_H
#include "../protocol/miles_wire.h"
namespace MilesImage93 {
// Explicit bootstrap policy, not a native API limit. Two independent N-byte
// owners: unchanged BufferUpload plus its sealed copy. Frames are separate.
inline bool validBudget(uint32_t bytes) { return bytes >= 2; }
inline bool allows(uint32_t budget, uint32_t imageBytes) {
    return validBudget(budget) && imageBytes && imageBytes <= budget / 2 &&
        imageBytes <= 0x7fffffffu; // original Win32 contiguous signed pointer extent
}
inline bool parseBudget(const char *text, uint32_t &out) {
    if (!text || !*text) return false;
    uint32_t value = 0;
    for (const char *p = text; *p; ++p) {
        if (*p < '0' || *p > '9') return false;
        const uint32_t digit = static_cast<uint32_t>(*p - '0');
        if (value > (UINT32_MAX - digit) / 10) return false;
        value = value * 10 + digit;
    }
    if (!validBudget(value)) return false;
    out = value;
    return true;
}
enum { ChunkBytes = MilesWire::MaxFrameBytes - 136 };
}
#endif
