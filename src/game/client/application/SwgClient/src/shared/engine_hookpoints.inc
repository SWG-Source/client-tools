// ======================================================================
//
// engine_hookpoints.inc -- canonical X-macro name list for the
// engine entry-point advertisement contract. This file is the single source of
// truth for the set of advertised endpoint NAMES.
//
// NOT VENDORED BY THE CONSUMER (corrected 2026-08-04 -- this file used to claim it
// was "shared verbatim with D:/Code/Utinni, re-copied at each catalog wave").
// Utinni was sunset 2026-07-19; SWG-Toolkit binds by CATALOG NAME and keeps no copy
// of this file. So the names below ARE the contract surface: a catalog string change
// silently no-ops that row on their side. See engine_hookpoints.h for the full note.
//
// Format: ENGINE_HOOKPOINT(group, name) -- the contract name is "group::name".
// The includer #defines ENGINE_HOOKPOINT before #include-ing this file and
// #undef's it after, expanding it to (a) the table rows on the provider side,
// (b) the required-name set + the coverage gate on the consumer side, and
// (c) the s_requiredNames[] / count self-check in engine_advertise.cpp.
//
// 37-01 spike: the 3 representative rows below prove the full mechanism --
// a crash-fixer thunk (config), a static fn (game), and a non-virtual member
// PMF (commandParser). 37-02/37-03 EXTEND this file with the MVP / full
// catalog. The table BODY in engine_advertise.cpp stays hand-written (rows
// are heterogeneous: static fn / bit_cast PMF / ctor thunk / accessor) and
// the coverage self-check bridges this name list to that body.
//
// See .planning/handoff/2026-06-20-utinni-engine-entrypoint-advertisement-spec.md.
// ======================================================================

// ----------------------------------------------------------------------
// 37-02 MVP required set (~boot -> render-overlay -> scene-change repro path).
// Groups: config, client, game, graphics, scene::groundScene(clean rows only),
// cui::manager, cui::io, consoleHelper, commandParser. Each name here has a
// source-confirmed, signature-checked row in engine_advertise.cpp (the table
// BODY stays hand-written: rows are heterogeneous -- static fn / bit_cast PMF /
// accessor). The runtime engine_verifyNoNullNoDup() name-set-equality check
// bridges this list to that body (EPA-04). MISMATCH corrections (RESEARCH "Spec best-guesses that are WRONG")
// are baked into the table rows, NOT here -- this list carries CONTRACT names.
//
// OMITTED / DEFERRED-to-37-03 MVP names (recorded in 37-02-SUMMARY, not advertised):
//   config::setModalChat/getModalChat (unresolved symbol), game::g_mainLoopCounter
//   (only an INLINE accessor Game::getLoopCount -> no ODR-emitted address, Pitfall 2),
//   client::{wndProc,writeMiniDump,writeCrashLog,setupStartDataInstall} (file-local
//   exe statics -> each needs an external-linkage shim, deferred), cui::manager::
//   findObjectUnderCursor (not on CuiManager), ALL groundScene::* rows (the class
//   is MULTIPLE-INHERITANCE via NetworkScene -> inflated PMFs fail the sizeof guard;
//   even reloadTerrain/getCurrentCamera/setView need a __thiscall thunk, 37-03),
//   cui::io::processEvent
//   (VIRTUAL -> the consumer resolves off the live vtable), cui::chatWindow::* +
//   chatWindow::ctor (MULTIPLE-INHERITANCE class -> inflated PMF fails the
//   sizeof(PMF)==sizeof(void*) guard -> needs __thiscall thunks, 37-03),
//   commandParser::{ctor1,ctor2}(thunk) + createDelegateCommands(PROTECTED).
// ----------------------------------------------------------------------

// -- config (sharedFoundation ConfigFile statics + 38-02 CuiPreferences statics) --
// setModalChat/getModalChat are the 37-02 CORRECTION: they exist as PUBLIC
// CuiPreferences statics (NOT config/ConfigFile) -- the row comments in
// engine_advertise.cpp carry the class MISMATCH; this list keeps the CONTRACT
// names.
ENGINE_HOOKPOINT(config,        loadOverrideConfig)
ENGINE_HOOKPOINT(config,        loadConfigFileBuffer)
ENGINE_HOOKPOINT(config,        loadConfigFileString)
ENGINE_HOOKPOINT(config,        setModalChat)
ENGINE_HOOKPOINT(config,        getModalChat)

// -- client (exe entry + win32 exe-statics; ClientMain.h / Os.cpp / DebugHelp.cpp) --
// wndProc/writeMiniDump (38-02) are external-linkage shims over the PRIVATE
// Os::WindowProc (__stdcall) and DebugHelp::writeMiniDump (win32-private header).
// writeCrashLog + setupStartDataInstall are NONEXISTENT in this tree (grep = 0
// hits) -> OMITTED here + in the .cpp, flagged for the EPA-08 handback.
ENGINE_HOOKPOINT(client,        clientMain)
ENGINE_HOOKPOINT(client,        wndProc)
ENGINE_HOOKPOINT(client,        writeMiniDump)
ENGINE_HOOKPOINT(client,        advertisedArchBits)   // v34: int (void) -- provider pointer width (32/64); the consumer's fail-closed arch assert

