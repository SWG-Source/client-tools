// ======================================================================
//
// engine_hookpoints.h -- engine entry-point advertisement
// contract (handoff 2026-06-20). The exe-side game-logic twin of the
// shipped graphics gl11_r.dll!GetHookPoints (Direct3d11.cpp:856-888).
//
// Defines the name->pointer table structs an injected modding overlay
// reads via GetProcAddress(hExe, "GetEngineHookPoints") to detour
// / call / read engine functions and globals by NAME rather than by a
// hardcoded SWGEmu RVA. Pure read-only contract: each row is { name, addr },
// every address taken at compile time by &EngineSymbol so it is correct by
// construction and survives every rebuild. The client stays consumer-agnostic;
// if the consumer is not injected, nothing reads this table and it is inert.
//
// NOT VENDORED BY THE CONSUMER (corrected 2026-08-04). This header used to claim
// it was "shared verbatim with D:/Code/Utinni, copied at each catalog wave".
// That is wrong twice over: Utinni was SUNSET 2026-07-19, and SWG-Toolkit -- the
// sole consumer since -- does not vendor the .h/.inc at all. It resolves rows by
// CATALOG NAME ("group::name") from the table and keeps no copy of these files;
// its only artifact from here is ENGINE_HOOKPOINTS_VERSION. Consequences worth
// knowing before editing:
//   * renaming an exported C symbol is INVISIBLE to the consumer, hence free;
//   * changing a CATALOG STRING breaks that row SILENTLY on their side -- an
//     unresolved row is null-guarded by design and degrades to a no-op, so they
//     get neither a link error nor a crash. Keep catalog strings stable, or hand
//     over the old->new list (their standing request, 2026-08-04).
// It therefore carries ONLY the structs + version + the X-macro name list
// (.inc). It MUST NOT carry the provider-side exported GetEngineHookPoints()
// declaration -- a consumer repo must never import a dll-exported symbol. The
// export declaration+definition lives ONLY in the SwgClient-only TU
// engine_advertise.cpp.
//
// Keep this header EXE-LOCAL: it must not be added to any shared header the
// gl0X renderer plugins compile (AGENTS.md shared-header ABI cascade trap).
// It pulls in no engine library header by design.
//
// See .planning/handoff/2026-06-20-utinni-engine-entrypoint-advertisement-spec.md.
// ======================================================================

#ifndef INCLUDED_engine_hookpoints_H
#define INCLUDED_engine_hookpoints_H

