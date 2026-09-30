#include "invocation_guard.h"
namespace MilesCallbackGuard47 {
namespace {
#if defined(_MSC_VER)
__declspec(thread) Scope *active = 0;
#else
thread_local Scope *active = 0;
#endif
}
Scope::Scope() throw() : previous_(active), violated_(active != 0) {
    // Unsupported nested invocation poisons every containing invocation,
    // even if its caller catches the rejection and returns normally.
    for (Scope *scope = previous_; scope; scope = scope->previous_)
        scope->violated_ = true;
    active = this;
}
Scope::~Scope() throw() { active = previous_; }
bool Scope::admitted() const throw() { return previous_ == 0; }
bool Scope::violated() const throw() { return violated_; }
void requireForwardAllowed() {
    if (!active)
        return;
    for (Scope *scope = active; scope; scope = scope->previous_)
        scope->violated_ = true;
    throw ReentryDenied();
}
}
