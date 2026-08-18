// ======================================================================
//
// engine_chatWindow_forward.h -- exe-local declaration for the PRIVATE
// SwgCuiChatWindow construction-funnel address provider (24-§4 4d).
//
// The engine-hookpoint advertisement contract wants to expose the chat-window
// constructor as a DETOUR target (Utinni hkCtor tracks chat-window construction).
// You CANNOT take the address of a C++ constructor (`&Class::Class` is ill-formed;
// there is no ctor PMF), so the requested "real ctor entry" is infeasible at the
// source level. Instead we advertise the SOLE construction funnel: the static
// factory SwgCuiChatWindow::createNewWindow(UIPage&, Game::SceneType,
// std::string const&) [SwgCuiChatWindow.h:258] -- the only path to
// `new SwgCuiChatWindow` [SwgCuiChatWindow.cpp:1549]. Detouring createNewWindow's
// entry intercepts every chat-window construction, the same coverage the ctor
// detour would have given.
//
// createNewWindow is PRIVATE, so &SwgCuiChatWindow::createNewWindow cannot be
// named from engine_advertise.cpp (C2248). The accessor below is DEFINED in
// SwgCuiChatWindow.cpp (the class's own TU, friend-granted member access -- exactly
// the GroundScene private-method real-entry pattern in engine_groundScene_forward.h)
// and returns the REAL function entry. createNewWindow is STATIC, so its address is
// a plain function pointer (no multiple-inheritance PMF inflation, no delta) -- the
// accessor just returns (void *)&createNewWindow.
//
// EXE-LOCAL: included ONLY by engine_advertise.cpp. MUST NOT be pulled by any gl0X
// plugin TU -- the friend decl + accessor definition add NO SwgCuiChatWindow struct
// ABI change, but keeping this header exe-local avoids any shared-header ABI cascade
// (AGENTS.md). Both platforms (x64 port 2026-08-15; was 32-bit only).
// ======================================================================

#ifndef INCLUDED_engine_chatWindow_forward_H
#define INCLUDED_engine_chatWindow_forward_H

// Real-entry address provider for the PRIVATE static SwgCuiChatWindow::createNewWindow
// funnel. Defined in SwgCuiChatWindow.cpp (friend access to take the private &fn);
// returns the engine code entry Utinni DETOURS. Takes no args -- pure address provider.
void * engine_chatWindowCreateNewWindowEntry();

#endif
