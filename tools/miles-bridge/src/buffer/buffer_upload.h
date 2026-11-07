#ifndef MILES_BUFFER_UPLOAD_CANDIDATE_H
#define MILES_BUFFER_UPLOAD_CANDIDATE_H
#include <stdint.h>
#include <stddef.h>
#include <vector>
namespace MilesHost {
// Dispatch-thread owned. Session reserves aggregate budget and owns registry entry.
// Upload never lends a pointer to vendor code; publish an independent owned copy.
class BufferUpload {
public:
    BufferUpload(uint32_t declaredBytes,uint32_t authorizedLimit);
    bool append(uint32_t offset,const void *source,size_t count);
    bool seal();
    bool copySealed(std::vector<unsigned char> &out) const;
    uint32_t received() const { return filled; }
    uint32_t size() const { return static_cast<uint32_t>(storage.size()); }
    bool sealed() const { return immutable; }
private:
    std::vector<unsigned char> storage;
    uint32_t filled;
    bool immutable;
    BufferUpload(const BufferUpload &);
    BufferUpload &operator=(const BufferUpload &);
};
}
#endif
