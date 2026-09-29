// Adapted from SWG-Source/src tools/test-wire-compatibility/fixtures.cpp (src#35).
// Literal bytes are the legacy x86 (32-bit) wire format, transcribed independently of the
// code under test. Client-tools changes: uint32 spelling, and the packed-map case uses the
// client's AutoDeltaPackedMap<int, unsigned long> specialization (same bytes).
#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/PlayerQuestData.h"
#include "Archive/AutoDeltaPackedMap.h"
#include "Archive/AutoDeltaVector.h"
#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaSet.h"
#include "Archive/AutoDeltaQueue.h"
#include "sharedFoundation/AutoDeltaNetworkIdPackedMap.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"
#include "sharedNetworkMessages/BuffBuilderChangeMessage.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <initializer_list>
#ifdef WIRE_TEST_MISSIONS
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseArchive.h"
#endif

static int failures;
static void check(bool ok, char const *name) { std::printf("%s: %s\n", ok ? "PASS" : "FAIL", name); failures += !ok; }
static Archive::ByteStream literal(std::initializer_list<unsigned char> bytes) {
 Archive::ByteStream out; for (auto b : bytes) out.put(&b,1); return out;
}
static bool equal(Archive::ByteStream const &a, Archive::ByteStream const &b) {
 return a.getSize()==b.getSize() && (!a.getSize() || !std::memcmp(a.getBuffer(),b.getBuffer(),a.getSize()));
}
// Legacy timestamps are signed 32-bit time_t (Win32 _USE_32BIT_TIME_T; Linux -m32).
// Encode T with the message's real pack(); the result must differ from the T=0 encoding only
// in the 4 little-endian bytes at the legacy offset, and unpack() must return T as signed.
template<class Msg> static bool timestampRoundTrip(int offset, long long t) {
 Msg zero, msg; zero.setStartingTime(0); msg.setStartingTime(static_cast<time_t>(t));
 Archive::ByteStream a, b; Msg::pack(&zero,a); Msg::pack(&msg,b);
 if (a.getSize()!=b.getSize() || b.getSize()<static_cast<unsigned>(offset+4)) return false;
 unsigned const u=static_cast<unsigned>(static_cast<int>(t));
 for (unsigned i=0;i<b.getSize();++i) {
  unsigned char const want = (i>=static_cast<unsigned>(offset) && i<static_cast<unsigned>(offset+4)) ? static_cast<unsigned char>(u>>(8*(i-offset))) : a.getBuffer()[i];
  if (b.getBuffer()[i]!=want) return false; }
 auto rr=b.begin(); MessageQueue::Data *d=Msg::unpack(rr);
 bool const ok = static_cast<long long>(static_cast<Msg*>(d)->getStartingTime())==t && rr.getSize()==0;
 delete d; return ok;
}
static bool chatTimeRoundTrip(long long t) {
 ChatLogEntry e(Unicode::String(),Unicode::String(),Unicode::String(),Unicode::String(),static_cast<time_t>(t));
 unsigned const u=static_cast<unsigned>(static_cast<int>(t));
 Archive::ByteStream bytes; Archive::put(bytes,e);
 unsigned char const want[20]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, (unsigned char)u,(unsigned char)(u>>8),(unsigned char)(u>>16),(unsigned char)(u>>24)};
 if (bytes.getSize()!=20 || std::memcmp(bytes.getBuffer(),want,20)) return false;
 ChatLogEntry back; auto rr=bytes.begin(); Archive::get(rr,back);
 return static_cast<long long>(back.m_time)==t && rr.getSize()==0;
}

