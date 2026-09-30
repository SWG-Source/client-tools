#ifndef CLIENT_MILES_PRIVATE_LIVE_CHANNEL_H
#define CLIENT_MILES_PRIVATE_LIVE_CHANNEL_H
#include "Channel.h"
#include <memory>
#include "Session.h"

namespace ClientMilesPipe {
// Composition root only. Creates the existing private pipes/process and performs
// Hello. Public ClientMiles.h has no process configuration or bootstrap calls.
// Private root only. Runtime remains retained; Session destruction is terminal in59.
Session *connectSession(const char *hostExecutable,const char *originalDll,
    std::shared_ptr<void> engineLifetime,std::shared_ptr<void> callbackCodeLifetime,
    uint32_t uploadBudgetBytes);
} // namespace ClientMilesPipe
#endif