// ----------------------------------------------------------------------
// Contract version. POLICY (D-03, Phase 38 -- overrides the 37-era "pinned at
// 1, do NOT bump per wave" note): any NAME ADD/REMOVE to the .inc bumps this
// version. 38-05 EXTENDS the policy: an ADDRESS-CORRECTNESS change behind an
// UNCHANGED name set ALSO bumps the version, because a consumer that has already
// bound an endpoint must know the address behind the name moved (here: 4 detoured
// rows re-pointed from a call-through forwarder to the real engine entry). The
// version is advisory -- the contract is name-keyed, so a consumer resolves
// endpoints by name regardless -- but the bump is the explicit "what's behind a
// name changed" signal -- and since 2026-07-19 it is the ONLY signal: there is no
// byte-level lockstep to enforce, because the consumer does not vendor the .h/.inc
// (see the header note above). The version is what tells them a re-sync is due.
//
// Bumped 1 -> 2 in 38-03: covered ALL the Phase-38 catalog GROWTH (38-01
// groundScene::* x8, 38-02 client/config x4, 38-03 cuiChatWindow::* x4) -- 94 names.
// Bumped 2 -> 3 in 38-05: ADDRESS-CORRECTNESS only -- the SAME 94-name set, but the
// 4 DETOURED rows (groundScene::{update,handleInputMapEvent}, cuiChatWindow::
// {enableTextInput,chatEnterHandler}) now advertise the REAL engine entry instead of
// a call-through forwarder thunk (a detour on a forwarder is silently dead). The
// consumer's required-set is UNAFFECTED -- only the addresses behind four names moved.
// Bumped 3 -> 4 in 24-§4 (MISC/INPUT editor-unlock batch): 3 NAME ADDs --
// game::g_mainLoopCounter (new out-of-line Game::getMainLoopCount accessor for ms_loops),
// treeFile::searchTree (&TreeFile::addSearchTree -- resolves the open/searchTree collision),
// cuiChatWindow::createNewWindow (&SwgCuiChatWindow::createNewWindow -- the sole ctor funnel;
// the requested raw-ctor real-entry is infeasible, you cannot address a ctor in C++). 97 names.
// Also an ADDRESS re-point under an UNCHANGED name: game::mainLoop now points at the per-frame
// Game::runGameLoopOnce (was the once-per-process Game::run).
// Bumped 4 -> 5 in 24-ENUM-A (editor file-pickers): 1 NAME ADD -- treeFile::enumerateFiles
// (&TreeFile::enumerateFiles -- new static that walks ms_searchNodes and yields every
// SearchTree/SearchTOC filename via a callback, so the consumer Repository can populate the
// editor pickers). 98 names. (The companion in-world CUI reflow fix is behavior-only -- no row.)
// Bumped 5 -> 6 in 24-SCENE-LOAD (editor "Load scene"): 1 NAME ADD -- game::loadScene (a
// __cdecl(terrain,player) thunk over Game::setScene(true,terrain,player,nullptr), the full
// SceneCreator lifecycle). game::setupScene -> _setScene(Scene*) only sets ms_scene and leaves
// a pre-built GroundScene half-integrated (next-frame throw); loadScene lets the engine build
// + integrate from filenames. 99 names. (game::setupScene stays mapped as the low-level
// set-pre-built-scene primitive -- unchanged.)
// Bumped 6 -> 7 in 24-§2.B (Bucket B, Effects editor live preview): 5 NAME ADDs --
// skeletalAppearance::getDisplayLodSkeleton (non-virtual LOD read, bit_cast PMF),
// renderWorld::addObjectNotifications (static &fn), bloom::preSceneRender +
// bloom::postSceneRender (static &fn), particlePreview::retrigger (friend free fn
// engine_retriggerClientEffect over ClientEffectManager::m_particleSystems -- the
// cooperative particle hot-reload entry). 104 names. The requested-but-NOT-advertisable
// rows are OMITTED/SKIPPED (documented in engine_advertise.cpp + the HANDBACK): the
// VIRTUAL render methods particleEffectAppearance::render + skeletalAppearance::render
// (consumer vtable-resolves, handoff §2.C), the un-addressable particleEffectAppearance
// ctor (no &Class::Class, no construction funnel), the NONEXISTENT skeletalAppearance::
// addShaderPrimitives + renderWorld::render, and the wildcard shaderPrimitiveSorter::*
// (no concrete method named). Render globals: consumer drives the draw via the already-
// advertised graphics::* statics (handoff's preferred shape) -- no raw-global rows added.
// Bumped 7 -> 8 in Bucket B-2 (live .cef RE-PLAY): 1 NAME ADD --
// particlePreview::replayClientEffect (free fn engine_replayClientEffect; re-plays a .cef
// FRESH on Game::getPlayer() via the public ClientEffectManager::playClientEffect, the
// transient muzzle/hit/explosion case the retrigger's restart() cannot cover). Constant
// &fn, public APIs only (not a friend). 105 names.
// Bumped 8 -> 9 in Bucket A (per-editor real-entry detour rows, ledger §2.A): 6 NAME ADDs --
// cuiRadialMenuManager::update + cuiMenu::infoTypesFindDefaultCursor (free fn) +
// systemMessageManager::receiveMessage (static &fn) + creatureObject::setTarget (pmfRealEntry,
// MI) + messageQueue::appendMessage + messageQueue::appendMessageData (pmfToVoid). 111 names.
// The other 8 of the 14 requested §2.A rows are OMITTED (un-addressable ctors cuiChatWindow/
// cuiLoginScreen -- chat's createNewWindow funnel already covers it; nonexistent
// cuiManager::findObjectUnderCursor + cuiHud::getTarget) or SKIPPED (virtual: cuiHud::
// actionPerformAction, cuiHud::update, cuiLoginScreen::activate, debugCamera::alter ->
// consumer vtable-resolves) -- documented in engine_advertise.cpp + the handback, NOT in
// this required-name set.
// Bumped 9 -> 10 in Bucket A-2 (world-pick / HUD-target): 2 NAME ADDs --
// cuiHud::getTarget (was a Bucket A OMIT; now the real entry of SwgCuiHud::
// getLastSelectedObject() const via a __fastcall MI thunk -- the world-picked object) +
// cuiHud::g_instance (static SwgCuiHudFactory::findMediatorForCurrentHud -> the LIVE
// ground/space hud the consumer calls getTarget on, mirroring cuiIo::g_instance). 113 names.
// Bumped 10 -> 11 in Bucket A-2.1 (crash revert): 1 NAME REMOVE --
// systemMessageManager::receiveMessage (Bucket A v9) is REVERTED. It was mapped to the static
// CuiSystemMessageManager::receiveSystemMessage(const ChatSystemMessage&), but the consumer's
// hkReceiveMessage detour targets the MessageDispatch::Receiver::receiveMessage(emitter,message)
// byte-stream receiver -- pointing that 2-arg network hook at the 1-arg static CRASHED world-load
// (c0000005 via sendFakeSystemMessage->receiveSystemMessage->hkReceiveMessage on region-enter). The
// real receiver is the file-local Listener::receiveMessage (no external symbol) -> un-advertisable;
// OMIT (a wrong & is worse than a missing row). 112 names.
// Bumped 11 -> 12 in Bucket A-3 (target-change resolver): 1 NAME ADD --
// network::getObjectById -> static NetworkIdManager::getObjectById(const NetworkId&) [the
// NetworkId->Object* lookup]. Unblocks the creatureObject::setTarget callback: the consumer's
// hkSetTarget resolves the new target id->Object* via Network::getObjectById, which on the
// advertised client hit a stale hardcoded RVA and crashed. constant &fn (true static). 113 names.
// Bumped 12 -> 13 in the FREE-CAM editor-unlock wave (2026-06-29): 6 NAME ADDs, all CALLED
// accessors that encapsulate fragile NGE struct byte-offsets (the particlePreview::retrigger /
// config::setModalChat shim pattern) so the consumer stops hardcoding offsets that drift:
//   groundScene::isFreeCameraActive               -- bool, currentView == CI_debugPortal (==5==consumer cm_Free)
//   groundScene::getDebugPortalCameraMessageQueue -- the MQ at m_debugPortalCameraInputMap (was read at InputMap+0xC)
//   gameCamera::getMessageQueue                   -- the camera's movement MQ via getController()->getMessageQueue() (was GameCamera+0x248)
//   messageQueue::getCount                        -- MISMATCH: real entry MessageQueue::getNumberOfMessages
//   messageQueue::getMessage                      -- the 4-arg (int,int*,float*,uint32*) overload (read side; appendMessage already advertised)
//   object::isActive                              -- bool; NON-virtual but INLINE -> external-linkage shim (no ODR address)
// All constant &thunk / &fn or pmfToVoid rows (no detours). 119 names.
// Bumped 13 -> 14 in the sysmsg-SEND provider request (2026-07-02): 1 NAME ADD --
// systemMessageManager::sendMessage -> &CuiSystemMessageManager::sendFakeSystemMessage (all-static class;
// plain constant &fn, no MI/virtual/thunk). The INJECT half of the sysmsg pair: a CALLED endpoint (the consumer
// injects a system message into the chat/system feed), byte-exact ABI match to the consumer typedef
// void(__cdecl*)(const Unicode::String&, bool). SEND ONLY -- the RECEIVE half stays OMIT (the reverted Bucket
// A-2.1 receiveMessage that AV'd world-load; the real inbound receiver is a file-local anonymous
// MessageDispatch::Receiver Listener, un-advertisable -- receiveSystemMessage next to the static is NOT it). 120 names.
// Bumped 14 -> 15 in sysmsg-SEND rev-2 (2026-07-03): 1 NAME REPLACE -- systemMessageManager::sendMessage
// REMOVED, systemMessageManager::sendMessageUtf8 ADDED -> extern "C" void __cdecl
// engine_sendFakeSystemMessage(const char* utf8Msg, bool chatBoxOnly), a provider-side utf8 shim that widens
// and calls sendFakeSystemMessage. The v14 row CRASHED live smoke (WRITE-AV): "byte-exact ABI match" was true
// of the signature only -- the const Unicode::String& PARAMETER crossed the boundary as a C++ object, and the
// consumer's swg::WString models the 2002 SWGEmu 3-pointer layout while ours is a v145 SSO basic_string ->
// the engine read _Mysize past the consumer's 12-byte object. ABI RULE adopted by both repos: only primitives
// and pointers cross the advertised boundary on CALLED endpoints; class-type params (strings above all) need
// an extern "C" shim (the engine_replayClientEffect precedent). Name-REPLACE, not re-point, so version-skewed
// pairings miss by name and degrade instead of mis-calling. Companion rev-2 AUDIT flagged the other CALLED
// rows with class-type params/returns as ABI-UNSAFE TO CALL until shimmed (row comments in
// engine_advertise.cpp): cuiChatWindow::writeToAllTabs + writeToCurrentTab and consoleHelper::sendInput
// (const Unicode::String& params), worldSnapshot::addObject (CrcString const& + std::string const&),
// objectTemplate::get{AppearanceFilename,PortalLayoutFilename,ClientDataFile} (return our std::string by
// const&). DETOURED rows are unaffected (they read args our own code constructed). Still 120 names.
// Bumped 15 -> 16 in the lookAtTarget-accessor request (2026-07-09): 1 NAME ADD --
// game::getPlayerLookAtTargetId -> extern "C" __int64 __cdecl engine_getPlayerLookAtTargetId(void)
// (defined in CreatureObject.cpp), the READ twin of creatureObject::setTarget: returns the PLAYER's
// lookAt/selection-target NetworkId VALUE (full 64 bits, cluster-id bits included; 0 = no player /
// no target -- NetworkId::cms_invalid is NetworkId(0)). Strictly the lookAt target (m_lookAtTarget),
// NOT the NGE intended/combat target (getIntendedTarget -- a future separate request, never a
// re-point). Shim per the rev-2 ABI RULE: CreatureObject::getLookAtTarget() is INLINE (no ODR
// address) and returns const CachedNetworkId& (embeds a mutable Watcher<Object> the consumer does
// not model) -- the shim collapses it to a primitive in EDX:EAX. Unblocks the consumer's
// Game::getPlayerLookAtTargetObject() reroute through the v12 network::getObjectById row ->
// target-aware editor affordances on the advertised client. 121 names.
// Bumped 16 -> 17 in Goal B Wave 1 (snapshot-editor READ wave, rev-3 freeze
// 2026-07-15): 7 NAME ADDs -- worldSnapshot::{wsGetNodeCount, wsGetTopNodeIdAt,
// wsGetChildCount, wsGetChildIdAt, wsGetNodeInfo, wsGetNodeTemplateName,
// wsGetGeneration}, all extern "C" __cdecl shims DEFINED in clientGame
// WorldSnapshot.cpp (the ms_reader file-local-singleton TU; the engine_ws*
// symbol names are the rev-3 frozen export names). Id-keyed read/browse of the
// CURRENT scene's live snapshot for the Utinni snapshot editor: enumeration is
// live, AUTHORED-ONLY (tombstones + buildout-provenance rows are never
// enumerated -- the retained ms_buildoutObjects filter, ANSWERS 5.1a/b); node
// reads force-finish the CONSULT-60 incremental parse (the finishLoadNow
// mutator discipline); wsGetGeneration is a PURE counter read (no parse force
// -- pollable during loading without re-synchronizing the load). Introduces
// the shared POD EngineWsNodeInfo (below, rev-3 FROZEN: 80 bytes, size-first
// protocol, childCount included). 128 names.
// Bumped 17 -> 18 in Goal B Wave 2 (snapshot-editor LIVE-ONLY mutation wave,
// frozen 2026-07-18; Wave-1 live smoke PASSED): 5 NAME ADDs --
// worldSnapshot::{wsAddObject, wsAddNodeAt, wsRemoveNode, wsSetNodeRadius,
// wsConfigureIdAllocator}, extern "C" __cdecl shims in WorldSnapshot.cpp.
// Explicitly NON-PERSISTENT (nothing touches disk; persistence = Wave 3).
// Headline semantics (the accepted ANSWERS 5.2/5.3/5.5 + Wave-2 deltas):
// wsAddObject pre-validates EVERYTHING before minting (origin guards,
// template resolvable, contiguous id range free vs reader + buildout set +
// NetworkIdManager + band, live container) -- a failed add never mutates;
// mints id..id+cellCount with atomic POB cell expansion and routes through
// the FULL streamed-create bookkeeping (sphere handle + in-world mark +
// loaded-list), spawning immediately. wsAddNodeAt is the undo-replay
// primitive: data re-add at the EXPLICIT id; top-level gets a sphere handle
// + dirties the update-diff sentinels (stationary-player starvation fix);
// a child under a spawned in-world parent spawns immediately (no engine
// path ever would); fail-closed on id-present/live-object/band/buildout/
// missing-container. wsRemoveNode is the 7-step occupancy-guarded subtree
// teardown: 1 removed / 0 miss / -1 OCCUPIED (a non-client-cached object
// -- e.g. the player -- inside the containment subtree; Container's dtor
// cascade-deletes contents, so delete is refused, never silent). 133 names.
// Bumped 18 -> 19 in Goal B Wave 3 (persistence + riders, frozen 2026-07-18;
// Wave-2 mutation smoke PASSED): 7 NAME ADDs. Persistence (the disk half):
// worldSnapshot::{wsSaveSnapshot, wsGetSavePath, wsUnloadSnapshot} --
// authored-only save (tombstone-skip recursive + retained buildout-set
// filter; the above-ceiling AUTHORED collection-item nodes of the
// TOC-resolved .ws copies SERIALIZE by design -- they are real world
// content, see the idmint-CLOSED handoff), absolute destination in the
// winning loose SearchPath, negative-cache invalidation, post-write shadow
// verification, typed result enum (0 ok / 1 no-snapshot / 2 no-loose-path /
// 3 shadowed / 4 id-overflow / 5 buildout-integrity / 6 write-fail);
// wsUnloadSnapshot also resets the sticky ms_sceneName so reload works.
// Rider 4B: cuiPreferences::{setAllowTargetAnything, getAllowTargetAnything}
// -- the NGE world-pick/target gate (SwgCuiHud + CuiRadialMenuManager),
// plain statics, replaces the consumer's corrupting SWGEmu RVA patch.
// Rider 4C: camera::{getProjectionMatrix, getTransformO2W} -- copy-out
// matrix accessors for the gizmo (GlMatrix4x4 verbatim / row-major 3x4
// position-col-3), replaces raw struct-offset camera reads that crash on
// the NGE layout. 140 names.
//
// v19 -> v20 (2026-07-19, SWG-Toolkit live-editor change request):
// clientWorld::collideScreenRay -- engine-side copy-out cursor ray-pick
// (screen pixel -> nearest-hit NetworkId + world point; the consumer's
// collideCursorWithWorld done provider-side so CollisionInfo /
// ClientWorld::collide's CollideParameters / camera viewport math never
// cross the boundary; terrain hit = id 0 + valid point, the place-at-cursor
// case). Plus cuiRadialMenuManager::clear -- the rider pre-approved in the
// 2026-07-18 positionchanged ANSWER. 142 names.
//
// v20 -> v21 (2026-07-19, SWG-Toolkit change request #2): game::getSceneId
// -- current-scene-id copy-out (wsGetSavePath convention: needed length
// INCLUDING NUL; 0 = no scene). Game::getSceneId() is inline + returns
// const std::string& -> shim mandatory (ABI RULE). Enables one-click
// "Reload current scene" (unload + load(getSceneId())) and .ws
// auto-naming. 143 names.
//
// v21 -> v22 (2026-07-19, SWG-Toolkit change request #3):
// clientWorld::collideScreenRayObject -- borrowed-Object* sibling of the
// v20 ray-pick. The CONSULT-69 layer probe MEASURED that pure .ilf interior
// decorations never reach the hud pick (cuiHud::getTarget = null for an
// id-less table the ray demonstrably hits), so watcher-keyed selection
// misses that layer; this row returns the RAW nearest-hit Object* (no
// ancestor walk, no id resolution; borrowed, game-thread-only, cleared by
// the consumer on cell/zone change). Unblocks gizmo manipulation of
// id-less .ilf decoration. 144 names.
//
// v22 -> v23 (2026-07-19, SWG-Toolkit change request #4):
// worldSnapshot::wsSetNodeTemplateName -- in-place template re-point for an
// authored .ws node (OTNL name intern + index swap; subtree/id/transform/
// crc untouched). The CONSULT-70 lossless-rebind fix for per-instance
// interior editing (model D: derived template overriding
// interiorLayoutFileName + edited .ilf, both loose) -- remove+re-add would
// tombstone the building's whole authored subtree and churn the id.
// Fail-closed: authored non-buildout nodes; the new template must resolve
// via TreeFile at call time (negative cache invalidated first). 145 names.
//
// v23 -> v24 (2026-07-19, SWG-Toolkit change request #5):
// object::getTransformO2P -- copy-out o2p read on a borrowed consumer-held
// Object* (the .ilf stores o2p; the gizmo drives o2w). getTransform_o2p()
// is inline + const Transform& -> shim mandatory (ABI RULE); layout =
// camera::getTransformO2W byte-for-byte (row-major 3x4, position col 3).
// The last accessor of the model-D persist flow. 146 names.
//
// v24 -> v25 (2026-07-30, SWG-Toolkit change request #6):
// object::getContainingBuildingId -- copy-out containing-POB-building
// NetworkId for a borrowed consumer-held Object* (an .ilf decoration, a
// cell, the player). The whole chain is inline / reference-returning ->
// un-advertisable directly (ABI RULE): cell = the object's OWN
// CellProperty if it IS a cell (the wall-click fallback resolves to the
// CELL object -- getParentCell on a cell walks ANCESTORS and lands in the
// world cell, which would wrongly report "not inside"), else
// getParentCell() [never null -- world-cell fallback, Object.cpp:1372];
// -> getPortalProperty() [CellProperty.h:119, null for the world cell]
// -> getOwner() [Property.h:34, the building] -> getNetworkId().getValue().
// 0 = null arg / not inside a POB. Unblocks the model-D Arm step: building
// id -> getObjectById -> getObjectTemplateName = the template carrying
// interiorLayoutFileName. 147 names.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// v26 (2026-08-01): + game::getShutdownPhase -- process-wide MONOTONIC
// shutdown phase for attached consumers (0=running, 1=quit requested,
// 2=ExitChain unwinding). Closes a real gap: the contract had game::quit and
// game::cleanupScene (both IMPERATIVE -- the consumer CAUSES teardown) but no
// NOTIFICATION that unwind has begun. SWG-Toolkit's safety was emergent from
// Present-hook ordering (the main loop is over before ExitChain runs, so
// Present stops firing first), not promised -- it breaks the moment any
// consumer thread calls an advertised row directly instead of queueing.
// The two obvious candidates do NOT work: ExitChain::isRunning() reads
// PerThreadData (per-THREAD -- a consumer thread always sees false), and
// game::g_runningFlags (&Game::isOver) dereferences IoWinManager/Os state so
// it goes unsafe exactly when teardown starts. Backed by a plain process-wide
// int in sharedFoundation -- safe to read from any thread at any time,
// including CRT teardown. 148 names.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// v27 (2026-08-02): + object::setParentCell, + cellProperty::getWorldCellProperty
// (SWG-Toolkit change request 2026-08-02). Writing a world transform alone
// leaves the object in the WORLD cell, so the engine evaluates the outdoor
// portal set from a position that is physically indoors -> interiors render
// see-through from inside; a portal crossing repairs it, and a scene load
// triggers it too. Beyond teleport, this backs PLACEMENT ROUTING: the
// container is resolved from the PLACEMENT POINT, never the player, so
// standing in a cantina doorway must place OUT to .ws and IN to .ilf -- the
// doorway is the acceptance test in both directions.
// setParentCell is virtual -> shim; getWorldCellProperty is an out-of-line
// static -> plain &fn row, and it is REQUIRED because setParentCell NOT_NULLs
// its argument (null is a FATAL, not "outside"). 150 names.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// v28 (2026-08-02): +5 rows answering the toolkit's post-v27 field report.
//   cellProperty::setPortalTransitionsEnabled -- suppress the portal sweep
//     across a teleport write. THE row: it lets a consumer use the engine's own
//     idiom (GroundScene.cpp:1492-1497) instead of reasoning about sweep
//     ordering at all. Global state -- always re-enable.
//   collisionWorld::objectWarped -- completes that same idiom (:1497).
//   clientWorld::findCellAtWorldPosition -- "which cell contains world point P".
//     THE placement-routing primitive: a coordinate-only destination (bookmark,
//     scripted placement) has no object to pick, so the collideScreenRayObject
//     -> getParentCell path cannot serve the doorway test. Never null.
//   object::getAttachedTo -- SAFETY. setParentCell on a MOUNTED player silently
//     corrupts pose in Release (the isChildObject DEBUG_FATAL at Object.cpp:1396
//     is #if 0'd). Non-null = do not reparent.
//   worldSnapshot::wsIsParsePending -- PURE, non-forcing completion read, so a
//     consumer can WAIT instead of calling a forcing row for its side effect and
//     paying the whole remaining ~3.1s synchronous parse.
// 155 names.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// v29 (2026-08-03): + object::isChildObject. CORRECTS v28's §1.4 guidance,
// which was falsified live by the consumer. We advertised object::getAttachedTo
// as the mount guard ("non-null means do not reparent"); it refused every
// teleport made from INSIDE a building, because cell parentage and mount
// attachment share m_attachedToObject -- Object::setParentCell attaches via
// attachToObject_w(&cellProperty->getOwner(), false) (Object.cpp:1404-1405), so
// a player merely standing in a POB has a non-null getAttachedTo (the cell
// owner). m_childObject is the actual discriminator: set only from the
// asChildObject argument (Object.cpp:1931), and cells attach with FALSE. It is
// what the compiled-out DEBUG_FATAL(isChildObject()) at Object.cpp:1396 tests --
// the assert that would otherwise catch the silent mounted-player pose
// corruption. getAttachedTo stays advertised for reading the actual parent.
// 156 names.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// v30 (2026-08-03): + playerCreatureController::warpClient. A consumer writing
// the player's transform directly (object::setTransform_o2w) performs an
// UNSEQUENCED local write the server never hears about, so the next
// authoritative update legitimately overwrites it -- observed live as "lands,
// then yanked back after ~1s" on a server session and NOT offline, because
// offline nothing corrects it. warpClient is the engine's own client-initiated
// teleport (its other caller is DebugPortalCamera.cpp:314): it stamps
// m_previousTransform_p, mints a CLIENT-side sequence number, and sends
// CM_netUpdateTransform with SEND|RELIABLE|DEST_AUTH_SERVER|DEST_AUTH_CLIENT,
// so the server is told AND the local apply runs through
// handleNetUpdateTransform -- which brings CollisionWorld::objectWarped and the
// FreeChaseCamera retarget with it. NOTE the server->client teleport path
// (handleNetUpdateTransform -> ackTeleport) is the opposite direction: its
// sequence number comes from the server and the client only ACKs, so a client
// cannot fabricate it. Shim takes WORLD coords, converts to parent space, and
// does NOT reparent -- set the cell first. 157 names.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// v31 (2026-08-03): NO name change -- BEHAVIOUR CORRECTION behind
// playerCreatureController::warpClient, bumped per the address-correctness
// policy above. The v30 implementation was wrong in a way only a live server
// could reveal, and the consumer reproduced it immediately.
//
// warpClient sends MessageQueueDataTransform / CM_netUpdateTransform -- the
// NO-PARENT variant -- and its handler (ClientController.cpp:433) opens with
// "transfer from cell to world": it calls setParentCell(getWorldCellProperty())
// whenever the object is attached. That message STRUCTURALLY UN-PARENTS by
// design, so warpClient can never place a player inside a cell, and the
// setParentCell we told the consumer to call first was undone by the local
// apply. Compounding it, the v30 shim converted world -> cell space and then
// shipped the result in the world-space message (target (3448,4,-4824) put the
// player at (15,193,5), the cell-relative magnitudes at world origin).
//
// The shim now performs the engine's own sequence and resolves the destination
// cell ITSELF: findClosestCellObjectFromWorldPosition -> setParentCell ->
// suppressed setTransform_o2p -> CollisionWorld::objectWarped ->
// ClientController::sendTransform(transform_p, reliable). sendTransform
// [ClientController.h:44 / .cpp:311-340] picks the message from parentage --
// MessageQueueDataTransformWithParent + CM_netUpdateTransformWithParent with
// the CELL id when attached, plain otherwise. That was the piece a bare local
// write was always missing.
//
// CONSUMER CONTRACT CHANGE: do NOT call object::setParentCell around this any
// more -- the shim owns cell resolution, and an external reparent will fight
// the message-variant choice. Still 157 names.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// v32 (2026-08-04): +3 rows, the consolidated placement request. 160 names.
//
//   worldSnapshot::wsForgetNode -- drop a node from the snapshot WITHOUT
//     despawning the live Object. The consumer's placement gesture mints a
//     preview node, and on Persist the row must leave the .ws (a copy there is a
//     second, world-space instance of the same decoration -- they MEASURED two
//     such 84-byte children of building 1082874, and the engine cannot dedupe
//     them because .ilf-created objects never get a NetworkId so
//     CEC_objectAlreadyExists can never fire). But wsRemoveNode is TEARDOWN, so
//     using it made the object the modder just placed VANISH at the moment they
//     saved it. This is the missing half. No occupancy guard: nothing is deleted.
//     Forgetting does NOT free the id -- wsAllocateIdRange's collision test
//     consults NetworkIdManager and the Object is still registered.
//
//   clientInteriorLayoutManager::refreshInteriorLayout -- re-apply a changed .ilf
//     to ONE building, no scene reload. This RETIRES the kept-root staleness
//     residual the unload guard introduced: nothing is torn down, so nothing is
//     kept, so nothing goes stale. Three steps, and missing any one is a silent
//     no-op: delete ONLY the client-only interior objects (NOT every client-cached
//     object in the cells -- that would sweep the consumer's unpersisted
//     placements, which have no on-disk copy); reload the TEMPLATE's cached
//     InteriorLayoutReaderWriter (the layout is cached on
//     ClientBuildingObjectTemplate, not the object, so a latch/cursor reset alone
//     rebuilds the PRE-EDIT file) after TreeFile::forgetMissingFile; then clear
//     each cell's applied-latch AND resume cursor. Re-creation is left to the
//     existing budgeted update(), so a large cantina spreads across frames.
//
//   cellProperty::getCellName -- COPY-OUT of the inline getCellName(). The
//     consumer's .ilf stores the cell name as a literal string and they were
//     writing an operator-typed one, so a wrong-cell row looked healthy on disk.
//     Copy-out rather than pointer-return because m_cellName points into the
//     TEMPLATE (CellProperty.cpp:456). World cell returns "world", not null.
//
// Bumped 32 -> 33 in the 2026-08-04 scene-teardown fix: NO name change (still 160),
// BEHAVIOUR correction behind game::loadScene -- the same address-correctness policy
// that took v30 -> v31 for warpClient. The shim now destroys the outgoing GroundScene
// before installing the new one, which every other by-name scene installer in the
// engine already did (GameNetwork.cpp:483-492, SwgCuiCommandParserScene.cpp:271-283,
// SwgCuiLocations.cpp:171-177). CONSUMER-VISIBLE CONSEQUENCE, which is why this bumps:
// any Object*/CellProperty* cached from the previous scene is now genuinely DELETED
// across a loadScene call. Previously the outgoing scene leaked, so such pointers
// stayed readable and stale rather than dangling -- code that got away with holding
// them will now fault, and should re-resolve after every loadScene. Calling your own
// close+delete first is still safe: the shim no-ops when getScene() is already null.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// Bumped 33 -> 34 in the DUAL-ARCH wave (2026-08-15, answering the consumer's
// x64 CHANGE-REQUEST). The headline is not a name change: the WHOLE surface now
// builds and exports on x64 as well (identical 162-name set on both arches;
// binding stays by GetProcAddress NAME -- the x64 ordinal happens to land at 82
// like Win32, but nothing may rely on ordinals). Two NAME ADDs:
//   client::advertisedArchBits -- extern "C" int __cdecl (void), returns the
//     provider pointer width in bits (32/64), compile-time per-arch. Ask #5:
//     the consumer's export-absent fallback seeds x86 SWGEmu RVA literals and
//     there is NO legacy x64 client, so their x64 agent fails closed on a
//     missing export -- and asserts the arch from the table itself when present.
//   object::setScale -- &Object::setScale via pmfToVoid (non-virtual,
//     out-of-line [Object.h:228 / Object.cpp:2205]; Vector is a 3-float POD by
//     const&, the setPosition_w boundary shape). Ask #4 / their D-09 gap: their
//     rva_table deliberately seeds nullptr for it pending this row.
// x64 ABI facts consumers need (full detail in the 2026-08-15 dual-arch
// PROVIDER-HANDBACK): __thiscall consumer typedefs work UNCHANGED on x64 (the
// keyword is inert; this in RCX); the Win32 __fastcall dummy-EDX thunks take NO
// dummy on x64 (the same __thiscall typedef matches both, by construction);
// version/count stay 32-bit; entries sits at offset 8 in EngineHookPoints on
// BOTH arches; EngineHookPoint rows are 2 pointers (8 bytes Win32 / 16 x64);
// EngineWsNodeInfo keeps the identical frozen 80-byte layout. 162 names.
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// Bumped 34 -> 35 in the NO-DETOUR OVERLAY wave (2026-08-15, answering the
// consumer's x64 round-2 CHANGE-REQUEST: DetourXS proved x86-only by
// construction, so every consumer vtable/prologue patch is an x64 casualty).
// Three NAME ADDs -- callback REGISTRATION rows that replace the consumer's
// three detours with provider-invoked callbacks, stable on both arches:
//   graphics::registerFrameCallback -- void (void (__cdecl*)(void)). Fires on
//     the render thread inside the D3D11 present path, AFTER the provider's
//     last back-buffer write (the BCG/gamma pass), BEFORE the swap-chain
//     Present -- byte-for-byte where the consumer's IDXGISwapChain::Present
//     vtable patch used to draw. Retires that patch on BOTH arches.
//   graphics::registerResizeCallback -- void (void (__cdecl*)(int phase,
//     int width, int height)). Fires around the D3D11 back-buffer resize:
//     phase 0 BEFORE ResizeBuffers (the consumer MUST release every
//     back-buffer-referencing view -- an outstanding reference fails
//     ResizeBuffers), phase 1 after the new views exist (rebuild). Replaces
//     the ResizeBuffers vtable patch.
//   game::registerTickCallback -- void (void (__cdecl*)(void)). Fires once at
//     the TOP of Game::runGameLoopOnce, game thread, OUTSIDE any render call
//     chain (previous frame fully presented) -- the deferred-command drain
//     point. Replaces the consumer's detour on the advertised game::mainLoop
//     row (which stays advertised unchanged).
// Shared slot rules: single-slot (one consumer; last-write-wins), null
// clears, register from a LIVE session (after graphics install -- an early
// call WARNs and drops). The graphics pair rides new Gl_api tail slots
// (setFrameCallback/setResizeCallback): D3D11 invokes; the D3D9 plugins
// accept-and-ignore with a Release-visible line (the overlay is D3D11-only).
// 165 names.
// ----------------------------------------------------------------------
#define ENGINE_HOOKPOINTS_VERSION 35

