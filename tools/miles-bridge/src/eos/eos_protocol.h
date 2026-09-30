#ifndef MILES_EOS_PROTOCOL_H
#define MILES_EOS_PROTOCOL_H
#include "../wire/codec.h"
namespace MilesEos {
bool validEvent(const MilesWire::Header &,const MilesWire::Eos &);
bool encodeCompletion(MilesWire::Header,const MilesWire::Eos &,std::vector<unsigned char> &);
bool validateCompletion(MilesTransport::Bytes,const MilesWire::Header &,const MilesWire::Eos &);
bool encodeConsumption(MilesWire::Header,const MilesWire::Eos &,std::vector<unsigned char> &);
bool validateConsumption(MilesTransport::Bytes,const MilesWire::Header &,const MilesWire::Eos &);
}
#endif
