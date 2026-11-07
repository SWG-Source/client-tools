#include "Archive.h"
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
  unsigned char bytes[16];
  for (unsigned i = 0; i < 16; ++i)
    bytes[i] = static_cast<unsigned char>(i + 1);
  Archive::ReadIterator nullIterator;
  nullIterator.advance(0);
  CHECK(nullIterator.getSize() == 0);
  bool nullRejected = false;
  try {
    nullIterator.advance(1);
  } catch (Archive::ReadException const &) {
    nullRejected = true;
  }
  CHECK(nullRejected);
  Archive::ByteStream empty;
  Archive::ReadIterator er = empty.begin();
  er.get(0, 0);
  CHECK(er.getReadPosition() == 0);
  Archive::ByteStream b(bytes, 16);
  Archive::ReadIterator r = b.begin();
  unsigned char got = 0;
  r.get(&got, 1);
  CHECK(got == 1);
  b.clear();
  bool rejected = false;
  try {
    r.get(&got, 1);
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  CHECK(r.getReadPosition() == 1);
  rejected = false;
  try {
    r.getSize();
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  r = b.begin();
  rejected = false;
  try {
    r.advance(1);
  } catch (Archive::ReadException const &) {
    rejected = true;
  }
  CHECK(rejected);
  CHECK(r.getReadPosition() == 0);
  b.put(bytes, 16);
  r = b.begin();
  r.advance(16);
  CHECK(r.getSize() == 0);
  r.get(0, 0);
  CHECK(r.getReadPosition() == 16);
  // Valid self-source; candidate only. No failing/corrupting stock run.
  b.put(b.getBuffer(), 16);
  CHECK(b.getSize() == 32);
  CHECK(std::memcmp(b.getBuffer(), b.getBuffer() + 16, 16) == 0);
  Archive::ByteStream shared = b;
  unsigned char tail = 99;
  b.put(&tail, 1);
  CHECK(shared.getSize() == 32);
  CHECK(b.getSize() == 33);
  CHECK(b.getBuffer()[32] == 99);
  CHECK(std::memcmp(shared.getBuffer(), b.getBuffer(), 32) == 0);
  shared.setAllocatedSizeLimit(128);
  shared.put(bytes, 16);
  CHECK(shared.getSize() == 48);
  CHECK(b.getSize() == 33);
  Archive::ReadIterator copyin = shared.begin();
  copyin.advance(16);
  Archive::ByteStream copied(copyin);
  CHECK(copied.getSize() == 32);
  CHECK(copyin.getSize() == 0);
  // Exercise ordinary pool reuse and detach using only small valid buffers.
  for (unsigned n = 1; n <= 16; ++n) {
    Archive::ByteStream x(bytes, n);
    Archive::ByteStream y = x;
    x.put(bytes, n);
    CHECK(y.getSize() == n);
    CHECK(x.getSize() == 2 * n);
    CHECK(std::memcmp(x.getBuffer(), bytes, n) == 0);
    CHECK(std::memcmp(x.getBuffer() + n, bytes, n) == 0);
  }
  std::printf("PASS: %u bounded initialized-storage checks\n", checks);
  return 0;
}
