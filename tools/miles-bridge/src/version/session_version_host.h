#ifndef PAIRED_VERSION90_HOST_H
#define PAIRED_VERSION90_HOST_H
#include "session_version.h"
namespace MilesSessionVersion {
// Private selected Windows module/resource operation. Zero count is not failure.
// False means invalid request or an unsupported buffer-write form; caller fails terminally.
bool queryNativeVersion(MilesTransport::Bytes query, std::vector<unsigned char> &frame);
}
#endif
