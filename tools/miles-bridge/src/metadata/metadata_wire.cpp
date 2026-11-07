#include "metadata.h"
#include <cstring>
#include <limits>
namespace MilesStartup {
const char* SessionInputs::retain(MilesTransport::Bytes input) {
    if(!input.data || !input.size || input.size>RequestTextLimit || input.size>MilesWire::MaxFrameBytes-used)return 0;
    std::vector<unsigned char> candidate(input.data,input.data+input.size);
    paths.push_back(std::vector<unsigned char>());
    paths.back().swap(candidate); // Stable list entry; no reliance on implicit moves.
    used+=input.size;
    return reinterpret_cast<const char*>(&paths.back()[0]);
}
Reply::Reply(){std::memset(&result,0,sizeof result);}
bool preferenceValue(int64_t value,uint32_t& bits) {
    if(value < (std::numeric_limits<int32_t>::min)() || value > (std::numeric_limits<int32_t>::max)())return false;
    int32_t narrowed=static_cast<int32_t>(value);std::memcpy(&bits,&narrowed,4);return true;
}
int64_t signedValue(uint32_t bits){int32_t value;std::memcpy(&value,&bits,4);return value;}
namespace {
bool nullHandle(const MilesWire::Handle& h){return !h.kind&&!h.slot&&!h.generation;}
}
Status validate(uint32_t op,const MilesWire::Call& c,MilesTransport::Bytes frame) {
    unsigned values=0;bool driver=false,text=false;
    switch(op) {
    case MilesWire::AIL_get_preference: values=1;break;
    case MilesWire::AIL_set_preference: values=2;break;
    case MilesWire::AIL_last_error:break;
    case MilesWire::AIL_set_redist_directory:text=true;break;
    case MilesWire::AIL_speaker_configuration:driver=true;break;
    default:return Unsupported;
    }
    if(c.reserved||c.callback||c.bytes.offset||c.bytes.length||!nullHandle(c.resource))return InvalidFields;
    for(unsigned i=values;i<8;++i)if(c.value[i])return InvalidFields;
    if(driver) { if(c.output_mask!=8)return InvalidFields; }
    else if(!nullHandle(c.target)||c.output_mask)return InvalidFields;
    if(!text && (c.text.offset||c.text.length))return InvalidFields;
    if(text) {
        if(!frame.data||!c.text.length||c.text.length>RequestTextLimit||c.text.offset>frame.size||c.text.length>frame.size-c.text.offset)return InvalidFields;
        const unsigned char* p=frame.data+c.text.offset;
        if(p[c.text.length-1] || std::memchr(p,0,c.text.length-1))return InvalidFields;
    }
    if(op==MilesWire::AIL_get_preference && c.value[0]!=1 && c.value[0]!=42)return InvalidFields;
    if(op==MilesWire::AIL_set_preference && (c.value[0]!=42 || (c.value[1]!=16 && c.value[1]!=64)))return InvalidFields;
    return Complete;
}
bool copyText(const char* source,Reply& destination) {
    std::vector<unsigned char> candidate;
    uint32_t nullMask=TextNull;
    if(source) {
        size_t n=0;while(n<ReplyTextLimit && source[n])++n;
        if(n==ReplyTextLimit)return false;
        candidate.assign(reinterpret_cast<const unsigned char*>(source),reinterpret_cast<const unsigned char*>(source)+n+1);
        nullMask=0;
    }
    destination.text.swap(candidate);destination.result.null_mask=nullMask;return true;
}
}