// -- game (clientGame, all static; Game.h) ----------------------------------
ENGINE_HOOKPOINT(game,          install)
ENGINE_HOOKPOINT(game,          quit)
ENGINE_HOOKPOINT(game,          mainLoop)
ENGINE_HOOKPOINT(game,          setupScene)
ENGINE_HOOKPOINT(game,          loadScene)
ENGINE_HOOKPOINT(game,          cleanupScene)
ENGINE_HOOKPOINT(game,          getPlayer)
ENGINE_HOOKPOINT(game,          getPlayerCreatureObject)
ENGINE_HOOKPOINT(game,          getCamera)
ENGINE_HOOKPOINT(game,          getConstCamera)
ENGINE_HOOKPOINT(game,          isViewFirstPerson)
ENGINE_HOOKPOINT(game,          isHudSceneTypeSpace)
ENGINE_HOOKPOINT(game,          g_runningFlags)
ENGINE_HOOKPOINT(game,          g_mainLoopCounter)
ENGINE_HOOKPOINT(game,          getShutdownPhase)
ENGINE_HOOKPOINT(game,          registerTickCallback)   // v35: fn fires at the TOP of runGameLoopOnce, game thread, outside any render chain -- the consumer deferred-command drain point (replaces their game::mainLoop detour)

// -- graphics (clientGraphics facade, all static; Graphics.h) ---------------
ENGINE_HOOKPOINT(graphics,      install)
ENGINE_HOOKPOINT(graphics,      update)
ENGINE_HOOKPOINT(graphics,      beginScene)
ENGINE_HOOKPOINT(graphics,      endScene)
ENGINE_HOOKPOINT(graphics,      present)
ENGINE_HOOKPOINT(graphics,      presentWindow)
ENGINE_HOOKPOINT(graphics,      resize)
ENGINE_HOOKPOINT(graphics,      flushResources)
ENGINE_HOOKPOINT(graphics,      screenshot)
ENGINE_HOOKPOINT(graphics,      useHardwareCursor)
ENGINE_HOOKPOINT(graphics,      showMouseCursor)
ENGINE_HOOKPOINT(graphics,      setSystemMouseCursorPosition)
ENGINE_HOOKPOINT(graphics,      setStaticShader)
ENGINE_HOOKPOINT(graphics,      g_renderTargetWidth)
ENGINE_HOOKPOINT(graphics,      g_renderTargetHeight)
ENGINE_HOOKPOINT(graphics,      registerFrameCallback)   // v35: fn fires render-thread in the D3D11 present path, post-BCG pre-Present -- the no-detour overlay draw point (replaces the consumer DXGI vtable patch)
ENGINE_HOOKPOINT(graphics,      registerResizeCallback)  // v35: fn(phase, w, h) -- phase 0 BEFORE ResizeBuffers (release back-buffer views), phase 1 after the new views exist (rebuild)

// -- scene::groundScene (clientGame; GroundScene.h) -- 38-01, EPA-05 ---------
// GroundScene is MULTIPLE-INHERITANCE (NetworkScene : public Scene, public
// MessageDispatch::Receiver) -> every PMF inflated, so each row is a __fastcall
// thunk (public methods + ctor in engine_advertise.cpp; PRIVATE init/update/
// handleInputMap* as in-TU GroundScene.cpp forwarders). draw is VIRTUAL (SKIP,
// vtable-resolved) and g_instance is inline-getter/private (OMIT) -> both kept
// OUT of this required set (see the .cpp SKIP/OMIT notes).
ENGINE_HOOKPOINT(groundScene,   ctor)
ENGINE_HOOKPOINT(groundScene,   init)
ENGINE_HOOKPOINT(groundScene,   reloadTerrain)
ENGINE_HOOKPOINT(groundScene,   changeCamera)
ENGINE_HOOKPOINT(groundScene,   getCurrentCamera)
ENGINE_HOOKPOINT(groundScene,   update)
ENGINE_HOOKPOINT(groundScene,   handleInputMapUpdate)
ENGINE_HOOKPOINT(groundScene,   handleInputMapEvent)

