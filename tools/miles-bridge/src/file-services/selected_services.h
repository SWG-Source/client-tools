#ifndef MILES_SELECTED_FILE_SERVICES44_H
#define MILES_SELECTED_FILE_SERVICES44_H
#include "../file-callbacks/ClientMilesFileCallbacks.h"
#include "../file-channel/file_channel.h"

namespace MilesSelectedFileServices44 {
// Private composition helper, not registration or a public callback ABI.
// A nonnull pin must own callback code/state through the last invocation and
// session teardown. It does not prove TLS readiness or vendor quiescence.
// Null-table/default-file and table replacement semantics remain unsupported.
MilesFileChannel26::FileServices retain(
    ClientMiles::FileOpenCallback open, ClientMiles::FileCloseCallback close,
    ClientMiles::FileSeekCallback seek, ClientMiles::FileReadCallback read,
    std::shared_ptr<void> callbackLifetime);
}
#endif
