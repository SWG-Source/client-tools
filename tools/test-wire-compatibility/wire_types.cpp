// Compile-time wire-width oracle: fields serialized with Archive::put/get must be 4 bytes on
// every ABI, because the stock server (-m32) and existing Win32 clients read 4 bytes.
#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"
#include "sharedNetworkMessages/BuffBuilderChangeMessage.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"
#include <utility>
static_assert(sizeof(std::declval<ImageDesignChangeMessage const&>().getStartingTime())==4, "ImageDesignChangeMessage startingTime must be 4 bytes on the wire");
static_assert(sizeof(std::declval<BuffBuilderChangeMessage const&>().getStartingTime())==4, "BuffBuilderChangeMessage startingTime must be 4 bytes on the wire");
static_assert(sizeof(ChatLogEntry().m_time)==4, "ChatLogEntry m_time must be 4 bytes on the wire");
