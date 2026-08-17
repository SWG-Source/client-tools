// ======================================================================
//
// ClientMain.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientMain_H
#define INCLUDED_ClientMain_H

// ======================================================================

int ClientMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

// ----------------------------------------------------------------------
// Utinni engine entry-point advertisement (37-01): a distinctly-named,
// external-linkage forwarding shim that calls the file-local crash-fixer
// ClientMainNamespace::installConfigFileOverride(). Named engine_* so it
// does NOT collide with the unqualified installConfigFileOverride() callers
// inside ClientMain.cpp (which compile under `using namespace
// ClientMainNamespace;`). engine_advertise.cpp wraps this in the
// config::loadOverrideConfig thunk row (EPA-02 crash-fixer).
// ----------------------------------------------------------------------
void engine_installConfigFileOverride();

// ----------------------------------------------------------------------
// Utinni hookpoint coverage self-check (37-01, EPA-04 seed). Defined in
// engine_advertise.cpp (Win32-only). Scans the advertised table for null
// addrs, duplicate names, and name-set equality vs the X-macro-generated
// required set; logs + returns false on any miss (never crashes). Called once
// on the Debug boot path (PRODUCTION == 0). On x64 the definition is compiled
// out (#if !defined(_WIN64)), so the ClientMain.cpp call site is likewise
// !_WIN64-gated.
// ----------------------------------------------------------------------
bool engine_verifyNoNullNoDup();

// ======================================================================

#endif
