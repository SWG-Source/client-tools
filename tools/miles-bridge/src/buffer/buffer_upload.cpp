#include "buffer_upload.h"
#include <cstring>
#include <stdexcept>
namespace MilesHost {
BufferUpload::BufferUpload(uint32_t declaredBytes,uint32_t authorizedLimit)
    :filled(0),immutable(false) {
    if(declaredBytes>authorizedLimit)throw std::out_of_range("upload exceeds authorized byte limit");
    storage.resize(declaredBytes);
}
bool BufferUpload::append(uint32_t offset,const void *source,size_t count) {
    if(immutable || offset!=filled || !source || !count || count>storage.size()-filled)return false;
    std::memcpy(&storage[filled],source,count);
    filled+=static_cast<uint32_t>(count); // Bounded by uint32 declared size above.
    return true;
}
bool BufferUpload::seal() {
    if(filled!=storage.size())return false;
    immutable=true;
    return true;
}
bool BufferUpload::copySealed(std::vector<unsigned char> &out) const {
    if(!immutable)return false;
    std::vector<unsigned char> copy(storage);
    out.swap(copy);
    return true;
}
}
