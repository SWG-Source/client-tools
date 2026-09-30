#include "metadata.h"
#include <Mss.h>
#include <cstring>
#if !defined(_M_IX86) || _MSC_VER != 1800
#error Original SDK dispatch requires native v120 x86.
#endif
static_assert(DIG_MIXER_CHANNELS==1 && DIG_DS_MIX_FRAGMENT_CNT==42,"reviewed preference IDs");
static_assert(sizeof(SINTa)==4 && SINTa(-1)<0,"original SDK preference result is signed 32-bit");
namespace MilesStartup {
namespace {
uint32_t bits(SINTa value){uint32_t out;std::memcpy(&out,&value,4);return out;}
}
Status dispatch(uint32_t op,const MilesWire::Call& c,MilesTransport::Bytes frame,Reply& out,MilesHost::Resolver& resolver,SessionInputs& inputs) {
    Reply candidate;
    Status status=validate(op,c,frame);
    if(status==Complete) {
        switch(op) {
        case MilesWire::AIL_get_preference:
            candidate.result.return_bits=bits(::AIL_get_preference(c.value[0]));break;
        case MilesWire::AIL_set_preference:
            candidate.result.return_bits=bits(::AIL_set_preference(c.value[0],static_cast<SINTa>(signedValue(c.value[1]))));break;
        case MilesWire::AIL_last_error:
            if(!copyText(::AIL_last_error(),candidate))status=TextTooLong;break;
        case MilesWire::AIL_set_redist_directory: {
            const char* retained=inputs.retain(MilesTransport::Bytes(frame.data+c.text.offset,c.text.length));
            if(!retained){status=InputBudgetExceeded;break;}
            if(!copyText(::AIL_set_redist_directory(retained),candidate))status=TextTooLong;break;
        }
        case MilesWire::AIL_speaker_configuration: {
            uintptr_t native=0;
            if(!resolver.resolve(c.target,1u<<MilesWire::Driver,native)||!native){status=InvalidResource;break;}
            MSS_MC_SPEC spec=MSS_MC_51_DISCRETE; // Distinct valid sentinel; fixture verifies a real write.
            ::AIL_speaker_configuration(reinterpret_cast<HDIGDRIVER>(native),0,0,0,&spec);
            candidate.result.value[3]=bits(static_cast<SINTa>(spec));break;
        }
        default:status=Unsupported;break;
        }
    }
    candidate.result.transport_status=status;
    out.text.swap(candidate.text);out.result=candidate.result;
    return status;
}
}
