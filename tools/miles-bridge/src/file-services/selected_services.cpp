#include "selected_services.h"
#include <stdexcept>
#include <type_traits>

namespace MilesSelectedFileServices44 {
namespace {
struct Table {
    const ClientMiles::FileOpenCallback open;
    const ClientMiles::FileCloseCallback close;
    const ClientMiles::FileSeekCallback seek;
    const ClientMiles::FileReadCallback read;
    const std::shared_ptr<void> lifetime;
    Table(ClientMiles::FileOpenCallback o, ClientMiles::FileCloseCallback c,
          ClientMiles::FileSeekCallback s, ClientMiles::FileReadCallback r,
          std::shared_ptr<void> pin):open(o),close(c),seek(s),read(r),lifetime(pin) {}
private:
    Table &operator=(const Table &);
};
static_assert(std::is_same<ClientMiles::FileHandle, uintptr_t>::value,
              "local file values retain native pointer width");
const Table &table(const void *p) { return *static_cast<const Table *>(p); }
ClientAudioFileCallbacks::OpenResult open(const void *p, const char *name) {
    ClientMiles::FileHandle handle = 0;
    const uint32_t status = table(p).open(name, &handle);
    // Preserve every status bit. Zero is a valid handle when status is nonzero.
    ClientAudioFileCallbacks::OpenResult result = {status, {handle}};
    return result;
}
void close(const void *p, ClientAudioFileCallbacks::LocalFileHandle handle) {
    table(p).close(handle.value);
}
int32_t seek(const void *p, ClientAudioFileCallbacks::LocalFileHandle handle,
             int32_t offset, uint32_t origin) {
    return table(p).seek(handle.value, offset, origin);
}
uint32_t read(const void *p, ClientAudioFileCallbacks::LocalFileHandle handle,
              void *buffer, uint32_t count) {
    return table(p).read(handle.value, buffer, count);
}
}
MilesFileChannel26::FileServices retain(
    ClientMiles::FileOpenCallback o, ClientMiles::FileCloseCallback c,
    ClientMiles::FileSeekCallback s, ClientMiles::FileReadCallback r,
    std::shared_ptr<void> callbackLifetime) {
    if (!o || !c || !s || !r || !callbackLifetime)
        throw std::invalid_argument("four selected callbacks and their lifetime owner required");
    std::shared_ptr<const Table> selected(new Table(o, c, s, r, callbackLifetime));
    MilesFileChannel26::FileServices result = {selected, &open, &close, &seek, &read};
    return result;
}
}
