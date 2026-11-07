#include "Archive.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <cstdio>
#include <cstring>
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      std::printf("FAIL line %d\n", __LINE__);                                 \
      return 1;                                                                \
    }                                                                          \
    ++checks;                                                                  \
  } while (0)
int main() {
  unsigned checks = 0;
  // Every input buffer is small and fully initialized. Candidate-only rejection
  // tests.
  unsigned char shortBytes[] = {3, 0, 'a', 0, 'b'};
  Archive::ByteStream shortStream(shortBytes, sizeof(shortBytes));
  auto si = shortStream.begin();
  std::string text = "old";
  Archive::get(si, text);
  CHECK(text == std::string("a\0b", 3));
  CHECK(si.getSize() == 0);
  unsigned char longBytes[] = {255, 255, 3, 0, 0, 0, 'x', 'y', 'z'};
  Archive::ByteStream longStream(longBytes, sizeof(longBytes));
  auto li = longStream.begin();
  Archive::get(li, text);
  CHECK(text == "xyz");
  CHECK(li.getSize() == 0);
  unsigned char emptyBytes[] = {0, 0, 0, 0};
  Archive::ByteStream zero(emptyBytes, 4);
  auto zi = zero.begin();
  Archive::get(zi, text);
  CHECK(text.empty());
  CHECK(zi.getReadPosition() == 2);
  unsigned char byteBytes[] = {2, 0, 0, 0, 8, 9};
  Archive::ByteStream src(byteBytes, 6), dst(emptyBytes, 1);
  auto bi = src.begin();
  Archive::get(bi, dst);
  CHECK(dst.getSize() == 3);
  CHECK(dst.getBuffer()[1] == 8 && dst.getBuffer()[2] == 9);
  CHECK(bi.getSize() == 0);
  zi = zero.begin();
  Archive::get(zi, dst);
  CHECK(dst.getSize() == 3);
  CHECK(zi.getSize() == 0);
  unsigned char unicodeBytes[] = {77, 3, 0, 0, 0, 65, 0, 0, 0, 0xac, 0x20};
  Archive::ByteStream us(unicodeBytes, sizeof(unicodeBytes));
  auto ui = us.begin();
  ui.advance(1);
  Unicode::String unicode(1, 99);
  Archive::get(ui, unicode);
  CHECK(unicode.size() == 3);
  CHECK(unicode[0] == 65 && unicode[1] == 0 && unicode[2] == 0x20ac);
  CHECK(ui.getSize() == 0);
  zi = zero.begin();
  Archive::get(zi, unicode);
  CHECK(unicode.empty());
  CHECK(zi.getSize() == 0);
  unsigned char badShort[] = {3, 0, 1, 2};
  Archive::ByteStream bs(badShort, 4);
  auto br = bs.begin();
  text = "keep";
  bool rejected = false;
  try {
    Archive::get(br, text);
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  CHECK(text == "keep");
  CHECK(br.getReadPosition() == 2);
  unsigned char badLong[] = {255, 255, 255, 255, 255, 255};
  Archive::ByteStream bl(badLong, 6);
  br = bl.begin();
  rejected = false;
  try {
    Archive::get(br, text);
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  CHECK(text == "keep");
  CHECK(br.getReadPosition() == 6);
  unsigned char badByte[] = {3, 0, 0, 0, 1, 2};
  Archive::ByteStream bb(badByte, 6);
  br = bb.begin();
  rejected = false;
  try {
    Archive::get(br, dst);
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  CHECK(dst.getSize() == 3 && dst.getBuffer()[2] == 9);
  CHECK(br.getReadPosition() == 4);
  unsigned char badUnicode[] = {3, 0, 0, 0, 1, 0, 2, 0};
  Archive::ByteStream bu(badUnicode, 8);
  br = bu.begin();
  unicode.assign(1, 99);
  rejected = false;
  try {
    Archive::get(br, unicode);
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  CHECK(unicode.size() == 1 && unicode[0] == 99);
  CHECK(br.getReadPosition() == 4);
  unsigned char maxUnits[] = {255, 255, 255, 255};
  Archive::ByteStream mu(maxUnits, 4);
  br = mu.begin();
  rejected = false;
  try {
    Archive::get(br, unicode);
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  CHECK(unicode.size() == 1 && unicode[0] == 99);
  CHECK(br.getReadPosition() == 4);
  Archive::ByteStream incomplete(emptyBytes, 1);
  br = incomplete.begin();
  rejected = false;
  try {
    Archive::get(br, text);
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  CHECK(text == "keep");
  CHECK(br.getReadPosition() == 0);
  std::printf("PASS: %u bounded decoder checks\n", checks);
  return 0;
}
