#ifndef MILES_CALLBACK_INVOCATION_GUARD47_H
#define MILES_CALLBACK_INVOCATION_GUARD47_H

// Private client C++ boundary. Never used as an SDK callback exception ABI.
namespace MilesCallbackGuard47 {
struct ReentryDenied {};
class Scope {
public:
    Scope() throw();
    ~Scope() throw();
    bool admitted() const throw();
    bool violated() const throw();
private:
    Scope *previous_;
    bool violated_;
    Scope(const Scope &);
    Scope &operator=(const Scope &);
    friend void requireForwardAllowed();
};
// No allocation or shared-session access on the allowed path.
void requireForwardAllowed();
}
#endif
