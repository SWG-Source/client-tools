#include "host_dispatch.h"
#include "../protocol/pair_outputs.h"
#include "Mss.h"
#include <string.h>
#if !defined(_M_IX86) || _MSC_VER != 1800
#error This candidate targets the original x86 vendor ABI with v120.
#endif
namespace MilesHost {
namespace {
S32 s(uint32_t v){ S32 result; memcpy(&result,&v,4); return result; }
F32 f(uint32_t v){ F32 result; memcpy(&result,&v,4); return result; }
uint32_t bits(S32 v){uint32_t result;memcpy(&result,&v,4);return result;}
uint32_t floatBits(F32 v){uint32_t result;memcpy(&result,&v,4);return result;}
}
bool supports(uint32_t opcode){switch(opcode){
case MilesWire::AIL_start_sample:
case MilesWire::AIL_stop_sample:
case MilesWire::AIL_end_sample:
case MilesWire::AIL_start_stream:
case MilesWire::AIL_active_sample_count:
case MilesWire::AIL_digital_CPU_percent:
case MilesWire::AIL_digital_latency:
case MilesWire::AIL_room_type:
case MilesWire::AIL_sample_status:
case MilesWire::AIL_sample_position:
case MilesWire::AIL_sample_playback_rate:
case MilesWire::AIL_stream_status:
case MilesWire::AIL_sample_ms_position:
case MilesWire::AIL_stream_ms_position:
case MilesWire::AIL_sample_volume_levels:
case MilesWire::AIL_sample_reverb_levels:
case MilesWire::AIL_set_3D_rolloff_factor:
case MilesWire::AIL_set_room_type:
case MilesWire::AIL_set_listener_3D_position:
case MilesWire::AIL_set_listener_3D_velocity_vector:
case MilesWire::AIL_set_listener_3D_orientation:
case MilesWire::AIL_set_sample_3D_position:
case MilesWire::AIL_set_sample_3D_velocity_vector:
case MilesWire::AIL_set_sample_3D_distances:
case MilesWire::AIL_set_sample_obstruction:
case MilesWire::AIL_set_sample_occlusion:
case MilesWire::AIL_set_sample_volume_levels:
case MilesWire::AIL_set_sample_reverb_levels:
case MilesWire::AIL_set_sample_loop_count:
case MilesWire::AIL_set_sample_ms_position:
case MilesWire::AIL_set_sample_playback_rate:
case MilesWire::AIL_set_sample_position:
case MilesWire::AIL_set_sample_loop_block:
case MilesWire::AIL_set_stream_loop_block:
case MilesWire::AIL_set_stream_loop_count:
case MilesWire::AIL_set_stream_ms_position:
case MilesWire::AIL_file_error:
case MilesWire::AIL_get_timer_highest_delay:
case MilesWire::AIL_serve:
return true; default:return false;}}
static DispatchStatus dispatchImpl(uint32_t opcode,MilesWire::Call const &call,MilesWire::Result &out,Resolver &resolver){
 if(!supports(opcode))return Unsupported;
 if(call.reserved || call.callback || call.bytes.offset || call.bytes.length || call.text.offset || call.text.length || call.resource.kind || call.resource.slot || call.resource.generation)return InvalidFields;
 switch(opcode){
case MilesWire::AIL_start_sample: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_start_sample(sample);
 return Complete; }
case MilesWire::AIL_stop_sample: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_stop_sample(sample);
 return Complete; }
case MilesWire::AIL_end_sample: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_end_sample(sample);
 return Complete; }
case MilesWire::AIL_start_stream: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Stream,address) || !address)return InvalidResource;
 HSTREAM stream=reinterpret_cast<HSTREAM>(address);
 ::AIL_start_stream(stream);
 return Complete; }
case MilesWire::AIL_active_sample_count: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 out.return_bits=bits(::AIL_active_sample_count(driver));
 return Complete; }
case MilesWire::AIL_digital_CPU_percent: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 out.return_bits=bits(::AIL_digital_CPU_percent(driver));
 return Complete; }
case MilesWire::AIL_digital_latency: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 out.return_bits=bits(::AIL_digital_latency(driver));
 return Complete; }
