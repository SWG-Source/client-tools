// Private protocol v2 pair-output topology, not SDK nullability policy.
#ifndef MILES_PAIR_OUTPUTS_H
#define MILES_PAIR_OUTPUTS_H
#include <stdint.h>
namespace MilesWire {
inline bool validPairMask(uint32_t mask) {
    return !(mask & ~7u) && (!(mask & 4u) || (mask & 3u) == 3u);
}
template<class T> inline uint32_t pairMask(T *first, T *second) {
    return (first ? 1u : 0u) | (second ? 2u : 0u) |
           ((first && first == second) ? 4u : 0u);
}
template<class T> struct PairOutputs {
    T firstValue, secondValue;
    explicit PairOutputs(uint32_t mask) : firstValue(), secondValue(), mask_(mask) {}
    T *first() { return (mask_ & 1u) ? &firstValue : 0; }
    T *second() { return (mask_ & 2u) ? ((mask_ & 4u) ? &firstValue : &secondValue) : 0; }
private:
    uint32_t mask_;
};
}
#endif
