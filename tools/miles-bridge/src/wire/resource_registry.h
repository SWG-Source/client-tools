#ifndef MILES_TRANSPORT_REGISTRY_H
#define MILES_TRANSPORT_REGISTRY_H
#include "../protocol/miles_wire.h"
#include <stdexcept>
#include <vector>
namespace MilesTransport {
inline bool knownResource(uint32_t k) { return k >= MilesWire::Driver && k <= MilesWire::Video; }
inline bool validHandle(const MilesWire::Handle &h) {
    return (h.kind == 0 && h.slot == 0 && h.generation == 0) ||
           (knownResource(h.kind) && h.slot && h.generation);
}
// Dispatch-thread owned. Vendor quiescence and session binding
// are required from the caller. No vendor memory is freed by this container.
class ResourceRegistry {
  public:
    class Reservation {
        friend class ResourceRegistry;
        ResourceRegistry *owner;
        MilesWire::Handle handle;
        Reservation(const Reservation &);
        Reservation &operator=(const Reservation &);
      public:
        Reservation() : owner(0), handle() {}
        ~Reservation();
    };
  private:
    struct Entry {
        uint32_t kind, generation, state;
        void *local;
        MilesWire::Handle parent;
        Entry() : kind(0), generation(1), state(0), local(0) {
            parent.kind = parent.slot = parent.generation = 0;
        }
    };
    std::vector<Entry> entries;
    uint32_t capacity, generationLimit;
    Entry *entry(const MilesWire::Handle &h) {
        if (!knownResource(h.kind) || !h.slot || h.slot > entries.size())
            return 0;
        Entry &e = entries[h.slot - 1];
        return e.kind == h.kind && e.generation == h.generation && (e.state == 1 || e.state == 2)
                   ? &e
                   : 0;
    }

    bool insertEntry(MilesWire::ResourceKind kind, void *local, MilesWire::Handle &out) {
        if (!knownResource(kind) || !local)
            return false;
        size_t i = 0;
        for (; i < entries.size(); ++i)
            if (entries[i].state == 0)
                break;
        if (i == entries.size()) {
            if (i >= capacity)
                return false;
            entries.push_back(Entry());
        }
        Entry &e = entries[i];
        e.kind = kind;
        e.local = local;
        e.state = 1;
        MilesWire::Handle h = {static_cast<uint32_t>(kind), static_cast<uint32_t>(i + 1),
                               e.generation};
        out = h;
        return true;
    }
    static bool same(const MilesWire::Handle &a, const MilesWire::Handle &b) {
        return a.kind == b.kind && a.slot == b.slot && a.generation == b.generation;
    }
    void retireEntry(Entry &e) {
        e.local = 0;
        e.kind = 0;
        e.parent.kind = e.parent.slot = e.parent.generation = 0;
        if (e.generation == generationLimit)
            e.state = 3;
        else {
            ++e.generation;
            e.state = 0;
        }
    }

    void retireBorrowed(const MilesWire::Handle &stream) {
        for (size_t i = 0; i < entries.size(); ++i) {
            Entry &alias = entries[i];
            if (alias.state == 1 && alias.kind == MilesWire::BorrowedSample &&
                same(alias.parent, stream)) retireEntry(alias);
        }
    }