// ----------------------------------------------------------------------
// One row per advertised endpoint: a stable contract name + the borrowed
// address of the engine function/global (or a thunk wrapping it).
// ----------------------------------------------------------------------
struct EngineHookPoint
{
	const char * name;   // stable contract identity, e.g. "config::loadOverrideConfig"
	void *       addr;   // &EngineSymbol (or thunk) -- borrowed, process-lifetime
};

// ----------------------------------------------------------------------
// The advertised table. Returned by GetEngineHookPoints() as a pointer to
// a process-lifetime static; the consumer only reads it. No NUL-name sentinel --
// count is sizeof/sizeof of the row array (see engine_advertise.cpp).
// ----------------------------------------------------------------------
struct EngineHookPoints
{
	unsigned int                 version;   // == ENGINE_HOOKPOINTS_VERSION at build time
	unsigned int                 count;     // number of rows in entries[]
	const EngineHookPoint * entries;  // static array of `count` rows
};

// ----------------------------------------------------------------------
// EngineWsNodeInfo -- the worldSnapshot::wsGetNodeInfo POD-out struct
// (Goal B Wave 1, v17; FROZEN by the 2026-07-15 rev-3 request §2 -- any
// layout change is a NEW version wave, never an in-place edit).
//
// Size-first protocol: the CALLER sets `size = sizeof(EngineWsNodeInfo)`
// (its compiled-against size) BEFORE the call; the provider writes
// min(callerSize, providerSize) bytes and never assumes growth, so the
// struct may gain trailing fields in later versions without breaking a
// version-skewed pairing. Fixed-width members only (no bool/enum/size_t).
// Provider real == float and Transform == row-major 3x4 with position in
// column 3 are verified provider-side (ANSWERS 5.4).
// ----------------------------------------------------------------------
struct EngineWsNodeInfo
{
	unsigned int size;            // offset  0 -- caller fills FIRST (size-first protocol)
	unsigned int flags;           // offset  4 -- bit0 = deleted (reads 0 in practice: tombstones
	                              //              answer miss); bit1 = buildout (RESERVED, always 0
	                              //              in v1, against the ANSWERS 5.1a enumeration extension)
	__int64      containedById;   // offset  8 -- 0 = top-level
	int          cellIndex;       // offset 16
	unsigned int portalLayoutCrc; // offset 20
	float        radius;          // offset 24
	float        transform[12];   // offset 28 -- row-major 3x4, position = column 3
	int          childCount;      // offset 76 -- rev-3 addition: enumerable (non-tombstone)
	                              //              direct-child count (same value as wsGetChildCount)
};                                // sizeof == 80 (x86 MSVC: __int64 8-aligned at offset 8)

#endif // INCLUDED_engine_hookpoints_H
