#ifndef MILES_HOST_REGISTRY_RESOLVER_H
#define MILES_HOST_REGISTRY_RESOLVER_H
#include "host_dispatch.h"
#include "../wire/resource_registry.h"
namespace MilesHost {
class RegistryResolver : public Resolver {
 MilesTransport::ResourceRegistry const &registry;
 RegistryResolver &operator=(RegistryResolver const &);
public:
 explicit RegistryResolver(MilesTransport::ResourceRegistry const &value):registry(value) {}
 bool resolve(MilesWire::Handle const &handle,uint32_t allowed,uintptr_t &native) {
  native=0;
  if(handle.kind!=MilesWire::Driver && handle.kind!=MilesWire::OwnedSample && handle.kind!=MilesWire::Stream && handle.kind!=MilesWire::BorrowedSample)return false;
  if(!(allowed & (1u<<handle.kind)))return false;
  void *local=0;
  if(!registry.resolve(handle,static_cast<MilesWire::ResourceKind>(handle.kind),local))return false;
  native=reinterpret_cast<uintptr_t>(local);return true;
 }
};
}
#endif