// -- cui::chatWindow (swgClientUserInterface; SwgCuiChatWindow.h) -- 38-03, EPA-05 --
// SwgCuiChatWindow is TRIPLE-INHERITANCE (SwgCuiLockableMediator, UINotification,
// MessageDispatch::Receiver [SwgCuiChatWindow.h:58-61]) -> every PMF inflated, so
// each row is a __fastcall thunk in engine_advertise.cpp (all 4 target PUBLIC
// non-virtual methods -- no friend decl needed). Contract names here are the SPEC
// names; the in-tree method NAME MISMATCH (acceptTextInput/appendToAllTabs/
// appendTextToCurrentTab/performEnterKey) is baked into the .cpp row comments.
// ctor is an MI ctor requiring a live UIPage& arg (37-03 already deferred) -> OMIT
// here (DEFER); chatEnterHandler advertises the CLEAN-entry performEnterKey only,
// the Issue #11 mid-function NOP is a separate SWGEmu-only joint decision (.cpp note).
ENGINE_HOOKPOINT(cuiChatWindow, enableTextInput)
ENGINE_HOOKPOINT(cuiChatWindow, writeToAllTabs)
ENGINE_HOOKPOINT(cuiChatWindow, writeToCurrentTab)
ENGINE_HOOKPOINT(cuiChatWindow, chatEnterHandler)
ENGINE_HOOKPOINT(cuiChatWindow, createNewWindow)

// -- cui::manager (clientUserInterface, all static; CuiManager.h) ------------
ENGINE_HOOKPOINT(cuiManager,    render)
ENGINE_HOOKPOINT(cuiManager,    setSize)
ENGINE_HOOKPOINT(cuiManager,    togglePointer)
ENGINE_HOOKPOINT(cuiManager,    restartMusic)
ENGINE_HOOKPOINT(cuiManager,    g_instance)

// -- cui::io (clientUserInterface; CuiIoWin.h) -------------------------------
ENGINE_HOOKPOINT(cuiIo,         setKeyboardInputActive)
ENGINE_HOOKPOINT(cuiIo,         requestKeyboard)
ENGINE_HOOKPOINT(cuiIo,         g_instance)

// -- consoleHelper (clientUserInterface; CuiConsoleHelper.h) -----------------
ENGINE_HOOKPOINT(consoleHelper, sendInput)

// -- commandParser (sharedCommandParser; CommandParser.h) --------------------
ENGINE_HOOKPOINT(commandParser, addSubCommand)

// ======================================================================
// 37-03 full-catalog required set. Extends the 37-02 MVP. Every name here
// has a source-confirmed, signature-checked row in engine_advertise.cpp.
// Symbols that the plan named but that do NOT exist in this tree (or are
// virtual / inline / multiple-inheritance-inflated) are OMITTED here and
// documented in the .cpp + 37-03-SUMMARY (graceful degradation, EPA-04:
// "a wrong & is worse than a missing row").
// ======================================================================

// -- extent (sharedCollision; BaseExtent.h) ---------------------------------
// §8 #2: the NON-virtual BaseExtent::intersect overload (collapses the
// UtinniCore retail/SWGEmu RVA split). Extent::intersect does NOT exist;
// Extent::realIntersect is protected-virtual (skipped).
ENGINE_HOOKPOINT(extent,        intersect)

// -- object (sharedObject; Object.h) -- NON-VIRTUAL getters/setters only -----
// (addToWorld/removeFromWorld/setParentCell are VIRTUAL -> skipped in .cpp;
//  move_o is INLINE Object.h:1216 -> OMITTED; no plain getType/move symbol.)
ENGINE_HOOKPOINT(object,        getObjectType)
ENGINE_HOOKPOINT(object,        getObjectTemplate)
ENGINE_HOOKPOINT(object,        getObjectTemplateName)
ENGINE_HOOKPOINT(object,        getNetworkId)
ENGINE_HOOKPOINT(object,        getParentCell)
ENGINE_HOOKPOINT(object,        getTransform_o2w)
ENGINE_HOOKPOINT(object,        setTransform_o2w)
ENGINE_HOOKPOINT(object,        getPosition_w)
ENGINE_HOOKPOINT(object,        setPosition_w)
ENGINE_HOOKPOINT(object,        getAppearance)
ENGINE_HOOKPOINT(object,        setAppearance)
ENGINE_HOOKPOINT(object,        setScale)             // v34: void (const Vector&) non-virtual [Object.h:228] -- closes the consumer's D-09 gap
ENGINE_HOOKPOINT(object,        move_p)

// -- objectTemplate (sharedObject/sharedGame) -------------------------------
// Base ObjectTemplate is virtual-heavy; the filename getters live on the
// NON-virtual SharedObjectTemplate (single-inheritance chain -> clean PMF).
ENGINE_HOOKPOINT(objectTemplate, createObject)
ENGINE_HOOKPOINT(objectTemplate, getAppearanceFilename)
ENGINE_HOOKPOINT(objectTemplate, getPortalLayoutFilename)
ENGINE_HOOKPOINT(objectTemplate, getClientDataFile)

// -- worldSnapshot (clientGame; WorldSnapshot.h) -- ALL STATIC in this tree --
// (the plan's instance methods openFile/saveFile/getNodeByIndex/... are the
//  consumer OFFLINE WorldSnapshot, not our runtime scene loader; OMITTED.)
ENGINE_HOOKPOINT(worldSnapshot, load)
ENGINE_HOOKPOINT(worldSnapshot, addObject)
ENGINE_HOOKPOINT(worldSnapshot, removeObject)
ENGINE_HOOKPOINT(worldSnapshot, moveObject)
ENGINE_HOOKPOINT(worldSnapshot, getLoadingPercent)
ENGINE_HOOKPOINT(worldSnapshot, detailLevelChanged)

