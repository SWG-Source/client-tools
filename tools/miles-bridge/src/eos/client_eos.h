#ifndef MILES_CLIENT_EOS_JOB_H
#define MILES_CLIENT_EOS_JOB_H
#include "../api/ClientMiles.h"
#include "../file-executor/EngineFileWorker.h"
#include <memory>
namespace MilesEos {
// Modern-runtime owner; engine queue receives only an opaque context/function.
// Exactly one typed callback is captured before submission. It cannot be changed
// by a later registration while this event is pending.
class ClientJob {
public:
    static std::shared_ptr<ClientJob> submit(
        MilesFileExecutor30::EngineFileWorker &, ClientMiles::HSAMPLE,
        ClientMiles::SampleCallback, ClientMiles::HSTREAM,
        ClientMiles::StreamCallback, std::shared_ptr<void> codeLifetime);
    enum Status { Pending, Completed, Failed };
    Status status() const;
    ~ClientJob();
private:
    struct State;
    struct Context;
    State *state_;
    ClientJob(ClientMiles::HSAMPLE, ClientMiles::SampleCallback,
              ClientMiles::HSTREAM, ClientMiles::StreamCallback,
              std::shared_ptr<void>);
    static void execute(void *);
    ClientJob(const ClientJob &);
    ClientJob &operator=(const ClientJob &);
};
}
#endif