int main() {
 // First quest use in this process: pack's Command constructs age 1;
 // active and completed values receive ages 2 and 3. This is a legacy32
 // fixture, including the non-persisted relative-age field (not normalized).
 auto quests=literal({2,0,0,0, 0,0,0,0,
   0, 0,0,0,128, 42,0,0,0,0,0,0,0, 1,0, 2,0, 0, 2,0,0,0, 0,
   0, 255,255,255,255, 0,0,0,0,0,0,0,0, 0,0, 0,0, 1, 3,0,0,0, 1});
 Archive::ByteStream questBytes;
 Archive::AutoDeltaPackedMap<uint32,PlayerQuestData>::pack(questBytes,"2147483648 1 2 42:4294967295 1:");
 check(equal(questBytes,quests),"active and completed high-key quests match legacy32 literal bytes including ages");
 std::string questText; auto questRead=quests.begin();
 Archive::AutoDeltaPackedMap<uint32,PlayerQuestData>::unpack(questRead,questText);
 check(questText=="2147483648 1 2 42:4294967295 1:","legacy32 quests decode active tasks, completed reward and high-bit keys");
 // Client-tools specialization <int, unsigned long> ("%i %u"); same legacy x86 bytes.
 // Legacy x86 layout: int32 count, uint32 baseline count, ADD=0, uint32 key/value.
 Archive::ByteStream expected=literal({2,0,0,0, 0,0,0,0, 0, 0,0,0,128, 255,255,255,255, 0, 255,255,255,255, 0,0,0,128});
 Archive::ByteStream actual;
 Archive::AutoDeltaPackedMap<int,unsigned long>::pack(actual,"-2147483648 4294967295:-1 2147483648:");
 check(equal(actual,expected),"packed map high-bit keys and values match legacy32 literal bytes");
 std::string text; auto r=expected.begin();
 Archive::AutoDeltaPackedMap<int,unsigned long>::unpack(r,text);
 check(text=="-2147483648 4294967295:-1 2147483648:","packed map legacy32 literal decodes to exact unsigned decimal values");
 // Empty vector, baseline=UINT32_MAX. Two INSERT commands reach target=1
 // after modulo-2^32 wrap; legacy32 must retain both values, not skip them.
 auto baseline=literal({0,0,0,0,255,255,255,255});
 auto delta=literal({2,0,0,0,1,0,0,0, 1,0,0,65,0,0,0, 1,1,0,66,0,0,0});
 Archive::AutoDeltaVector<uint32> v;
 r=baseline.begin(); v.unpack(r); r=delta.begin(); v.unpackDelta(r);
 check(v.size()==2 && v[0]==65 && v[1]==66,"legacy32 counter wrap decodes two inserts into [65,66]");
 auto finalExpected=literal({2,0,0,0,1,0,0,0,65,0,0,0,66,0,0,0});
 Archive::ByteStream finalBytes; v.pack(finalBytes);
 check(equal(finalBytes,finalExpected),"counter wrap resulting baseline matches legacy32 bytes");
 // AutoDeltaMap counters are legacy 32-bit unsigned (size_t on Win32): all delta
 // arithmetic is modulo 2^32. A client behind by commands that cross 2^31 must apply
 // the pending ADD and catch up, not treat the target as negative and drop it.
 {
  auto mapBaseline=literal({0,0,0,0, 0xf0,0xff,0xff,0x7f});
  auto mapDelta=literal({1,0,0,0, 5,0,0,0x80, 0, 1,0,0,0, 10,0,0,0});
  Archive::AutoDeltaMap<uint32,uint32> m;
  r=mapBaseline.begin(); m.unpack(r); r=mapDelta.begin(); m.unpackDelta(r);
  check(m.size()==1 && m.find(1)!=m.end() && m.find(1)->second==10,"map behind across 2^31 applies the pending ADD");
  Archive::ByteStream mapBytes; m.pack(mapBytes);
  check(equal(mapBytes,literal({1,0,0,0, 5,0,0,0x80, 0,1,0,0,0,10,0,0,0})),"map catches up to baseline 0x80000005 in legacy32 bytes");
 }
 // Wrap through zero: baseline 0xfffffffe, three commands, target 0. The first command
 // is already reflected (skip one); the last two apply and the counter wraps to 0.
 {
  auto mapBaseline=literal({0,0,0,0, 0xfe,0xff,0xff,0xff});
  auto mapDelta=literal({3,0,0,0, 0,0,0,0, 0,1,0,0,0,1,0,0,0, 0,2,0,0,0,2,0,0,0, 0,3,0,0,0,3,0,0,0});
  Archive::AutoDeltaMap<uint32,uint32> m;
  r=mapBaseline.begin(); m.unpack(r); r=mapDelta.begin(); m.unpackDelta(r);
  check(m.size()==2 && m.find(1)==m.end() && m.find(2)!=m.end() && m.find(3)!=m.end(),"map wrap through zero skips the one already-applied command");
  Archive::ByteStream mapBytes; m.pack(mapBytes);
  check(equal(mapBytes,literal({2,0,0,0, 0,0,0,0, 0,2,0,0,0,2,0,0,0, 0,3,0,0,0,3,0,0,0})),"map wrap resulting baseline 0 matches legacy32 bytes");
 }
 // Map, repeated deltas: two in-order deltas apply; replaying the second is skipped.
 {
  Archive::AutoDeltaMap<uint32,uint32> m;
  auto b=literal({0,0,0,0, 0,0,0,0}); r=b.begin(); m.unpack(r);
  auto d1=literal({1,0,0,0, 1,0,0,0, 0,1,0,0,0,1,0,0,0});
  auto d2=literal({1,0,0,0, 2,0,0,0, 0,2,0,0,0,2,0,0,0});
  r=d1.begin(); m.unpackDelta(r); r=d2.begin(); m.unpackDelta(r); r=d2.begin(); m.unpackDelta(r);
  Archive::ByteStream bytes; m.pack(bytes);
  check(equal(bytes,literal({2,0,0,0, 2,0,0,0, 0,1,0,0,0,1,0,0,0, 0,2,0,0,0,2,0,0,0})),"map repeated deltas apply once; a replayed delta is skipped");
 }
 // Set counters follow the same legacy unsigned arithmetic as the map. Deltas carry a
 // command byte (INSERT=1); the baseline encoding is count, baseline, then bare values.
 {
  Archive::AutoDeltaSet<uint32> st;
  auto b=literal({0,0,0,0, 0xf0,0xff,0xff,0x7f}); r=b.begin(); st.unpack(r);
  auto d=literal({1,0,0,0, 5,0,0,0x80, 1,10,0,0,0}); r=d.begin(); st.unpackDelta(r);
  Archive::ByteStream bytes; st.pack(bytes);
  check(st.size()==1 && equal(bytes,literal({1,0,0,0, 5,0,0,0x80, 10,0,0,0})),"set behind across 2^31 applies the INSERT and catches up");
 }
 {
  Archive::AutoDeltaSet<uint32> st;
  auto b=literal({0,0,0,0, 0xfe,0xff,0xff,0xff}); r=b.begin(); st.unpack(r);
  auto d=literal({3,0,0,0, 0,0,0,0, 1,1,0,0,0, 1,2,0,0,0, 1,3,0,0,0}); r=d.begin(); st.unpackDelta(r);
  Archive::ByteStream bytes; st.pack(bytes);
  check(equal(bytes,literal({2,0,0,0, 0,0,0,0, 2,0,0,0, 3,0,0,0})),"set wrap through zero skips the already-applied INSERT");
 }
 // Mirrored verbatim from SWG-Source/src#35 7ace7d51 (server counterpart of this repair).
 // Queue uses unsigned subtraction and clamps the skip count, without map catch-up.
 // baseline=0, one PUSH, target=2 => difference UINT32_MAX: skip the whole delta.
 {
  auto queueBaseline=literal({0,0,0,0, 0,0,0,0});
  auto queueDelta=literal({1,0,0,0, 2,0,0,0, 0, 65,0,0,0});
  Archive::AutoDeltaQueue<uint32_t> q;
  r=queueBaseline.begin(); q.unpack(r); r=queueDelta.begin(); q.unpackDelta(r);
  check(q.empty() && r.getSize()==0,"queue unsigned skip clamps and consumes an ahead delta");
  Archive::ByteStream queueBytes; q.pack(queueBytes);
  check(equal(queueBytes,queueBaseline),"queue skipped delta preserves legacy baseline zero");
 }
 // Two PUSHes wrap UINT32_MAX to 1; applying the same delta twice must not duplicate them.
 {
  auto queueBaseline=literal({0,0,0,0, 255,255,255,255});
  auto queueDelta=literal({2,0,0,0, 1,0,0,0, 0,65,0,0,0, 0,66,0,0,0});
  auto queueExpected=literal({2,0,0,0, 1,0,0,0, 0,65,0,0,0, 0,66,0,0,0});
  Archive::AutoDeltaQueue<uint32_t> q;
  r=queueBaseline.begin(); q.unpack(r); r=queueDelta.begin(); q.unpackDelta(r);
  Archive::ByteStream queueBytes; q.pack(queueBytes);
  check(equal(queueBytes,queueExpected) && r.getSize()==0,"queue wrap preserves both PUSHes and baseline one");
  r=queueDelta.begin(); q.unpackDelta(r);
  Archive::ByteStream repeatedBytes; q.pack(repeatedBytes);
  check(equal(repeatedBytes,queueExpected) && r.getSize()==0,"queue duplicate delta is consumed without reapplying PUSHes");
 }
 // Packed maps with NetworkId (8-byte) and Unicode (uint32 length + UTF-16) values.
 {
  auto expected=literal({2,0,0,0, 0,0,0,0, 0,0xff,0xff,0xff,0xff,0,0,0,0,1,0,0,0, 0,7,0,0,0,42,0,0,0,0,0,0,0});
  Archive::ByteStream bytes; Archive::AutoDeltaPackedMap<int,NetworkId>::pack(bytes,"-1 4294967296:7 42:");
  check(equal(bytes,expected),"packed map <int, NetworkId> matches legacy32 bytes");
  std::string text; auto rr=expected.begin(); Archive::AutoDeltaPackedMap<int,NetworkId>::unpack(rr,text);
  check(text=="-1 4294967296:7 42:","packed map <int, NetworkId> decodes to the same text");
 }
 {
  auto expected=literal({1,0,0,0, 0,0,0,0, 0,0,0,0,0,1,0,0,0, 0xfe,0xff,0xff,0xff});
  Archive::ByteStream bytes; Archive::AutoDeltaPackedMap<NetworkId,int>::pack(bytes,"4294967296 -2:");
  check(equal(bytes,expected),"packed map <NetworkId, int> matches legacy32 bytes");
  std::string text; auto rr=expected.begin(); Archive::AutoDeltaPackedMap<NetworkId,int>::unpack(rr,text);
  check(text=="4294967296 -2:","packed map <NetworkId, int> decodes to the same text");
 }
 {
  auto expected=literal({1,0,0,0, 0,0,0,0, 0,3,0,0,0, 2,0,0,0, 0x68,0, 0xe9,0});
  Archive::ByteStream bytes; Archive::AutoDeltaPackedMap<int,Unicode::String>::pack(bytes,"3 h\xc3\xa9:");
  check(equal(bytes,expected),"packed map <int, Unicode::String> matches legacy32 bytes");
  std::string text; auto rr=expected.begin(); Archive::AutoDeltaPackedMap<int,Unicode::String>::unpack(rr,text);
  check(text=="3 h\xc3\xa9:","packed map <int, Unicode::String> decodes to the same UTF-8 text");
 }
 // A real timestamp through ChatLogEntry's own serializer: four empty strings, then 4 bytes.
 {
  ChatLogEntry e(Unicode::String(),Unicode::String(),Unicode::String(),Unicode::String(),static_cast<time_t>(0x80000001u));
  auto expected=literal({0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,0,0x80});
  Archive::ByteStream bytes; Archive::put(bytes,e);
  check(equal(bytes,expected),"ChatLogEntry timestamp encodes as 4 legacy32 bytes");
  ChatLogEntry back; auto rr=expected.begin(); Archive::get(rr,back);
  check(back.m_time==0x80000001u && rr.getSize()==0,"ChatLogEntry timestamp decodes with no trailing bytes");
 }
 ImageDesignChangeMessage::install(); BuffBuilderChangeMessage::install(); // creates their allocation pools
 for (long long t : {-1LL, 0LL, 2147483647LL}) {
  char name[128];
  std::snprintf(name,sizeof(name),"ImageDesignChangeMessage startingTime %lld round-trips as signed legacy32",t);
  check(timestampRoundTrip<ImageDesignChangeMessage>(33,t),name);
  std::snprintf(name,sizeof(name),"BuffBuilderChangeMessage startingTime %lld round-trips as signed legacy32",t);
  check(timestampRoundTrip<BuffBuilderChangeMessage>(16,t),name);
  std::snprintf(name,sizeof(name),"ChatLogEntry time %lld round-trips as signed legacy32",t);
  check(chatTimeRoundTrip(t),name);
 }
#ifdef WIRE_TEST_MISSIONS
 MessageQueueMissionListResponse::DataVector missions;
 MessageQueueMissionListResponse empty(missions, 7, true);
 Archive::ByteStream emptyBytes; Archive::put(emptyBytes,empty);
 check(equal(emptyBytes,literal({7,1,0,0,0,0})),"empty mission response has six-byte legacy32 header");
 MessageQueueMissionListResponseData mission;
 mission.bond=7; mission.difficulty=8; mission.reward=9; mission.missionData=NetworkId(int64(42));
 missions.push_back(mission); missions.push_back(mission);
 MessageQueueMissionListResponse two(missions,7,true);
 // Fixed legacy32 element layout: int32 bond, two empty UTF16 strings,
 // empty StringId(table uint16 length,index uint32,text uint16 length),
 // difficulty, empty planet/region, int64 ID, empty type, reward,
 // empty planet/region and empty title StringId.
 auto element=literal({7,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,0,0,0,0,
   8,0,0,0, 0,0, 0,0,0,0, 42,0,0,0,0,0,0,0, 0,0,0,0, 9,0,0,0,
   0,0, 0,0,0,0, 0,0,0,0,0,0,0,0});
 auto missionExpected=literal({7,1,2,0,0,0});
 missionExpected.put(element.getBuffer(),element.getSize());
 missionExpected.put(element.getBuffer(),element.getSize());
 Archive::ByteStream missionBytes; Archive::put(missionBytes,two);
 check(equal(missionBytes,missionExpected),"two mission entries match legacy32 count and element bytes");
 MessageQueueMissionListResponse decoded;
 auto missionRead=missionExpected.begin(); Archive::get(missionRead,decoded);
 check(decoded.getSequenceId()==7 && decoded.getBountyTerminal() && decoded.getResponse().size()==2
   && decoded.getResponse()[0].bond==7 && decoded.getResponse()[1].difficulty==8
   && decoded.getResponse()[1].reward==9 && decoded.getResponse()[1].missionData==NetworkId(int64(42))
   && missionRead.getSize()==0,"legacy32 mission response decodes both entries with no trailing bytes");
#else
 std::puts("NOT RUN: MissionListResponse (pass --build-dir with same-ABI server libraries)");
#endif
 return failures ? 1 : 0;
}
