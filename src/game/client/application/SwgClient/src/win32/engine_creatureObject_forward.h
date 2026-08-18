// ======================================================================
//
// engine_creatureObject_forward.h -- exe-local declaration for the Bucket A
// (2026-06-28) creatureObject::setTarget real-entry address provider.
//
// engine_advertise.cpp cannot #include clientGame/CreatureObject.h: that header
// transitively pulls sharedSkillSystem/SkillObjectArchive.h, whose include dir is
// NOT on the SwgClient (exe) project's include path (it is on clientGame's). So the
// PMF real-entry of CreatureObject::setLookAtTarget is computed by an out-of-line
// accessor compiled in CreatureObject.cpp (the class's own TU, where the header
// builds), exactly like the engine_groundScene*RealEntry() accessors live in
// GroundScene.cpp. This header only DECLARES it.
//
// WHY setLookAtTarget: there is NO CreatureObject::setTarget in this tree; the
// "current target" setter is the PUBLIC non-virtual setLookAtTarget(const NetworkId&)
// [CreatureObject.h:311] (m_lookAtTarget = "this creature's current target"). The
// contract name stays creatureObject::setTarget (the consumer's lookup key); this is
// the same MISMATCH-name mapping used elsewhere in the table.
//
// REAL ENTRY (detoured row): CreatureObject is MULTIPLE-INHERITANCE
// (TangibleObject : public ClientObject, public CallbackReceiver), so &CreatureObject::
// setLookAtTarget is an inflated PMF { void* pfn; int delta; }. setLookAtTarget is an
// OWN method of the most-derived class -> primary base at offset 0 -> delta==0 and pfn
// is the real engine entry the engine's own call path reaches (the address Utinni must
// DETOUR). The accessor hard-gates delta==0 and returns nullptr otherwise, so
// engine_verifyNoNullNoDup() fails loudly rather than advertising a wrong entry.
//
// EXE-LOCAL: included ONLY by engine_advertise.cpp. setLookAtTarget is PUBLIC, so the
// accessor needs no friend grant and CreatureObject.h is UNCHANGED (no shared-header
// ABI cascade). Both platforms (x64 port 2026-08-15): the definition is in
// CreatureObject.cpp, matching the whole advertise body.
// ======================================================================

#ifndef INCLUDED_engine_creatureObject_forward_H
#define INCLUDED_engine_creatureObject_forward_H

// Returns the REAL engine code entry of CreatureObject::setLookAtTarget (delta==0
// verified), the address Utinni detours for the creatureObject::setTarget row.
// Defined in CreatureObject.cpp.
void * engine_creatureSetTargetRealEntry();

// game::getPlayerLookAtTargetId (2026-07-09 request, v16): CALLED extern "C" shim
// returning the player's lookAt/selection-target NetworkId VALUE (full 64 bits;
// 0 = no player / no target). The read twin of the setTarget row above -- same
// m_lookAtTarget slot, NOT the NGE intended/combat target. Primitive-only
// boundary (the sysmsg rev-2 ABI RULE): getLookAtTarget() is INLINE and returns
// const CachedNetworkId& (embeds a Watcher the consumer does not model), so the
// shim collapses it to __int64 in EDX:EAX. Defined in CreatureObject.cpp
// (the exe TU cannot include CreatureObject.h -- see the header comment above).
// Consumer typedef: int64_t(__cdecl*)()
extern "C" __int64 __cdecl engine_getPlayerLookAtTargetId(void);

// playerCreatureController::warpClient (v30): CALLED extern "C" shim performing a
// CLIENT-INITIATED teleport through the CONTROLLER path, so the move is sequenced
// and the server is told. A direct object::setTransform_o2w write is unsequenced
// and the next authoritative server update overwrites it ("yanked back after ~1s"
// on a live session; no revert offline). Takes WORLD coords and converts to
// parent space internally; does NOT reparent -- set the cell first.
// 1 = ok / 0 = no player / -1 = no PlayerCreatureController.
// Defined in PlayerCreatureController.cpp (the exe TU cannot include
// CreatureObject.h -- see the header comment above).
// Consumer typedef: int(__cdecl*)(float,float,float)
extern "C" int __cdecl engine_warpPlayer(float x_w, float y_w, float z_w);

#endif