// -- camera (clientGraphics; Camera.h) -- NON-VIRTUAL setters (non-inline) ---
ENGINE_HOOKPOINT(camera,        setViewport)
ENGINE_HOOKPOINT(camera,        setNearPlane)
ENGINE_HOOKPOINT(camera,        setFarPlane)
ENGINE_HOOKPOINT(camera,        setHorizontalFieldOfView)
ENGINE_HOOKPOINT(camera,        reverseProjectInViewportSpace)

// -- misc statics (memory/audio/file/report) --------------------------------
ENGINE_HOOKPOINT(memory,        allocate)
ENGINE_HOOKPOINT(memory,        free)
ENGINE_HOOKPOINT(audio,         setMasterVolume)
ENGINE_HOOKPOINT(audio,         getMasterVolume)
ENGINE_HOOKPOINT(treeFile,      open)
ENGINE_HOOKPOINT(treeFile,      searchTree)
ENGINE_HOOKPOINT(treeFile,      enumerateFiles)
ENGINE_HOOKPOINT(report,        print)

// -- commandParser ctors (free-function __thiscall thunks) ------------------
ENGINE_HOOKPOINT(commandParser, ctor1)
ENGINE_HOOKPOINT(commandParser, ctor2)

// -- globals (§8 #3) -- accessor-style; raw &g where genuinely accessible ----
ENGINE_HOOKPOINT(graphics,      g_frameNumber)

// ======================================================================
// 24-§2.B BUCKET B (Effects editor live preview) -- render/appearance group +
// particle retrigger. v6 -> v7 (NAME ADDs). Every name source-confirmed in THIS
// tree this wave; the requested-but-NOT-advertisable rows (virtual render methods,
// nonexistent methods, the un-addressable ctor, the wildcard sorter row, the raw
// render globals) are OMITTED/SKIPPED with reasons in engine_advertise.cpp + the
// HANDBACK -- a skipped virtual is consumer-vtable-resolved, NOT a missing required
// row (handoff §2.C).
// ======================================================================

// -- skeletalAppearance (clientSkeletalAnimation; SkeletalAppearance2.h) ------
// (render is VIRTUAL -> SKIP/vtable; addShaderPrimitives does NOT exist on the
//  class -> OMIT. Only the non-virtual LOD read is advertisable.)
ENGINE_HOOKPOINT(skeletalAppearance, getDisplayLodSkeleton)

// -- renderWorld (clientGraphics, all static; RenderWorld.h) ------------------
// (renderWorld::render does NOT exist -> OMIT; drawScene offered in the handback.)
ENGINE_HOOKPOINT(renderWorld,    addObjectNotifications)

// -- bloom (clientGame post-processing, all static; Bloom.h) ------------------
ENGINE_HOOKPOINT(bloom,          preSceneRender)
ENGINE_HOOKPOINT(bloom,          postSceneRender)

// -- particlePreview (consumer cooperative retrigger; ClientEffectManager) ------
// The Effects-editor live-preview hot-reload entry (handoff §2.B-ii). Friend free
// function engine_retriggerClientEffect over the private m_particleSystems.
ENGINE_HOOKPOINT(particlePreview, retrigger)
// Bucket B-2 (v7->v8): live .cef RE-PLAY. Free function engine_replayClientEffect
// re-plays a .cef FRESH on the local player via the public ClientEffectManager::
// playClientEffect -- the transient (muzzle/hit/explosion) case restart() above
// cannot cover. Public APIs only (not a friend); constant &fn (not a dyn[] row).
ENGINE_HOOKPOINT(particlePreview, replayClientEffect)

// -- Bucket A (v8->v9): per-editor real-entry detour rows (ledger §2.A) -------
// 6 of 14 requested are advertisable; the other 8 are OMIT (nonexistent / un-
// addressable ctor) or SKIP (virtual -> consumer vtable-resolves) and are
// therefore NOT in this required-name set -- they are documented in the Bucket A
// OMIT/SKIP ledger in engine_advertise.cpp + the handback. Several names below are
// MISMATCH-mapped to the real engine symbol (the contract name is the consumer's
// lookup key; the & is the real entry): setTarget->setLookAtTarget,
// receiveMessage->receiveSystemMessage, appendMessageData->appendMessage(data overload).
ENGINE_HOOKPOINT(cuiRadialMenuManager, update)              // static &fn (CuiRadialMenuManager::update)
ENGINE_HOOKPOINT(cuiMenu,              infoTypesFindDefaultCursor) // free fn Cui::MenuInfoTypes::findDefaultCursor
// systemMessageManager::receiveMessage REMOVED v10->v11 (A-2.1): advertised in Bucket A as the static
// receiveSystemMessage, it CRASHED world-load (c0000005) -- the consumer's hkReceiveMessage targets the
// MessageDispatch::Receiver::receiveMessage(emitter,message) byte-stream receiver (file-local Listener,
// un-advertisable), NOT the 1-arg static. A wrong & is worse than a missing row. See the OMIT note in
// engine_advertise.cpp + the handback.
ENGINE_HOOKPOINT(creatureObject,       setTarget)          // pmfRealEntry CreatureObject::setLookAtTarget (MI)
ENGINE_HOOKPOINT(messageQueue,         appendMessage)      // pmfToVoid MessageQueue::appendMessage(int,float,uint32)
ENGINE_HOOKPOINT(messageQueue,         appendMessageData)  // pmfToVoid MessageQueue::appendMessage(int,float,Data*,uint32)

