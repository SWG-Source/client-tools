// ======================================================================
//
// engine_clientEffect_forward.h -- exe-local declaration for the Utinni
// Bucket B (2026-06-26) particle-effect live-preview retrigger.
//
// The Effects/ClientEffect editor's basic edit+save already works on the
// advertised client (managed I/O). What was dark is the live "Preview in
// client" retrigger -- hot-reloading a just-saved particle template and seeing
// live scene instances restart. ClientEffectManager owns the live instances
// (the private static m_particleSystems) but has no public enumerate-and-restart
// surface, so the provider exposes engine_retriggerClientEffect(), a friend of
// ClientEffectManager defined in ClientEffectManager.cpp. It is
// advertised to Utinni as "particlePreview::retrigger" -- a plain free function,
// so its address is a compile-time constant (a constant table row, NOT a
// dynamic ensureDynamicRowsFilled() row).
//
// CONTRACT (matches the consumer's utinni::ParticlePreview seam in
// particle_preview.h): game-thread-only; called ONCE per editor save/reload
// (never per frame); allocation-free on any per-frame path.
//
// EXE-LOCAL: included ONLY by engine_advertise.cpp. The friend grant lives in
// ClientEffectManager.h (ABI-neutral). Both platforms (x64 port 2026-08-15).
// ======================================================================

#ifndef INCLUDED_engine_clientEffect_forward_H
#define INCLUDED_engine_clientEffect_forward_H

// Walks ClientEffectManager::m_particleSystems and restarts every live particle
// instance whose appearance-template name matches logicalName (case-insensitive).
// Friend of ClientEffectManager; defined in ClientEffectManager.cpp.
void engine_retriggerClientEffect(char const * logicalName);

// Bucket B-2 (2026-06-28): re-plays a .cef fresh on the local player (with the
// just-saved edit) for live .cef authoring preview -- restart() only covers
// sustained, currently-playing instances; transient .cef effects are gone before
// save. Uses only public APIs (Game::getPlayer / ClientEffectManager::playClientEffect
// / ClientEffectTemplateList::fetch), so NOT a friend. Defined in
// ClientEffectManager.cpp; advertised as "particlePreview::replayClientEffect".
// Returns playClientEffect's result; false (no crash) if there is no player/scene.
bool engine_replayClientEffect(char const * clientEffectName);

#endif
