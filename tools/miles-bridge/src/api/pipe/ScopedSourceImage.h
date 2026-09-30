#ifndef CLIENT_MILES_PRIVATE_SOURCE_IMAGE_H
#define CLIENT_MILES_PRIVATE_SOURCE_IMAGE_H
#include <stdint.h>
namespace ClientMilesPipe {
class Session;
// Private root/adoption only. Caller keeps allocation readable/stable and uses
// the same serialized owner through scope destruction. Not part of public70.
// Setup failures use the bound fatal reporter; no adapter exception crosses
// into the engine/STLport translation unit.
class ScopedSourceImage {
public:
    ScopedSourceImage(const void *base,uint32_t bytes);
    ~ScopedSourceImage(); // clears only this token; never calls transport
private:
    Session &owner_;
    const void *base_;
    uint32_t bytes_;
    friend class Session;
    ScopedSourceImage(const ScopedSourceImage &);
    ScopedSourceImage &operator=(const ScopedSourceImage &);
};
}
#endif
