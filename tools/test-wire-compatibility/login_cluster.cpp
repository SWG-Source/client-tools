// Oracle: original client-tools 94945103 LoginClusterStatus.h and Archive.h on Win32.
// These literal bytes are transcribed from that source, NOT produced by candidate pack().
#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedNetworkMessages/LoginClusterStatus.h"
#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace {
int failures;
void check(bool ok, char const *name) {
 std::printf("%s: LoginClusterStatus %s\n", ok ? "PASS" : "FAIL", name);
 failures += !ok;
}
Archive::ByteStream literal(std::initializer_list<unsigned char> bytes) {
 Archive::ByteStream out; for (auto b : bytes) out.put(&b, 1); return out;
}
bool equal(Archive::ByteStream const &a, Archive::ByteStream const &b) {
 return a.getSize()==b.getSize() && (!a.getSize() || !std::memcmp(a.getBuffer(),b.getBuffer(),a.getSize()));
}
bool same(LoginClusterStatus::ClusterData const &a, LoginClusterStatus::ClusterData const &b) {
 // Compare signed fields in a signed, wider domain so uint32 regressions cannot hide -1.
 return a.m_clusterId==b.m_clusterId && a.m_connectionServerAddress==b.m_connectionServerAddress
  && a.m_connectionServerPort==b.m_connectionServerPort && a.m_connectionServerPingPort==b.m_connectionServerPingPort
  && static_cast<long long>(a.m_populationOnline)==static_cast<long long>(b.m_populationOnline)
  && a.m_populationOnlineStatus==b.m_populationOnlineStatus
  && static_cast<long long>(a.m_maxCharactersPerAccount)==static_cast<long long>(b.m_maxCharactersPerAccount)
  && static_cast<long long>(a.m_timeZone)==static_cast<long long>(b.m_timeZone)
  && a.m_status==b.m_status && a.m_dontRecommend==b.m_dontRecommend
  && a.m_onlinePlayerLimit==b.m_onlinePlayerLimit && a.m_onlineFreeTrialLimit==b.m_onlineFreeTrialLimit
  && a.m_isAdmin==b.m_isAdmin && a.m_isSecret==b.m_isSecret;
}
bool recordValues(LoginClusterStatus::ClusterData const &a, unsigned index) {
 // Independent numeric checks remain discriminating if the candidate changes member signedness.
 return index==0 ? static_cast<long long>(a.m_populationOnline)==-1LL
   && static_cast<long long>(a.m_timeZone)==-2147483648LL
   && static_cast<unsigned long long>(a.m_clusterId)==4294967295ULL
   && static_cast<unsigned long long>(a.m_onlinePlayerLimit)==2147483648ULL
  : static_cast<long long>(a.m_populationOnline)==2147483647LL
   && static_cast<long long>(a.m_timeZone)==-3600LL
   && static_cast<unsigned long long>(a.m_clusterId)==2147483648ULL
   && static_cast<unsigned long long>(a.m_onlineFreeTrialLimit)==4294967295ULL;
}
}

int loginClusterFixtures() {
 typedef LoginClusterStatus::ClusterData C;
 std::vector<C> input(2);
 input[0] = C{0xffffffffUL, "alpha.swg", 0, 65535, -1, C::PS_very_light,
  2147483647, (-2147483647-1), C::S_down, false, 0x80000000UL, 0, true, false};
 input[1] = C{0x80000000UL, "b.example:42", 65535, 1, 2147483647, C::PS_full,
  0, -3600, C::S_full, true, 0, 0xffffffffUL, false, true};
 // Element layout: u32 ID, u16 string length + bytes, two u16 ports, five i32
 // (population, population status, max characters, timezone, status), bool,
 // two u32 limits, bool admin, bool secret. No padding. Sizes: 50 and 53 bytes.
 auto first=literal({
  255,255,255,255, 9,0,'a','l','p','h','a','.','s','w','g', 0,0, 255,255,
  255,255,255,255, 0,0,0,0, 255,255,255,127, 0,0,0,128, 0,0,0,0,
  0, 0,0,0,128, 0,0,0,0, 1,0});
 auto second=literal({
  0,0,0,128, 12,0,'b','.','e','x','a','m','p','l','e',':','4','2', 255,255, 1,0,
  255,255,255,127, 6,0,0,0, 0,0,0,0, 240,241,255,255, 5,0,0,0,
  1, 0,0,0,0, 255,255,255,255, 0,1});
 Archive::ByteStream const elements[]={first,second};
 for (unsigned i=0;i<2;++i) {
  Archive::ByteStream bytes; Archive::put(bytes,input[i]);
  check(equal(bytes,elements[i]),i==0 ? "first element encodes stock32 bytes" : "second element encodes stock32 bytes");
  bool ok=false;
  try {
   auto rr=elements[i].begin(); C back={}; Archive::get(rr,back);
   ok=same(back,input[i]) && recordValues(back,i) && rr.getSize()==0;
  } catch (...) { /* A decode exception is a failed fixture, never a pass. */ }
  check(ok,i==0 ? "first literal decodes every field and consumes exactly" : "second literal decodes every field and consumes exactly");
 }
 // The base constructor shim registers NO command CRC. Real AutoByteStream writes
 // one registered variable (u16 1); real AutoArray writes its u32 element count.
 auto expected=literal({1,0, 2,0,0,0});
 expected.put(first.getBuffer(),first.getSize()); expected.put(second.getBuffer(),second.getSize());
 LoginClusterStatus message(input); Archive::ByteStream encoded; message.pack(encoded);
 check(equal(encoded,expected),"two distinct galaxies encode stock32 bounded message bytes");
 bool ok=false;
 try {
  auto rr=expected.begin(); LoginClusterStatus back(rr); auto const &v=back.getData();
  ok=v.size()==2 && same(v[0],input[0]) && same(v[1],input[1])
   && recordValues(v[0],0) && recordValues(v[1],1) && rr.getSize()==0;
 } catch (...) {}
 check(ok,"two literal galaxies decode all fields with no trailing bytes");
 // A following field must remain untouched (including after the last two bools).
 auto framed=expected; unsigned char const sentinel[]={0xa5,0x5a,0xc3}; framed.put(sentinel,3);
 ok=false;
 try {
  auto rr=framed.begin(); LoginClusterStatus back(rr); unsigned char tail[3]={};
  bool const exact=rr.getSize()==3; rr.get(tail,3);
  ok=back.getData().size()==2 && exact && !std::memcmp(tail,sentinel,3) && rr.getSize()==0;
 } catch (...) {}
 check(ok,"decode preserves following sentinel and exact iterator boundary");
 std::vector<C> none; LoginClusterStatus empty(none); Archive::ByteStream emptyBytes; empty.pack(emptyBytes);
 auto emptyExpected=literal({1,0, 0,0,0,0});
 check(equal(emptyBytes,emptyExpected),"empty list encodes stock32 bounded header");
 ok=false;
 try { auto rr=emptyExpected.begin(); LoginClusterStatus back(rr); ok=back.getData().empty() && rr.getSize()==0; }
 catch (...) {}
 check(ok,"empty literal decodes with exact consumption");
 return failures;
}