// -- Bucket A-2 (v9->v10): world-pick / HUD-target (the §2.A getTarget gap) ----
// getTarget was OMIT in Bucket A (no SwgCuiHud::getTarget exists). Now advertised
// as the real entry of SwgCuiHud::getLastSelectedObject() const via a __fastcall MI
// thunk, plus the live-instance accessor (the hud the consumer calls getTarget on).
ENGINE_HOOKPOINT(cuiHud,              getTarget)            // __fastcall thunk -> SwgCuiHud::getLastSelectedObject (MI; CALLED)
ENGINE_HOOKPOINT(cuiHud,              g_instance)           // static accessor SwgCuiHudFactory::findMediatorForCurrentHud

// -- Bucket A-3 (v11->v12): network id->Object resolver (unblocks creatureObject::setTarget) ---
// The consumer's hkSetTarget resolves the new target id->Object* via Network::getObjectById; on the
// advertised client that hit a stale hardcoded RVA and crashed. Advertise the real static resolver.
ENGINE_HOOKPOINT(network,            getObjectById)        // static &fn NetworkIdManager::getObjectById(const NetworkId&)

// -- FREE-CAM editor unlock (v12->v13): accessor functions that encapsulate fragile NGE
// struct byte-offsets (the consumer will not hardcode offsets that drift vs our layout --
// same encapsulate-the-offset principle as particlePreview::retrigger + config::setModalChat).
// All 6 are CALLED accessors (the consumer INVOKES them; none detoured) -> plain &thunk/&fn or
// pmfToVoid. The consumer's "free camera" is OUR DebugPortalCamera: consumer cm_Free == 5 ==
// our GroundScene::CI_debugPortal (consumer cm_FreeChase == 2 == our CI_freeChase). See the
// row comments in engine_advertise.cpp for the exact real-entry mapping + each MISMATCH.
ENGINE_HOOKPOINT(groundScene,        isFreeCameraActive)               // __fastcall thunk: getCurrentView()==CI_debugPortal (==cm_Free)
ENGINE_HOOKPOINT(groundScene,        getDebugPortalCameraMessageQueue) // friend forwarder: m_debugPortalCameraInputMap->getMessageQueue()
ENGINE_HOOKPOINT(gameCamera,         getMessageQueue)                  // __fastcall thunk: getController()->getMessageQueue() (aliases the debugPortal MQ when free-cam active)
ENGINE_HOOKPOINT(messageQueue,       getCount)                         // MISMATCH: real entry MessageQueue::getNumberOfMessages (pmfToVoid)
ENGINE_HOOKPOINT(messageQueue,       getMessage)                       // 4-arg overload getMessage(int,int*,float*,uint32*) const (pmfToVoid, static_cast disambig)
ENGINE_HOOKPOINT(object,             isActive)                         // external-linkage shim (Object::isActive is NON-virtual but INLINE -> no PMF address)

// -- systemMessageManager SEND (v15): the INJECT half of the sysmsg pair (2026-07-02 request, rev-2 2026-07-03) --
// rev-2 NAME REPLACE (v14->v15): sendMessage -> sendMessageUtf8. The v14 direct-&fn row passed const
// Unicode::String& across the advertised boundary and CRASHED live smoke (consumer WString models the 2002
// SWGEmu 3-pointer string layout; ours is a v145 SSO basic_string -> garbage size read -> WRITE-AV). Now an
// extern "C" utf8 shim: void __cdecl engine_sendFakeSystemMessage(const char* utf8Msg, bool chatBoxOnly) widens
// on the PROVIDER side and calls sendFakeSystemMessage. ABI RULE (both repos): only primitives and pointers
// cross the boundary on CALLED endpoints; class-type params need an extern "C" shim (the
// engine_replayClientEffect precedent). Name-REPLACE (not re-point) so version-skewed pairings miss by name
// and degrade instead of mis-calling across a changed ABI. SEND half ONLY -- RECEIVE stays OMIT (the reverted
// Bucket A-2.1 receiveMessage; the real inbound receiver is a file-local Listener, un-advertisable).
ENGINE_HOOKPOINT(systemMessageManager, sendMessageUtf8)

