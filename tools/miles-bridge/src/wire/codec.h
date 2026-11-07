#ifndef MILES_TRANSPORT_CODEC_H
#define MILES_TRANSPORT_CODEC_H
#include "resource_registry.h"
namespace MilesTransport {
struct Bytes {
    const unsigned char *data;
    size_t size;
    Bytes(const void *p = 0, size_t n = 0) : data(static_cast<const unsigned char *>(p)), size(n) {}
};
bool encodeCall(MilesWire::Header, const MilesWire::Call &, Bytes, Bytes,
                std::vector<unsigned char> &);
// No allocation; rejection leaves buffer/written unchanged. Payload must not alias buffer.
bool encodeCallInto(MilesWire::Header, const MilesWire::Call &, Bytes, Bytes,
                    unsigned char *buffer, size_t capacity, size_t &written);
bool decodeCall(Bytes, MilesWire::Header &, MilesWire::Call &);
bool encodeResult(MilesWire::Header, const MilesWire::Result &, Bytes, Bytes,
                  std::vector<unsigned char> &);
// No allocation. Buffer must be pre-sized; payload must not alias its storage.
// On rejection leaves buffer and written unchanged.
bool encodeResultInto(MilesWire::Header, const MilesWire::Result &, Bytes, Bytes,
                      unsigned char *buffer, size_t capacity, size_t &written);
bool decodeResult(Bytes, MilesWire::Header &, MilesWire::Result &);
bool encodeEos(MilesWire::Header, const MilesWire::Eos &, std::vector<unsigned char> &);
bool decodeEos(Bytes, MilesWire::Header &, MilesWire::Eos &);
} // namespace MilesTransport
#endif
