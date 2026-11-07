#include "AudioBootstrap.h"
#include "../api/pipe/LiveChannel.h"
#include "../image/upload_policy.h"
#include <exception>

namespace ClientMilesDevelopment {
namespace {
ClientMilesPipe::Session *session=0;
struct ReleaseModule {
    void operator()(void *module) const { FreeLibrary(static_cast<HMODULE>(module)); }
};
std::shared_ptr<void> pinModule(const char *address) {
    HMODULE module=0;
    if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,address,&module))
        ClientMilesPrivate52::fail(ClientMilesPrivate52::InvalidComposition,"development code module pin failed");
    return std::shared_ptr<void>(module,ReleaseModule());
}
}
void connectAudio(const char *host,const char *dll,const char *budgetText,
                  ClientMilesPrivate52::FatalReporter reporter) {
    try {
        ClientMilesPrivate52::bindFatalReporter(reporter);
        uint32_t budget=0;
        if(session || !host || !*host || !dll || !*dll || !MilesImage93::parseBudget(budgetText,budget))
            ClientMilesPrivate52::fail(ClientMilesPrivate52::InvalidComposition,
                "development Miles requires explicit host, original DLL, and valid upload byte budget");
        // Engine globals are borrowed under the caller's explicit lifetime contract.
        // These genuine module references protect only modern and engine callback code.
        std::shared_ptr<void> bridgeCode=pinModule(reinterpret_cast<const char *>(&connectAudio));
        std::shared_ptr<void> engineCode=pinModule(reinterpret_cast<const char *>(reporter));
        session=ClientMilesPipe::connectSession(host,dll,bridgeCode,engineCode,budget);
    } catch(const std::exception &error) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,error.what());
    } catch(...) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::UnknownException,"development Miles connection failed");
    }
}
void shutdownAndCloseAudio() {
    try {
        if(!session || !session->started || session->stopped)
            ClientMilesPrivate52::fail(ClientMilesPrivate52::InvalidComposition,
                "development close requires successful Miles startup");
        ClientMiles::shutdown();
        session->close();
        delete session;session=0;
    } catch(const std::exception &error) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,error.what());
    } catch(...) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::UnknownException,"development Miles paired close failed");
    }
}
}
