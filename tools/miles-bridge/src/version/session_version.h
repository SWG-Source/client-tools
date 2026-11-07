#ifndef PAIRED_VERSION90_H
#define PAIRED_VERSION90_H
#include "../wire/codec.h"
namespace MilesSessionVersion {
// Existing reply payload budget, not a new native API restriction.
enum { CapacityLimit = MilesWire::MaxFrameBytes - 48 - 80 };
bool validCapacity(uint32_t capacity);
bool validateQuery(MilesTransport::Bytes frame, MilesWire::Header &request, uint32_t &capacity);
bool makeReply(const MilesWire::Header &request, MilesTransport::Bytes prefix,
               std::vector<unsigned char> &frame);
}
#endif