// -- game lookAt-target READ (v15->v16, 2026-07-09 request): the read twin of
// creatureObject::setTarget (same m_lookAtTarget slot; NOT the NGE intended/combat
// target -- that would be a separate request, never a re-point of this row).
// extern "C" __int64 __cdecl shim (engine_getPlayerLookAtTargetId, defined in
// CreatureObject.cpp): returns the PLAYER's lookAt-target NetworkId VALUE (full
// 64 bits, cluster-id bits included); 0 = no player / no target. Primitive-only
// boundary (the sysmsg rev-2 ABI RULE) -- CreatureObject::getLookAtTarget() is
// INLINE and returns const CachedNetworkId& (embeds a Watcher<Object> the
// consumer does not model), so a member row is advertisable neither by address
// nor by ABI. Unblocks Game::getPlayerLookAtTargetObject() on the advertised
// client via the v12 network::getObjectById resolve (consumer §2.2 reroute).
ENGINE_HOOKPOINT(game, getPlayerLookAtTargetId)

// -- worldSnapshot editor READ wave (v16->v17, Goal B Wave 1; rev-3 freeze
// 2026-07-15): id-keyed read/browse of the CURRENT scene's live snapshot for
// the Utinni snapshot editor. All 7 are extern "C" __cdecl shims DEFINED IN
// clientGame WorldSnapshot.cpp -- the ms_reader file-local-singleton TU (the
// sysmsg/lookAtTarget shim pattern at TU scale); the engine_ws* export names
// are the rev-3 frozen identities. Shared contracts (pinned): enumeration is
// live, AUTHORED-ONLY -- tombstones (ANSWERS 5.1b) AND buildout-provenance
// rows (ANSWERS 5.1a, the retained ms_buildoutObjects filter) are never
// enumerated and answer miss on id-keyed reads; node reads force-finish the
// CONSULT-60 incremental parse (finishLoadNow discipline); indices are
// transient (consumer snapshots ids, never holds an index across a possible
// load/unload); wsGetNodeInfo fills the FROZEN 80-byte EngineWsNodeInfo
// (engine_hookpoints.h) under the size-first protocol; wsGetGeneration bumps
// on load/unload ONLY and is a PURE counter read (no parse force).
ENGINE_HOOKPOINT(worldSnapshot, wsForgetNode)           // int (int64 id) -- drop the node, KEEP the live Object (1/0). Persist no longer makes the placed object vanish
ENGINE_HOOKPOINT(clientInteriorLayoutManager, refreshInteriorLayout)  // int (int64 buildingId) -- re-apply a changed .ilf to ONE building, no scene reload (1/0/-1)
ENGINE_HOOKPOINT(cellProperty, getCellName)             // int (void* cellProperty, char* buf, int cap) -- copy-out; needed length INCL NUL. world cell returns "world"
ENGINE_HOOKPOINT(worldSnapshot, wsIsParsePending)       // int (void) -- 1 = parse in flight; PURE/non-forcing (no finishLoadNow prologue)
ENGINE_HOOKPOINT(worldSnapshot, wsGetNodeCount)        // int (void) -- top-level authored non-tombstone count; 0 = empty/no snapshot
ENGINE_HOOKPOINT(worldSnapshot, wsGetTopNodeIdAt)      // __int64 (int index) -- id of the index-th enumerable top-level node; 0 = out-of-range
ENGINE_HOOKPOINT(worldSnapshot, wsGetChildCount)       // int (__int64 id) -- enumerable direct-child count; 0 = miss/tombstone/leaf
ENGINE_HOOKPOINT(worldSnapshot, wsGetChildIdAt)        // __int64 (__int64 id, int index) -- id of the index-th enumerable child; 0 = miss/out-of-range
ENGINE_HOOKPOINT(worldSnapshot, wsGetNodeInfo)         // int (__int64 id, EngineWsNodeInfo* out) -- POD-out, size-first; 1 ok, 0 miss/tombstone
ENGINE_HOOKPOINT(worldSnapshot, wsGetNodeTemplateName) // int (__int64 id, char* buf, int cap) -- copy-out; returns needed length INCLUDING the NUL; 0 = miss
ENGINE_HOOKPOINT(worldSnapshot, wsGetGeneration)       // int (void) -- bumps on load/unload ONLY (consumer cache/undo invalidation)