case MilesWire::AIL_room_type: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 out.return_bits=bits(::AIL_room_type(driver));
 return Complete; }
case MilesWire::AIL_sample_status: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 out.return_bits=::AIL_sample_status(sample);
 return Complete; }
case MilesWire::AIL_sample_position: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 out.return_bits=::AIL_sample_position(sample);
 return Complete; }
case MilesWire::AIL_sample_playback_rate: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,((1u<<MilesWire::OwnedSample)|(1u<<MilesWire::BorrowedSample)),address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 out.return_bits=bits(::AIL_sample_playback_rate(sample));
 return Complete; }
case MilesWire::AIL_stream_status: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Stream,address) || !address)return InvalidResource;
 HSTREAM stream=reinterpret_cast<HSTREAM>(address);
 out.return_bits=bits(::AIL_stream_status(stream));
 return Complete; }
case MilesWire::AIL_sample_ms_position: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(!MilesWire::validPairMask(call.output_mask))return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 MilesWire::PairOutputs<S32> pair(call.output_mask); ::AIL_sample_ms_position(sample,pair.first(),pair.second()); if(pair.first())out.value[0]=bits(*pair.first()); if(pair.second())out.value[1]=bits(*pair.second());
 return Complete; }
case MilesWire::AIL_stream_ms_position: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(!MilesWire::validPairMask(call.output_mask))return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Stream,address) || !address)return InvalidResource;
 HSTREAM stream=reinterpret_cast<HSTREAM>(address);
 MilesWire::PairOutputs<S32> pair(call.output_mask); ::AIL_stream_ms_position(stream,pair.first(),pair.second()); if(pair.first())out.value[0]=bits(*pair.first()); if(pair.second())out.value[1]=bits(*pair.second());
 return Complete; }
case MilesWire::AIL_sample_volume_levels: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(!MilesWire::validPairMask(call.output_mask))return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,((1u<<MilesWire::OwnedSample)|(1u<<MilesWire::BorrowedSample)),address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 MilesWire::PairOutputs<F32> pair(call.output_mask); ::AIL_sample_volume_levels(sample,pair.first(),pair.second()); if(pair.first())out.value[0]=floatBits(*pair.first()); if(pair.second())out.value[1]=floatBits(*pair.second());
 return Complete; }
case MilesWire::AIL_sample_reverb_levels: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(!MilesWire::validPairMask(call.output_mask))return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 MilesWire::PairOutputs<F32> pair(call.output_mask); ::AIL_sample_reverb_levels(sample,pair.first(),pair.second()); if(pair.first())out.value[0]=floatBits(*pair.first()); if(pair.second())out.value[1]=floatBits(*pair.second());
 return Complete; }
case MilesWire::AIL_set_3D_rolloff_factor: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 ::AIL_set_3D_rolloff_factor(driver,f(call.value[0]));
 return Complete; }
case MilesWire::AIL_set_room_type: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 ::AIL_set_room_type(driver,s(call.value[0]));
 return Complete; }