  public:
    bool empty() const {
        for (size_t i = 0; i < entries.size(); ++i)
            if (entries[i].state != 0 && entries[i].state != 3) return false;
        return true;
    }
    explicit ResourceRegistry(uint32_t maxSlots = 65536, uint32_t maxGeneration = UINT32_MAX)
        : capacity(maxSlots), generationLimit(maxGeneration) {
        if (!capacity || !generationLimit)
            throw std::invalid_argument("zero registry bound");
    }
    // reserve/insert/insertBorrowed may allocate; publish/cancel/retire do not.
    // Single dispatch thread; no callback registration.
    // A token must not outlive this registry. Parent retirement invalidates tokens;
    // later cancel is safe and cannot affect a replacement generation.
    bool reserve(MilesWire::ResourceKind kind, const MilesWire::Handle &parent,
                 Reservation &token) {
        if (token.owner || (kind != MilesWire::Driver && kind != MilesWire::OwnedSample && kind != MilesWire::Stream && kind != MilesWire::File && kind != MilesWire::Buffer && kind != MilesWire::Video))
            return false;
        if (kind == MilesWire::OwnedSample || kind == MilesWire::Stream || kind == MilesWire::Video) {
            void *driver = 0;
            if (!resolve(parent, MilesWire::Driver, driver)) return false;
        } else if (parent.kind || parent.slot || parent.generation) return false;
        size_t i = 0;
        for (; i < entries.size(); ++i) if (entries[i].state == 0) break;
        if (i == entries.size()) {
            if (i >= capacity) return false;
            entries.push_back(Entry()); // all tracking allocation precedes vendor work
        }
        Entry &e = entries[i];
        e.kind = kind;
        e.local = 0;
        e.parent = parent;
        e.state = 4; // unpublished reservation; cannot resolve/close/retire directly
        token.handle.kind = kind;
        token.handle.slot = static_cast<uint32_t>(i + 1);
        token.handle.generation = e.generation;
        token.owner = this;
        return true;
    }
    void cancel(Reservation &token) throw() {
        if (token.owner != this) return;
        Entry &e = entries[token.handle.slot - 1];
        if (e.state == 4 && e.generation == token.handle.generation &&
            e.kind == token.handle.kind) {
            e.kind = 0; e.local = 0; e.parent = MilesWire::Handle(); e.state = 0;
        }
        token.owner = 0;
    }
    bool publish(Reservation &token, void *local, MilesWire::Handle &out) throw() {
        // Violations are programming errors, never recoverable vendor-null replies.
        if (token.owner != this || !local) return false;
        Entry &e = entries[token.handle.slot - 1];
        if (e.state != 4 || e.kind != token.handle.kind ||
            e.generation != token.handle.generation) return false;
        e.local = local;
        e.state = 1; // durable registry ownership before external identity publication
        out = token.handle;
        token.owner = 0;
        return true;
    }
    bool insert(MilesWire::ResourceKind kind, void *local, MilesWire::Handle &out) {
        if (kind == MilesWire::BorrowedSample || kind == MilesWire::Stream || kind == MilesWire::Video)
            return false; // Stream/Video identities require a reserved, validated Driver parent.
        return insertEntry(kind, local, out);
    }
    bool insertBorrowed(const MilesWire::Handle &parent, void *local, MilesWire::Handle &out) {
        void *stream = 0;
        if (!local || !resolve(parent, MilesWire::Stream, stream))
            return false;
        for (size_t i = 0; i < entries.size(); ++i) {
            const Entry &e = entries[i];
            if (e.state == 1 && e.kind == MilesWire::BorrowedSample && same(e.parent, parent)) {
                if (e.local != local)
                    return false; // never silently retarget an existing alias
                MilesWire::Handle h = {MilesWire::BorrowedSample, static_cast<uint32_t>(i + 1),
                                       e.generation};
                out = h;
                return true;
            }
        }
        MilesWire::Handle h;
        if (!insertEntry(MilesWire::BorrowedSample, local, h))
            return false;
        entries[h.slot - 1].parent = parent;
        out = h;
        return true;
    }
    bool resolve(const MilesWire::Handle &h, MilesWire::ResourceKind expected, void *&out) const {
        out = 0;
        if (!knownResource(expected) || h.kind != static_cast<uint32_t>(expected) || !h.slot ||
            h.slot > entries.size())
            return false;
        const Entry &e = entries[h.slot - 1];
        if (e.state != 1 || e.kind != h.kind || e.generation != h.generation || !e.local)
            return false;
        if (e.kind == MilesWire::Stream || e.kind == MilesWire::Video || (e.kind == MilesWire::OwnedSample && e.parent.kind)) {
            void *driver = 0;
            if (!resolve(e.parent, MilesWire::Driver, driver)) return false;
        }
        if (e.kind == MilesWire::BorrowedSample) {
            void *stream = 0;
            if (!resolve(e.parent, MilesWire::Stream, stream))
                return false;
        }
        out = e.local;
        return true;
    }
    bool beginClose(const MilesWire::Handle &h) {
        Entry *e = entry(h);
        if (!e || e->state != 1 || e->kind == MilesWire::BorrowedSample)
            return false;
        e->state = 2;
        return true;
    }
    bool retire(const MilesWire::Handle &h) {
        Entry *e = entry(h);
        if (!e || e->kind == MilesWire::BorrowedSample)
            return false;
        if (e->kind == MilesWire::Driver) {
            for (size_t i = 0; i < entries.size(); ++i) {
                Entry &child = entries[i];
                if ((child.state == 1 || child.state == 2 || child.state == 4) &&
                    (child.kind == MilesWire::OwnedSample || child.kind == MilesWire::Stream || child.kind == MilesWire::Video) &&
                    same(child.parent, h)) {
                    if (child.kind == MilesWire::Stream) {
                        const MilesWire::Handle stream = {MilesWire::Stream,
                            static_cast<uint32_t>(i + 1), child.generation};
                        retireBorrowed(stream);
                    }
                    retireEntry(child);
                }
            }
        }
        if (e->kind == MilesWire::Stream) retireBorrowed(h);
        retireEntry(*e);
        return true;
    }
};
inline ResourceRegistry::Reservation::~Reservation() {
    if (owner) owner->cancel(*this);
}
} // namespace MilesTransport
#endif