// -- worldSnapshot editor MUTATION wave (v17->v18, Goal B Wave 2; frozen
// 2026-07-18): LIVE-ONLY mutation -- explicitly non-persistent (nothing
// touches disk; persistence is Wave 3). Same TU / discipline as Wave 1.
// Transform WRITE stays the advertised worldSnapshot::moveObject; live
// refresh stays detailLevelChanged. Semantics per the accepted ANSWERS
// 5.2/5.3/5.5 + Wave-2 deltas: adds route through the FULL streamed-create
// bookkeeping; wsAddObject pre-validates EVERYTHING before minting (a failed
// add never half-mutates); wsAddNodeAt = the undo-replay primitive (data
// re-add at the explicit id; top-level dirties the update-diff sentinels,
// child under a live parent spawns immediately); wsRemoveNode = the 7-step
// occupancy-guarded subtree teardown (1 ok / 0 miss / -1 occupied);
// allocator checks reader map + buildout set + NetworkIdManager inside the
// consumer-configurable [floor, ceiling) band.
ENGINE_HOOKPOINT(worldSnapshot, wsAddObject)           // __int64 (const char* sharedTemplateFilename, const float* transform12, __int64 containedById) -- mints id..id+cellCount, atomic POB cell expansion, spawns immediately; returns new top id, 0 = fail-closed nothing mutated
ENGINE_HOOKPOINT(worldSnapshot, wsAddNodeAt)           // int (__int64 explicitId, __int64 containedById, const char* templateFilename, int cellIndex, const float* transform12, float radius, unsigned int portalLayoutCrc) -- undo-replay re-add at the EXPLICIT id; 1 ok, 0 fail-closed
ENGINE_HOOKPOINT(worldSnapshot, wsRemoveNode)          // int (__int64 id) -- occupancy-guarded full subtree teardown + despawn; 1 removed / 0 miss / -1 occupied
ENGINE_HOOKPOINT(worldSnapshot, wsSetNodeRadius)       // int (__int64 id, float radius) -- 1 ok, 0 miss/tombstone; re-seats the sphere-tree extent
ENGINE_HOOKPOINT(worldSnapshot, wsConfigureIdAllocator) // int (__int64 floor, __int64 ceiling) -- one-time optional band config (0 = keep default per param); 1 accepted, 0 rejected (visible)

// -- worldSnapshot editor PERSISTENCE wave + riders (v18->v19, Goal B Wave 3;
// frozen 2026-07-18): the disk half + the 4B/4C rider rows. Save = authored-only
// (tombstone-skip recursive + the retained buildout-set filter), absolute
// destination in the winning loose SearchPath, searchPath negative-cache
// invalidation for the written name, post-write shadow verification via
// TreeFile::getPathName. Typed save-result enum (FROZEN, append-only): 0 ok /
// 1 no-snapshot / 2 no-loose-search-path / 3 destination-shadowed /
// 4 id-int32-overflow / 5 buildout-set-integrity / 6 write-failure.
ENGINE_HOOKPOINT(worldSnapshot, wsSaveSnapshot)         // int (void) -- save the CURRENT scene's authored .ws; typed result enum above
ENGINE_HOOKPOINT(worldSnapshot, wsGetSavePath)          // int (char* buf, int cap) -- save ROOT copy-out; needed length INCLUDING NUL; 0 = no loose SearchPath
ENGINE_HOOKPOINT(worldSnapshot, wsUnloadSnapshot)       // void (void) -- unload + sticky ms_sceneName reset (reload = this + advertised load); bumps generation
// 4B: the NGE world-pick/target gate (SwgCuiHud + CuiRadialMenuManager read it) --
// the engine's allowTargetAnything, NOT the consumer's SWGEmu patch working title.
ENGINE_HOOKPOINT(cuiPreferences, setAllowTargetAnything) // static void (bool) -- plain &fn [CuiPreferences.cpp:1051]
ENGINE_HOOKPOINT(cuiPreferences, getAllowTargetAnything) // static bool (void) -- plain &fn [CuiPreferences.cpp:1733]
// 4C: gizmo camera matrices -- copy-out shims (primitives-only ABI RULE; replaces
// the consumer's raw struct-offset camera reads that crash on the NGE layout).
ENGINE_HOOKPOINT(camera, getProjectionMatrix)           // int (float* out16) -- GlMatrix4x4 verbatim (row-major 4x4); 1 ok / 0 no camera
ENGINE_HOOKPOINT(camera, getTransformO2W)               // int (float* out12) -- row-major 3x4, position column 3 (the EngineWsNodeInfo convention); 1 ok / 0 no camera

// -- Live World Editor ray-pick + pre-approved radial clear (v19->v20,
// 2026-07-19 change request, SWG-Toolkit live-editor session): engine-side
// copy-out cursor ray-cast -- the consumer's collideCursorWithWorld done
// provider-side so CollisionInfo / ClientWorld::collide / camera viewport
// math never cross the boundary (ABI RULE; the rider-4C copy-out shape).
// Mirrors the engine's own hud pick (SwgCuiHud findObjectByPolygon /
// hitReticle flags + CollideParameters::cms_default + player excluded).
// Plus the cuiRadialMenuManager::clear rider pre-approved in the 2026-07-18
// positionchanged ANSWER.
ENGINE_HOOKPOINT(clientWorld, collideScreenRay)         // int (int screenX, int screenY, int objectsOnly, __int64* outHitObjectId, float* outPoint3) -- 1 hit / 0 miss; terrain hit -> id 0 + valid point; objectsOnly=1 drops terrain/terrainFlora/interiorGeometry
ENGINE_HOOKPOINT(cuiRadialMenuManager, clear)           // static void (void) -- plain &fn [CuiRadialMenuManager.h:47]