case MilesWire::AIL_set_listener_3D_position: {
 for(unsigned i=3;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 ::AIL_set_listener_3D_position(driver,f(call.value[0]),f(call.value[1]),f(call.value[2]));
 return Complete; }
case MilesWire::AIL_set_listener_3D_velocity_vector: {
 for(unsigned i=3;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 ::AIL_set_listener_3D_velocity_vector(driver,f(call.value[0]),f(call.value[1]),f(call.value[2]));
 return Complete; }
case MilesWire::AIL_set_listener_3D_orientation: {
 for(unsigned i=6;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Driver,address) || !address)return InvalidResource;
 HDIGDRIVER driver=reinterpret_cast<HDIGDRIVER>(address);
 ::AIL_set_listener_3D_orientation(driver,f(call.value[0]),f(call.value[1]),f(call.value[2]),f(call.value[3]),f(call.value[4]),f(call.value[5]));
 return Complete; }
case MilesWire::AIL_set_sample_3D_position: {
 for(unsigned i=3;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_3D_position(sample,f(call.value[0]),f(call.value[1]),f(call.value[2]));
 return Complete; }
case MilesWire::AIL_set_sample_3D_velocity_vector: {
 for(unsigned i=3;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_3D_velocity_vector(sample,f(call.value[0]),f(call.value[1]),f(call.value[2]));
 return Complete; }
case MilesWire::AIL_set_sample_3D_distances: {
 for(unsigned i=3;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_3D_distances(sample,f(call.value[0]),f(call.value[1]),s(call.value[2]));
 return Complete; }
case MilesWire::AIL_set_sample_obstruction: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_obstruction(sample,f(call.value[0]));
 return Complete; }
case MilesWire::AIL_set_sample_occlusion: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_occlusion(sample,f(call.value[0]));
 return Complete; }
case MilesWire::AIL_set_sample_volume_levels: {
 for(unsigned i=2;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,((1u<<MilesWire::OwnedSample)|(1u<<MilesWire::BorrowedSample)),address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_volume_levels(sample,f(call.value[0]),f(call.value[1]));
 return Complete; }
case MilesWire::AIL_set_sample_reverb_levels: {
 for(unsigned i=2;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,((1u<<MilesWire::OwnedSample)|(1u<<MilesWire::BorrowedSample)),address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_reverb_levels(sample,f(call.value[0]),f(call.value[1]));
 return Complete; }
case MilesWire::AIL_set_sample_loop_count: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_loop_count(sample,s(call.value[0]));
 return Complete; }
case MilesWire::AIL_set_sample_ms_position: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_ms_position(sample,s(call.value[0]));
 return Complete; }
case MilesWire::AIL_set_sample_playback_rate: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,((1u<<MilesWire::OwnedSample)|(1u<<MilesWire::BorrowedSample)),address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_playback_rate(sample,s(call.value[0]));
 return Complete; }
case MilesWire::AIL_set_sample_position: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_position(sample,call.value[0]);
 return Complete; }
case MilesWire::AIL_set_sample_loop_block: {
 for(unsigned i=2;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::OwnedSample,address) || !address)return InvalidResource;
 HSAMPLE sample=reinterpret_cast<HSAMPLE>(address);
 ::AIL_set_sample_loop_block(sample,s(call.value[0]),s(call.value[1]));
 return Complete; }
case MilesWire::AIL_set_stream_loop_block: {
 for(unsigned i=2;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Stream,address) || !address)return InvalidResource;
 HSTREAM stream=reinterpret_cast<HSTREAM>(address);
 ::AIL_set_stream_loop_block(stream,s(call.value[0]),s(call.value[1]));
 return Complete; }
case MilesWire::AIL_set_stream_loop_count: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Stream,address) || !address)return InvalidResource;
 HSTREAM stream=reinterpret_cast<HSTREAM>(address);
 ::AIL_set_stream_loop_count(stream,s(call.value[0]));
 return Complete; }
case MilesWire::AIL_set_stream_ms_position: {
 for(unsigned i=1;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 uintptr_t address=0; if(!resolver.resolve(call.target,1u<<MilesWire::Stream,address) || !address)return InvalidResource;
 HSTREAM stream=reinterpret_cast<HSTREAM>(address);
 ::AIL_set_stream_ms_position(stream,s(call.value[0]));
 return Complete; }
case MilesWire::AIL_file_error: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 if(call.target.kind || call.target.slot || call.target.generation)return InvalidResource;
 out.return_bits=bits(::AIL_file_error());
 return Complete; }
case MilesWire::AIL_get_timer_highest_delay: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 if(call.target.kind || call.target.slot || call.target.generation)return InvalidResource;
 out.return_bits=::AIL_get_timer_highest_delay();
 return Complete; }
case MilesWire::AIL_serve: {
 for(unsigned i=0;i<8;++i)if(call.value[i])return InvalidFields;
 if(call.output_mask & ~0u)return InvalidFields;
 if(call.target.kind || call.target.slot || call.target.generation)return InvalidResource;
 ::AIL_serve();
 return Complete; }
default:return Unsupported;}}
DispatchStatus dispatch(uint32_t opcode,MilesWire::Call const &call,MilesWire::Result &out,Resolver &resolver){
 memset(&out,0,sizeof(out));
 DispatchStatus const status=dispatchImpl(opcode,call,out,resolver);
 out.transport_status=static_cast<uint32_t>(status);
 return status;
}
}