// -- current-scene-id copy-out (v20->v21, 2026-07-19 change request #2):
// one-click "Reload current scene" (unload + load(getSceneId())) + .ws
// auto-naming. Game::getSceneId() is inline + returns const std::string&
// -> copy-out shim mandatory (ABI RULE), wsGetSavePath convention.
ENGINE_HOOKPOINT(game, getSceneId)                      // int (char* buf, int cap) -- needed length INCLUDING NUL; 0 = no scene loaded

// -- borrowed-Object* pick (v21->v22, 2026-07-19 change request #3): the
// CONSULT-69 layer probe MEASURED that pure .ilf decorations never reach the
// hud pick (cuiHud::getTarget null for an id-less table the ray hits) --
// this row returns the RAW nearest-hit Object* from the collideScreenRay ray
// (no ancestor walk / no id resolution). BORROWED, game-thread-only; valid
// until the owning building leaves world; consumer clears on cell/zone
// change; pair with collideScreenRay + wsGetNodeInfo for layer triage.
ENGINE_HOOKPOINT(clientWorld, collideScreenRayObject)   // void* (int screenX, int screenY, int objectsOnly) -- raw hit Object*; null = miss/no camera

// -- in-place .ws node template re-point (v22->v23, 2026-07-19 change request
// #4): the CONSULT-70 lossless-rebind fix for per-instance interior editing
// (model D) -- remove+re-add would tombstone the building's WHOLE subtree
// (authored in-cell content lost) and churn the id; this swaps ONLY the
// node's template-name index (OTNL intern, append-only). Fail-closed:
// authored non-buildout nodes; template must resolve via TreeFile at call
// time (negative-cache invalidated first). Data-only -- live object
// untouched; reload spawns from the new template; spawn-time pob-crc check
// still guards a bad derived template LOUDLY.
ENGINE_HOOKPOINT(worldSnapshot, wsSetNodeTemplateName)  // int (__int64 id, const char* name) -- 1 ok / 0 miss / -1 refused

// -- object o2p copy-out (v23->v24, 2026-07-19 change request #5): the .ilf
// stores o2p (object-to-parent-cell); the gizmo drives o2w; this reads o2p
// directly for the model-D persist flow (pick-time original + persist-time
// moved). Object::getTransform_o2p() is inline + const Transform& -> shim
// mandatory (ABI RULE); layout = camera::getTransformO2W byte-for-byte.
ENGINE_HOOKPOINT(object, getTransformO2P)               // int (void* object, float* out12) -- row-major 3x4, pos col 3; 1 ok / 0 null

// -- containing-building id copy-out (v24->v25, 2026-07-30 change request
// #6): the model-D Arm step -- from any cell-contained object (an .ilf
// decoration pick, a wall-click CELL object, the player) to the POB
// building's NetworkId (== the .ws node id wsSetNodeTemplateName takes).
// Chain is inline/reference-returning -> shim mandatory (ABI RULE); a cell
// object's OWN CellProperty is used (getParentCell on a cell walks
// ancestors -> world cell -> wrong 0); world-cell objects report 0
// (no PortalProperty). Borrowed Object*, game-thread-only.
ENGINE_HOOKPOINT(object, getContainingBuildingId)       // __int64 (void* object) -- building NetworkId value; 0 = null / not inside a POB
ENGINE_HOOKPOINT(object, setParentCell)                 // int (void* object, void* cellProperty) -- cell reparent; 1 ok / 0 refused. VIRTUAL -> shim. Null cell FATALs engine-side: use cellProperty::getWorldCellProperty for "outside"
ENGINE_HOOKPOINT(cellProperty, getWorldCellProperty)    // CellProperty* (void) -- world-cell sentinel; plain &fn (out-of-line static), no shim
ENGINE_HOOKPOINT(cellProperty, setPortalTransitionsEnabled) // void (bool) -- suppress the portal sweep across a teleport write; plain &fn. ALWAYS re-enable (global state)
ENGINE_HOOKPOINT(collisionWorld, objectWarped)          // void (Object*) -- reconcile collision after a discontinuous move; plain &fn
ENGINE_HOOKPOINT(clientWorld, findCellAtWorldPosition)  // void* (float x,y,z) -- CellProperty* containing world point P; never null (world-cell fallback). Placement routing
ENGINE_HOOKPOINT(object, getAttachedTo)                 // void* (void* object) -- parent object or 0. NOT a mount guard: cell parentage shares this field
ENGINE_HOOKPOINT(playerCreatureController, warpClient)  // int (float x,y,z) world -- client-initiated SEQUENCED teleport; a raw setTransform_o2w is overwritten by the server
ENGINE_HOOKPOINT(object, isChildObject)                 // int (void* object) -- 1 = genuine child/mount attachment. THE mount discriminator (v28's getAttachedTo guidance was falsified live)
