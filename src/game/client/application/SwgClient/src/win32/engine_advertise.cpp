// ======================================================================
//
// engine_advertise.cpp -- Utinni engine entry-point advertisement provider
// (handoff 2026-06-20). The exe-side game-logic twin of the shipped graphics
// gl11_r.dll!GetHookPoints (Direct3d11.cpp:856-888).
//
// Exports one undecorated extern "C" __cdecl GetEngineHookPoints() that hands
// an injected modding overlay (Utinni) a name->pointer table of engine
// functions/globals, each address taken at compile time by &EngineSymbol so
// it is correct by construction and survives every rebuild. The addresses are
// BORROWED (process-lifetime statics); Utinni only reads the table.
//
// Pure read-only getter -- no behavioral change, no Utinni dependency. If
// Utinni is not injected nothing calls GetEngineHookPoints(); the export is
// inert. Each row carries NO calling convention -- Utinni's typedef supplies
// it and must match MSVC's emitted convention.
//
// Both platforms (x64 port 2026-08-15; was 32-bit only). x64 has ONE calling
// convention, so every __cdecl/__stdcall/__fastcall annotation below is inert
// there and a free function f(C* pThis, args...) matches a member call
// natively -- the Win32 __fastcall dummy-EDX __thiscall emulation must NOT
// carry a dummy on x64 (it would shift every real arg one register right).
// The ENGINE_THIS macro below owns that difference; everything else in the
// surface is convention-agnostic by construction (primitives/pointers-only
// boundary, name-keyed GetProcAddress binding, fixed-width contract structs).
//
// See .planning/handoff/2026-06-20-utinni-engine-entrypoint-advertisement-spec.md.
// ======================================================================

#include "FirstSwgClient.h"

#include <bit>      // std::bit_cast (C++20, stdcpp20 enabled)
#include <cstring>  // std::memcpy (pmfRealEntry MI-PMF code-component extraction, 38-05)

#include "engine_hookpoints.h"
#include "ClientMain.h"                            // engine_installConfigFileOverride() + ClientMain()
#include "sharedFoundation/ConfigFile.h"           // ConfigFile::loadFile / loadFromBuffer (static)
#include "clientGame/Game.h"                       // Game::* (static) + isOver accessor
#include "clientGraphics/Graphics.h"               // Graphics::* (static)
#include "clientUserInterface/CuiManager.h"        // CuiManager::* (static) + getIoWin accessor
#include "clientUserInterface/CuiIoWin.h"          // CuiIoWin::* (member PMFs)
#include "clientUserInterface/CuiConsoleHelper.h"  // CuiConsoleHelper::processInput (member PMF)
#include "sharedCommandParser/CommandParser.h"     // CommandParser::addSubCommand (member PMF)
#include "clientGame/GroundScene.h"                 // GroundScene MI thunks + ctor (38-01)
#include "clientGame/Scene.h"                        // Scene complete type for the game::setupScene MI upcast (2026-06-25 re-map)
#include "engine_groundScene_forward.h"             // engine_groundScene* private-method forwarders (38-01; exe-local)
#include "engine_clientShims_forward.h"             // engine_osWindowProc / engine_writeMiniDump shims (38-02; exe-local)
#include "engine_chatWindow_forward.h"              // engine_chatWindowCreateNewWindowEntry() PRIVATE-funnel real-entry (24-4d; exe-local)
#include "clientUserInterface/CuiPreferences.h"     // CuiPreferences::setModalChat/getModalChat (38-02; 37-02 CORRECTION -- NOT ConfigFile)
#include "swgClientUserInterface/SwgCuiChatWindow.h" // SwgCuiChatWindow MI thunks (38-03; TRIPLE-MI -> __fastcall thunks, never pmfToVoid)

// -- 37-03 full-catalog includes --------------------------------------------
#include "sharedCollision/BaseExtent.h"            // BaseExtent::intersect (non-virtual overload, §8 #2)
#include "sharedObject/Object.h"                   // Object non-virtual getters/setters (PMF)
#include "sharedGame/SharedObjectTemplate.h"       // SharedObjectTemplate filename getters (PMF) + ObjectTemplate::createObject
#include "clientGame/WorldSnapshot.h"              // WorldSnapshot::* (all static in this tree)
#include "clientGraphics/Camera.h"                 // Camera non-virtual setters (PMF)
#include "sharedMemoryManager/MemoryManager.h"     // MemoryManager::allocate/free (static)
#include "clientAudio/Audio.h"                     // Audio::set/getMasterVolume (static)
#include "sharedFile/TreeFile.h"                   // TreeFile::open (static)
#include "sharedDebug/Report.h"                    // Report::puts (static)

// -- 24-§2.B Bucket B includes (Effects editor live preview) ----------------
#include "clientSkeletalAnimation/SkeletalAppearance2.h" // SkeletalAppearance2::getDisplayLodSkeleton (non-virtual LOD read, bit_cast PMF)
#include "clientGraphics/RenderWorld.h"            // RenderWorld::addObjectNotifications (static)
#include "clientGame/Bloom.h"                       // Bloom::preSceneRender/postSceneRender (static)
#include "engine_clientEffect_forward.h"            // engine_retriggerClientEffect (friend free fn over m_particleSystems; exe-local)

// -- Bucket A includes (per-editor real-entry rows; v8 -> v9) ----------------
#include "clientUserInterface/CuiRadialMenuManager.h"   // CuiRadialMenuManager::update (static &fn)
#include "clientUserInterface/CuiMenuInfoTypes.h"        // Cui::MenuInfoTypes::findDefaultCursor (namespace free fn)
#include "clientUserInterface/CuiSystemMessageManager.h" // sysmsg SEND: CuiSystemMessageManager::sendFakeSystemMessage, reached via the engine_sendFakeSystemMessage utf8 shim (v15). RE-ADDED post the v10->v11 receiveMessage revert -- this is the SEND half, NOT the reverted receive half (see the A-2.1 OMIT note).
#include "UnicodeUtils.h"                                // sysmsg SEND rev-2 (v14->v15): Unicode::narrowToWide for the utf8->Unicode::String widen inside the shim (widen on OUR side -- our CRT, our string layout)
#include "engine_creatureObject_forward.h"               // engine_creatureSetTargetRealEntry() -- CreatureObject.h too heavy for the exe TU (sharedSkillSystem); accessor lives in CreatureObject.cpp
#include "engine_worldSnapshot_forward.h"                // Goal B Wave 1 (v17): engine_ws* snapshot-editor READ shims -- DEFINED in clientGame WorldSnapshot.cpp (the ms_reader file-local-singleton TU)
#include "sharedFoundation/MessageQueue.h"               // MessageQueue::appendMessage overloads (flat class -> pmfToVoid)
#include "sharedObject/NetworkIdManager.h"               // Bucket A-3: NetworkIdManager::getObjectById (static NetworkId->Object* resolver)
#include "swgClientUserInterface/SwgCuiHud.h"            // Bucket A-2: SwgCuiHud::getLastSelectedObject (MI -> __fastcall thunk)
#include "swgClientUserInterface/SwgCuiHudFactory.h"     // Bucket A-2: SwgCuiHudFactory::findMediatorForCurrentHud (static active-hud accessor)

#include "clientObject/GameCamera.h"                     // FREE-CAM (v13): gameCamera::getMessageQueue thunk (via Object::getController)
#include "sharedObject/Controller.h"                     // FREE-CAM (v13): Controller::getMessageQueue (the camera's movement MQ)

// -- Live World Editor ray-pick (v19 -> v20, 2026-07-19 change request) -------
#include "clientGame/ClientWorld.h"                      // ClientWorld::collide (static) + the CF_* pick flags
#include "sharedCollision/CollisionWorld.h"              // v28 collisionWorld::objectWarped -- reconcile collision after a discontinuous move
#include "clientGame/ConfigClientGame.h"                 // getTargetingRange (the engine cursor-ray viewDistance, SwgCuiHud.cpp:1163)
#include "sharedCollision/CollideParameters.h"           // CollideParameters::cms_default (the hud pick's parameter set)
#include "sharedCollision/CollisionInfo.h"               // stack copy-out source (never crosses the boundary)
#include "sharedTerrain/TerrainObject.h"                 // terrain-instance check (terrain hit -> id 0, point valid)

#include "sharedObject/CellProperty.h"                   // v25 getContainingBuildingId: cell -> getPortalProperty() (inline, CellProperty.h:270)
#include "sharedObject/PortalProperty.h"                 // v25 getContainingBuildingId: portal -> getOwner() = the building (Property.h:57 via Container)
class Skeleton;                                     // for the getDisplayLodSkeleton PMF return type (incomplete is fine)

// ----------------------------------------------------------------------
// __thiscall-emulation ABI seam (x64 port 2026-08-15). Win32: MSVC v145
// forbids __thiscall on a free function (C3865), so a thunk emulates it as
// __fastcall(pThis /*ECX*/, dummy /*EDX*/, args...) -- byte-identical ABIs.
// x64: there is ONE convention (this/arg1 in RCX, then RDX/R8/R9); __fastcall
// is accepted-and-ignored, a free function f(C* pThis, args...) already
// matches a member call exactly, and the Win32 dummy would shift every real
// argument one register right. Consumer typedefs written as __thiscall work
// unchanged on both (the keyword is likewise ignored on x64).
// Mirrored (guarded #ifndef) in the clientGame TUs that define forwarders
// (GroundScene.cpp) -- exe-local headers are not on clientGame's include path.
// ----------------------------------------------------------------------
#ifndef ENGINE_THIS
#if defined(_WIN64)
	#define ENGINE_THIS(ClassPtrType) ClassPtrType pThis
#else
	#define ENGINE_THIS(ClassPtrType) ClassPtrType pThis, int /*edx*/
#endif
#endif

// ----------------------------------------------------------------------
// PMF -> void* helper. Non-static, non-virtual member function pointers are
// not void*-convertible by a plain cast; std::bit_cast is the standard,
// well-defined C++20 idiom (a union type-pun would be ill-formed). The size
// guard catches MULTIPLE/VIRTUAL-INHERITANCE PMF inflation (>4 bytes) ONLY --
// it does NOT catch a &Class::virtualMethod (still pointer-sized, but yields a
// vtable-dispatch stub, NOT the impl). Virtual rows must be SKIPPED, not
// bit_cast (per the landmine rules in 37-RESEARCH / 37-PATTERNS).
// ----------------------------------------------------------------------
template <class PMF>
inline void * pmfToVoid(PMF pmf)
{
	static_assert(sizeof(PMF) == sizeof(void *), "inflated PMF (multiple/virtual inheritance) -- needs a thunk");
	return std::bit_cast<void *>(pmf);
}

// ----------------------------------------------------------------------
// pmfRealEntry (38-05, address-correctness fix). The REAL engine code entry of
// a non-virtual member of an MI / inflated-PMF class -- for DETOURED endpoints.
//
// Why this exists: a DETOURED endpoint (Utinni patches the prologue so the
// engine's OWN call into the method is intercepted) MUST advertise the actual
// compiled method body the engine reaches, NOT a call-through forwarder thunk.
// A DetourXS patch on a forwarder fires only when someone calls the FORWARDER;
// the engine calls the real method directly, so a forwarder-advertised detour is
// silently dead (links, exports, boots, hook never fires). See the Utinni review
// finding .planning/handoff/2026-06-22-utinni-detour-vs-call-followup.md.
//
// MSVC 32-bit MI / virtual-inheritance NON-VIRTUAL PMF layout is
//   { void * pfn; int delta; [int vtordisp; int vindex] }
// pfn (offset 0) is the real code entry; delta (offset 4) is the this-adjustment
// the caller would apply to reach the correct base subobject before the call.
//
// SAFETY GATE (the whole point): for an OWN non-virtual method of the MOST-DERIVED
// class -- which all four 38-05 detour targets are (GroundScene::{update,
// handleInputMapEvent}, SwgCuiChatWindow::{acceptTextInput,performEnterKey}) --
// the primary base is at offset 0, so delta MUST be 0 and pfn IS the entry
// Utinni's __thiscall trampoline reaches with `this` in ECX. If delta != 0 the
// entry is a secondary-base method whose `this` needs adjustment and is NOT
// directly detour-able with the raw `this`; we MUST NOT advertise it. We return
// nullptr (after a DEBUG_FATAL) so engine_verifyNoNullNoDup() catches it as a
// null row and FAILS loudly -- never advertise a wrong / silent-dead entry.
//
// NOTE: this is for DETOURED rows only. CALLED rows keep their call-through
// __fastcall forwarders (correct there -- Utinni invokes the forwarder, it
// forwards). Do NOT use pmfRealEntry for a called endpoint.
// ----------------------------------------------------------------------
template <class PMF>
inline void * pmfRealEntry(PMF pmf)
{
	static_assert(sizeof(PMF) >= sizeof(void *) + sizeof(int), "PMF smaller than expected MI layout");
	struct MiPmf { void * pfn; int delta; };
	MiPmf m{};
	std::memcpy(&m, &pmf, sizeof(MiPmf));
	if (m.delta != 0)
	{
		DEBUG_FATAL(true, ("engine: non-zero PMF delta for a real-entry (detoured) row -- secondary-base method is NOT directly detour-able"));
		return 0;
	}
	return m.pfn;
}

// ----------------------------------------------------------------------
// Crash-fixer thunk (EPA-02 -- the single most important row). Utinni's
// config::loadOverrideConfig typedef is int(__cdecl*)() (zero-arg orchestrator;
// D:/Code/Utinni/UtinniCore/swg/misc/config.cpp). The spec's best-guess
// buffer-loader is the INNER call, NOT the hooked function. This thunk wraps
// the distinctly-named forwarding shim engine_installConfigFileOverride()
// (ClientMain.h), which forwards to ClientMainNamespace::installConfigFileOverride().
// Do NOT advertise the buffer-loader symbol here (EPA-02 correction).
// ----------------------------------------------------------------------
static int __cdecl engine_loadOverrideConfig()
{
	engine_installConfigFileOverride();
	return 0;
}

// ----------------------------------------------------------------------
// Constructor thunks (37-03). You cannot take &Class::Class in C++. Each
// ctor row is a free-function thunk that placement-constructs on the
// caller-supplied `this` and returns it -- matching UtinniCore's ctor
// typedef EXACTLY (D:/Code/Utinni/UtinniCore/swg/ui/command_parser.cpp:29-30:
// `CommandParser*(__thiscall*)(pThis, args...)`).
//
// CALLING CONVENTION (the landmine): MSVC v145 forbids __thiscall on a free
// function (C3865). The ABI-correct emulation of __thiscall(pThis, a, b, ...)
// is __fastcall(pThis /*ECX*/, dummy /*EDX*/, a, b, ...): __thiscall passes
// `this` in ECX and pushes the rest; __fastcall passes arg1 in ECX, arg2 in
// EDX, and the rest on the stack -- so a dummy EDX param makes the two ABIs
// byte-identical. Utinni's existing __thiscall typedef therefore calls this
// thunk correctly with NO Utinni-side change. (A plain __cdecl/__stdcall thunk
// would mismatch -- this/ECX vs stack -- and crash at the first detour.)
//
// CommandParser is a standalone single-inheritance class (no bases) so the
// `this` pointer needs no most-derived adjustment.
//
// MI-class UI ctors (chatWindow/loginScreen/gameMenu) are DEFERRED -- see the
// OMIT/DEFER block below and 37-03-SUMMARY (same MI-inflation rationale that
// deferred them in 37-02; they also require a live UIPage& arg).
// ----------------------------------------------------------------------
static CommandParser * __fastcall engine_commandParserCtor1(ENGINE_THIS(CommandParser *),
	const char * command, size_t argCount, const char * args, const char * helpInfo, CommandParser * delegate)
{
	return ::new (static_cast<void *>(pThis)) CommandParser(command, argCount, args, helpInfo, delegate);
}

static CommandParser * __fastcall engine_commandParserCtor2(ENGINE_THIS(CommandParser *),
	const CommandParser::CmdInfo & commandData, CommandParser * delegate)
{
	return ::new (static_cast<void *>(pThis)) CommandParser(commandData, delegate);
}

// ----------------------------------------------------------------------
// consoleHelper::sendInput thunk (WR-05 fix; the cross-repo follow-up for the
// Utinni Phase-24 D-02 carve-out -- the ONE .inc name they left unbound, on RVA,
// pending this provider). The retail target SwgCuiConsoleHelper::sendInput (spec
// line 283; SWGEmu 0x009141D0) has NO from-source twin in this client; the
// engine-layer equivalent is CuiConsoleHelper::processInput. But processInput
// takes a REQUIRED 2nd arg -- a stdset<Unicode::String>& recursion-guard stack
// [CuiConsoleHelper.h:76] -- that an injector calling a one-string
// "sendInput(text)" cannot know to supply, so a raw &CuiConsoleHelper::processInput
// is detoured with garbage/absent arg2 and faults at the first call (WR-05). This
// thunk presents the one-string __thiscall ABI and supplies the recursion stack
// itself from the public static getRecurseStackForCommandBeingParsed()
// [CuiConsoleHelper.h:123 / CuiConsoleHelper.cpp:1136] -- the SAME canonical call
// form the engine uses internally (CuiConsoleHelper.cpp:99). (processInputLine,
// the no-stack sibling, is PRIVATE [:132] so its address cannot be taken here.)
//
// CALLING CONVENTION: same __fastcall(pThis /*ECX*/, dummy /*EDX*/, args) ==
// __thiscall emulation as the ctor thunks above (MSVC v145 forbids __thiscall on
// a free function, C3865). CuiConsoleHelper is single-inheritance (UIEventCallback
// only) so `this` needs no most-derived adjustment.
// UTINNI-SIDE TYPEDEF TO MATCH: bool(__thiscall*)(CuiConsoleHelper*, const Unicode::String&).
// ----------------------------------------------------------------------
static bool __fastcall engine_consoleHelperSendInput(ENGINE_THIS(CuiConsoleHelper *),
	const Unicode::String & istr)
{
	return pThis->processInput(istr, CuiConsoleHelper::getRecurseStackForCommandBeingParsed());
}

// ----------------------------------------------------------------------
// groundScene::* MI thunks (38-01, EPA-05). GroundScene is MULTIPLE-INHERITANCE
// (NetworkScene : public Scene, public MessageDispatch::Receiver) so every PMF
// is inflated and trips the pmfToVoid sizeof guard (C2338) -- a __fastcall thunk
// is mandatory; pmfToVoid(&GroundScene::member) must NEVER be used here.
//
// CALLING CONVENTION: same __fastcall(pThis /*ECX*/, int /*EDX*/, args) ==
// __thiscall emulation as the ctor thunks above (MSVC v145 forbids __thiscall on
// a free function, C3865; the dummy EDX makes the two ABIs byte-identical).
//
// These three target PUBLIC GroundScene methods, so they can be named directly
// here (the four PRIVATE methods are reached via the GroundScene.cpp forwarders
// declared in engine_groundScene_forward.h -- a free thunk in this TU cannot
// name a private member, C2248).
// ----------------------------------------------------------------------
static void __fastcall engine_groundSceneReloadTerrain(ENGINE_THIS(GroundScene *))
{
	pThis->reloadTerrain();                                          // public [GroundScene.h:215]
}

static void __fastcall engine_groundSceneChangeCamera(ENGINE_THIS(GroundScene *),
	int newView, float value)
{
	pThis->setView(newView, value);                                 // public [GroundScene.h:207] (contract name changeCamera -> ours setView)
}

static GameCamera * __fastcall engine_groundSceneGetCurrentCamera(ENGINE_THIS(GroundScene *))
{
	return pThis->getCurrentCamera();                               // public, non-const this -> non-const overload, no cast [GroundScene.h:212]
}

// groundScene::ctor -- placement-new MI ctor thunk (you cannot take &Class::Class;
// 37-03 ctor-thunk pattern). MI class -> the caller-supplied `this` is the
// most-derived pointer; ::new(pThis) constructs the full object via the
// (const char*, const char*, CreatureObject*) overload [GroundScene.h:199] -- NOT
// the (const char*, const NetworkId&, ...) overload at :200.
static GroundScene * __fastcall engine_groundSceneCtor(ENGINE_THIS(GroundScene *),
	const char * terrainFilename, const char * playerFilename, CreatureObject * customizedPlayer)
{
	return ::new (static_cast<void *>(pThis)) GroundScene(terrainFilename, playerFilename, customizedPlayer);
}

// ----------------------------------------------------------------------
// game::setupScene -- pre-built-scene setter thunk (2026-06-25 re-map). The editor's
// "Load scene" uses the build-then-set pattern: Utinni constructs the scene itself via
// the advertised groundScene::ctor and hands the GroundScene* here (the SWGEmu
// setupScene(GroundScene*) shape). The row PREVIOUSLY advertised the strings-builder
// overload Game::setScene(bool,terrain,player,customized) [Game.h:108], which reads the
// pointer as its bool `immediately` + garbage filename args -> builds a scene with a
// garbage player -> 0xC0000005 WRITE in GroundScene::GroundScene (player->setPosition_p,
// GroundScene.cpp:948). The correct "set this pre-built scene active" entry is
// Game::_setScene(Scene*) [Game.h:126 / Game.cpp:1386, ms_scene = newScene]. (NOTE: the
// request assumed _setScene is PRIVATE; in THIS tree it is PUBLIC -> direct call, no
// friend/forwarder needed -- correction vs the SWGEmu layout.)
//
// CAST: Utinni passes the most-derived GroundScene* (from groundScene::ctor) as a void*.
// GroundScene is MULTIPLE-INHERITANCE (NetworkScene : public Scene, public
// MessageDispatch::Receiver) so the Scene base is at a NON-ZERO offset -- static_cast<Scene*>
// applies the correct this-adjustment; a raw reinterpret to Scene* would pass the wrong
// subobject. __cdecl(void*) matches Utinni's swg::game::setupScene typedef void(__cdecl*)(GroundScene*).
// ----------------------------------------------------------------------
static void __cdecl engine_gameSetupScene(void * groundScene)
{
	Game::_setScene(static_cast<Scene *>(reinterpret_cast<GroundScene *>(groundScene)));
}

// ----------------------------------------------------------------------
// game::loadScene -- FULL editor scene-load via the SceneCreator lifecycle (2026-06-25
// consult). game::setupScene (-> _setScene(Scene*)) only does `ms_scene = it`; it SKIPS
// the SceneCreator/_startScene integration (loading-manager handshake, endDeferredCreation,
// ms_nextScene management), so a pre-built GroundScene set that way is half-integrated ->
// next-frame C++ throw (Fatal). The correct entry is the string builder:
//   Game::setScene(immediately, terrain, player, customized) [Game.h:108]
//     -> SinglePlayerSceneCreator -> _setScene(SceneCreator&, immediately)
//     -> _startScene -> SinglePlayerSceneCreator::create() [new GroundScene] -> _setScene(Scene*)
// So the editor must pass terrain+avatar FILENAMES and let the engine build+integrate, NOT
// hand over a pre-built GroundScene.
//
// immediately = TRUE: load this scene NOW (no planet intro cinematic / cutscene defer). This
// is exactly what the engine's OWN by-name loaders use -- SwgCuiSceneSelection.cpp:264,
// SwgCuiLocations.cpp:179, SwgCuiCommandParserScene.cpp:288 all call setScene(true, ...).
// customizedPlayer = nullptr: the editor has no customization-screen object, so the GroundScene
// ctor loads the avatar synchronously from playerFilename (GroundScene.cpp:930-945 -- the normal
// single-player path; matches SwgCuiLocations / SwgCuiCommandParserScene which pass 0).
// REQUIREMENT: playerFilename must be a loadable object template or the ctor FATALs (GroundScene.cpp:942).
// ----------------------------------------------------------------------
static void __cdecl engine_gameLoadScene(const char * terrainFilename, const char * playerFilename)
{
	// SINGLE-PLAYER (2026-06-25 loading-screen-stuck consult, Option A): the advertised editor client
	// has NO connection, so the server-driven PlayerObject (ghost) never arrives. GroundScene's
	// loading-screen teardown is gated on isFinishedLoading(), which requires getPlayerObject() != NULL
	// (GroundScene.cpp:1837) -> offline that is permanently false -> the fullscreen load/cut screen never
	// dismisses. Mark single-player BEFORE the load: the engine's loading path honors getSinglePlayer()
	// (skips the network clientReady send, GroundScene.cpp:2103) and isFinishedLoading() is relaxed to
	// accept single-player in lieu of a ghost. The flag is otherwise false for SWGEmu connected play.
	Game::setSinglePlayer(true);

	// DESTROY THE OUTGOING SCENE FIRST (2026-08-04). Game::_setScene(Scene*) only
	// reassigns ms_scene -- it never closes or deletes the scene it replaces, so the
	// teardown is the CALLER's job. All THREE by-name scene installers in this tree do
	// it, and this shim was the only one that did not: GameNetwork::startScene
	// (GameNetwork.cpp:483-492, commented "First destroy the old scene if need be"),
	// SwgCuiCommandParserScene::performSceneChange (:271-283) and SwgCuiLocations
	// (:171-177). Omitting it is not merely a leak: ~GroundScene is what calls
	// ClientWorld::remove() (GroundScene.cpp:1132), so without it the incoming
	// GroundScene ctor's ClientWorld::install() (:691) runs a SECOND time over a live
	// world. Both install guards are DEBUG_FATAL (ClientWorld.cpp:854, World.cpp:162)
	// and so compile out in Release, where World::install() then replaces every WOL_*
	// object list with a fresh empty one (World.cpp:165-168) -- leaking the old lists
	// and orphaning every object still in them.
	//
	// NOT copied from performSceneChange: its `getAttachedTo() != 0` refusal. v29
	// established that predicate is ALSO true for any player merely standing in a cell
	// (cell parentage and mount attachment share m_attachedToObject), so mirroring it
	// would turn loadScene into a silent no-op exactly where the editor is used --
	// indoors. GameNetwork::startScene has no such guard either. A consumer that wants
	// to refuse while mounted should gate its own call site on object::isChildObject
	// (v29), which is the real discriminator.
	{
		GroundScene * const outgoing = dynamic_cast<GroundScene *>(Game::getScene());
		if (outgoing)
		{
			outgoing->close();
			delete outgoing;                                            // ~GroundScene self-clears ms_scene (GroundScene.cpp:1081-1084) and calls ClientWorld::remove()
		}
	}

	Game::setScene(true, terrainFilename, playerFilename, nullptr);   // immediately=true (load now), customizedPlayer=nullptr (engine loads avatar from playerFilename)
}

// ----------------------------------------------------------------------
// cui::chatWindow::* (38-03 EPA-05 remainder; 38-05 detour-correctness). SwgCuiChatWindow
// is TRIPLE-INHERITANCE (public SwgCuiLockableMediator, public UINotification, public
// MessageDispatch::Receiver [SwgCuiChatWindow.h:58-61]) so every PMF is inflated and
// trips the pmfToVoid sizeof guard (C2338).
//
// Two MECHANISMS now (the 38-05 split):
//   * CALLED rows (writeToAllTabs / writeToCurrentTab) keep a call-through __fastcall
//     thunk -- correct because Utinni INVOKES the thunk and it forwards. CALLING
//     CONVENTION: __fastcall(pThis /*ECX*/, int /*EDX*/, args) == __thiscall (MSVC v145
//     forbids __thiscall on a free function, C3865; dummy EDX makes the two ABIs byte-
//     identical). The two targets are PUBLIC, named directly here (no friend decl).
//   * DETOURED rows (enableTextInput->acceptTextInput, chatEnterHandler->performEnterKey)
//     are advertised by their REAL engine code entry via pmfRealEntry(&SwgCuiChatWindow::m)
//     INLINE in the table below (both are OWN non-virtual most-derived methods -> delta==0,
//     so pmfRealEntry returns the real body the engine detours). A call-through thunk would
//     be SILENTLY DEAD for a detour (the engine calls the real method, not the thunk) --
//     the Utinni review finding (2026-06-22-utinni-detour-vs-call-followup.md). The two
//     38-03 call-through chat thunks (engine_chatWindowAcceptTextInput / ...PerformEnterKey)
//     are therefore REMOVED here (38-05) -- they had no other referencer.
//
// The contract names are the SPEC names (enableTextInput/writeToAllTabs/writeToCurrentTab/
// chatEnterHandler); the in-tree methods have DIFFERENT names (acceptTextInput/
// appendToAllTabs/appendTextToCurrentTab/performEnterKey) -- the NAME MISMATCH is baked
// into each row comment, the contract names stay.
// ----------------------------------------------------------------------
static void __fastcall engine_chatWindowAppendToAllTabs(ENGINE_THIS(SwgCuiChatWindow *),
	const Unicode::String & str)
{
	pThis->appendToAllTabs(str);                                    // public [SwgCuiChatWindow.h:172] (contract writeToAllTabs -> ours appendToAllTabs)
}

static void __fastcall engine_chatWindowAppendTextToCurrentTab(ENGINE_THIS(SwgCuiChatWindow *),
	const Unicode::String & str)
{
	pThis->appendTextToCurrentTab(str);                             // public [SwgCuiChatWindow.h:174] (contract writeToCurrentTab -> ours appendTextToCurrentTab)
}

// ----------------------------------------------------------------------
// Bucket A-2 (2026-06-28) -- world-pick / HUD-target real-entry thunk.
// cuiHud::getTarget -> SwgCuiHud::getLastSelectedObject() const [SwgCuiHud.h:95]
// (m_lastSelectedObject = the world-picked object). SwgCuiHud is MULTIPLE-INHERITANCE
// (CuiMediator + UIEventCallback [SwgCuiHud.h:41-43]) -> a raw PMF is inflated and trips
// the pmfToVoid sizeof guard (C2338). This is a CALLED row (the consumer invokes it on the
// live hud to READ the pick), so a __fastcall(pThis /*ECX*/, int /*EDX*/) == __thiscall
// call-through thunk is correct (NOT pmfRealEntry -- that is for DETOURED rows). The consumer
// passes the live SwgCuiHud* from cuiHud::g_instance (SwgCuiHudFactory::findMediatorForCurrentHud)
// in ECX; getLastSelectedObject is SwgCuiHud's OWN method so `this` is the SwgCuiHud subobject
// (no adjustment). PUBLIC method -> named directly (no friend). Consumer typedef:
//   Object*(__thiscall*)(SwgCuiHud*)
// ----------------------------------------------------------------------
static Object * __fastcall engine_hudGetLastSelectedObject(ENGINE_THIS(SwgCuiHud *))
{
	return pThis->getLastSelectedObject();                          // public const [SwgCuiHud.h:95]
}

// ----------------------------------------------------------------------
// FREE-CAM editor-unlock wave (2026-06-29, v12->v13). Six CALLED accessors that
// encapsulate fragile NGE struct byte-offsets so the consumer stops hardcoding offsets
// that drift vs our layout (the particlePreview::retrigger / config::setModalChat shim
// principle). The consumer's "free camera" is OUR DebugPortalCamera: consumer cm_Free == 5
// == GroundScene::CI_debugPortal (cm_FreeChase == 2 == CI_freeChase). These three are
// public-method __fastcall thunks (CALLED, not detoured -> a call-through thunk is correct;
// __fastcall(pThis /*ECX*/, int /*EDX*/) == __thiscall, MSVC v145 forbids __thiscall on a
// free function, C3865). The two messageQueue read-side rows are flat-class PMFs (dyn[]),
// and groundScene::getDebugPortalCameraMessageQueue is a friend forwarder (GroundScene.cpp,
// private member) declared in engine_groundScene_forward.h.
// ----------------------------------------------------------------------

// groundScene::isFreeCameraActive -- replaces the consumer's currentView field read.
// getCurrentView() is PUBLIC [GroundScene.h:207] so no friend is needed; CI_debugPortal is the
// PUBLIC CameraIds enumerator [GroundScene.h:84-95] == 5 == consumer cm_Free. Consumer typedef:
//   bool(__thiscall*)(GroundScene*)
static bool __fastcall engine_groundSceneIsFreeCameraActive(ENGINE_THIS(GroundScene *))
{
	return pThis->getCurrentView() == GroundScene::CI_debugPortal;  // cm_Free (5) == CI_debugPortal
}

// gameCamera::getMessageQueue -- the camera's movement MessageQueue that the camera's alter()
// drains (the consumer read it at the hardcoded GameCamera+0x248). The camera's m_queue is wired
// to its CameraController's MQ (GroundScene.cpp:776,803), so the controller's MQ IS that pointer;
// reaching it via the base Object::getController() [Object.h:190] + Controller::getMessageQueue()
// [Controller.h:67] is layout-independent and works for any camera (no subclass m_queue offset).
// While free-cam is active this aliases groundScene::getDebugPortalCameraMessageQueue. Cameras are
// single-inheritance, so the GameCamera* (from the already-advertised getCurrentCamera) needs no
// adjustment. Consumer typedef: MessageQueue*(__thiscall*)(GameCamera*)
static MessageQueue * __fastcall engine_gameCameraGetMessageQueue(ENGINE_THIS(GameCamera *))
{
	Controller * const c = pThis->getController();                  // non-const this -> non-const overload [Object.h:190]
	return c ? c->getMessageQueue() : 0;                            // [Controller.h:67]
}

// object::isActive -- Object::isActive() is NON-VIRTUAL but INLINE [Object.h:158 decl / :1328 def],
// so &Object::isActive has NO ODR-emitted address (the g_mainLoopCounter inline-accessor Pitfall).
// A CALLED external-linkage shim supplies the address. Consumer typedef: bool(__thiscall*)(const Object*)
static bool __fastcall engine_objectIsActive(ENGINE_THIS(const Object *))
{
	return pThis->isActive();                                       // public inline [Object.h:1328]
}

// ----------------------------------------------------------------------
// systemMessageManager::sendMessageUtf8 -- sysmsg SEND rev-2 (2026-07-03, v14 -> v15).
// extern "C" utf8 shim over CuiSystemMessageManager::sendFakeSystemMessage(const
// Unicode::String&, bool) [CuiSystemMessageManager.h:38].
//
// WHY A SHIM (the v14 lesson): the v14 direct-&fn row systemMessageManager::sendMessage
// CRASHED live smoke (WRITE-AV). The signature matched but the PARAMETER is a C++ string
// object passed by reference: the consumer's swg::WString models the 2002 SWGEmu 3-pointer
// string layout (begin/end/allocEnd, 12 bytes), while this build's Unicode::String is a v145
// SSO basic_string (24 bytes) -- the engine read _Mysize past the end of the consumer's
// object -> garbage size -> wild write. ABI RULE (rev-2 handoff, both repos): only PRIMITIVES
// and POINTERS cross the advertised boundary on CALLED endpoints; any class-type parameter
// (strings above all) needs a provider-side extern "C" shim -- the engine_replayClientEffect
// precedent. The widen happens HERE, on OUR side (our CRT, our string layout).
// narrowToWide is a per-byte widen -- correct for the ASCII editor text the consumer sends
// (rev-2 defers real UTF-8 decoding as unnecessary).
// Threading: consumer calls game-thread-only (main-loop marshaled), never per-frame.
// ----------------------------------------------------------------------
extern "C" void __cdecl engine_sendFakeSystemMessage(const char * utf8Msg, bool chatBoxOnly)
{
	if (!utf8Msg)
		return;
	CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(utf8Msg), chatBoxOnly);
}

// ----------------------------------------------------------------------
// camera::{getProjectionMatrix, getTransformO2W} -- Goal B Wave-3 rider 4C
// (2026-07-18). The consumer's gizmo read camera matrices via RAW STRUCT
// OFFSETS (SWGEmu layout) and crashed on the NGE layout; these copy-out
// accessors are the primitives-only replacements (ABI RULE). Layouts:
//   out16: the engine GlMatrix4x4 verbatim -- row-major float[4][4]
//          (Camera::getProjectionMatrix(), the matrix the renderer consumes).
//   out12: row-major 3x4, position = column 3 -- the SAME convention as
//          EngineWsNodeInfo.transform (composed via the Transform column
//          accessors, independent of Transform's internal representation).
// Returns 1 ok / 0 no-current-camera-or-null-arg. CALLED, game-thread-only,
// per-frame-safe (plain copies, no allocation).
// ----------------------------------------------------------------------
extern "C" int __cdecl engine_getCameraProjectionMatrix(float * out16)
{
	if (!out16)
		return 0;

	const Camera * const camera = Game::getConstCamera();
	if (!camera)
		return 0;

	std::memcpy(out16, &camera->getProjectionMatrix().matrix[0][0], 16 * sizeof(float));
	return 1;
}

extern "C" int __cdecl engine_getCameraTransformO2W(float * out12)
{
	if (!out12)
		return 0;

	const Camera * const camera = Game::getConstCamera();
	if (!camera)
		return 0;

	const Transform & t = camera->getTransform_o2w();
	const Vector i = t.getLocalFrameI_p();
	const Vector j = t.getLocalFrameJ_p();
	const Vector k = t.getLocalFrameK_p();
	const Vector p = t.getPosition_p();
	out12[ 0] = i.x;  out12[ 1] = j.x;  out12[ 2] = k.x;  out12[ 3] = p.x;
	out12[ 4] = i.y;  out12[ 5] = j.y;  out12[ 6] = k.y;  out12[ 7] = p.y;
	out12[ 8] = i.z;  out12[ 9] = j.z;  out12[10] = k.z;  out12[11] = p.z;
	return 1;
}

// ----------------------------------------------------------------------
// clientWorld::collideScreenRay -- Live World Editor ray-pick (v19 -> v20,
// 2026-07-19 change request). Copy-out cursor ray-cast: the consumer's
// collideCursorWithWorld (Utinni cui_hud.cpp:221) done ENGINE-SIDE so the
// NGE-unsafe pieces (CollisionInfo layout, ClientWorld::collide's added
// CollideParameters param, camera viewport math) never cross the boundary
// (ABI RULE -- the rider-4C camera-accessor shape). Mirrors the engine's own
// cursor pick (SwgCuiHud.cpp findObjectByPolygon:230 / hitReticle:279): ray
// from the CURRENT camera through screen pixel (x,y) (reverseProjectInScreen-
// Space handles the viewport offset), length = ConfigClientGame::
// getTargetingRange() (the hud's targeting viewDistance, SwgCuiHud.cpp:1163),
// camera parent cell as the start cell, CollideParameters::cms_default,
// player excluded (the engine's &self). Flags = the hud's nine-flag pick set;
// objectsOnly=1 drops the three NON-OBJECT geometry classes (terrain,
// terrainFlora, interiorGeometry) so only client objects report.
// Hit id resolution: the terrain instance reports id 0 (a VALID hit -- the
// place-at-cursor case); otherwise the hit object's NetworkId, walking up
// getParent() to the nearest NETWORKED ancestor when the immediate hit is a
// non-networked TRUE CHILD part (a POB door part resolves to an id the
// consumer can feed network::getObjectById). NOTE (v22 correction):
// getParent() is m_childObject-gated [Object.h:604] -- cell-CONTAINED
// objects attach with asChildObject=false, so an id-less .ilf decoration
// reports id 0 WITHOUT dissolving into the cell/building id (the correct
// editor semantic; pair with collideScreenRayObject for the pointer).
// Returns 1 = hit (outs filled), 0 = miss/no-camera/null-outs (outs zeroed).
// CALLED, game-thread-only, per-frame-safe (stack CollisionInfo, no alloc).
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// object::getTransformO2P -- copy-out o2p read (v23 -> v24, 2026-07-19
// change request #5; the last accessor model-D persistence needs). The .ilf
// stores o2p (object-to-parent-CELL); the gizmo drives o2w; this reads the
// object's o2p directly. Object::getTransform_o2p() is INLINE + returns
// const Transform& [Object.h:233/:652] -> copy-out shim mandatory (ABI
// RULE), byte-for-byte the camera::getTransformO2W layout: row-major 3x4,
// columns = local frame i/j/k, column 3 = position (the .ilf /
// EngineWsNodeInfo convention). The Object* is BORROWED consumer-held
// (their pick rows) -- null-checked only; lifetime discipline is theirs.
// CALLED, game-thread-only, per-frame-safe (plain copy, no allocation).
// ----------------------------------------------------------------------
extern "C" int __cdecl engine_getObjectTransformO2P(void * object, float * out12)
{
	if (!object || !out12)
		return 0;

	const Transform & t = static_cast<const Object *>(object)->getTransform_o2p();
	const Vector i = t.getLocalFrameI_p();
	const Vector j = t.getLocalFrameJ_p();
	const Vector k = t.getLocalFrameK_p();
	const Vector p = t.getPosition_p();
	out12[ 0] = i.x;  out12[ 1] = j.x;  out12[ 2] = k.x;  out12[ 3] = p.x;
	out12[ 4] = i.y;  out12[ 5] = j.y;  out12[ 6] = k.y;  out12[ 7] = p.y;
	out12[ 8] = i.z;  out12[ 9] = j.z;  out12[10] = k.z;  out12[11] = p.z;
	return 1;
}

// ----------------------------------------------------------------------
// object::getContainingBuildingId -- containing-POB building id (v24 -> v25,
// 2026-07-30 change request #6; the model-D Arm-step unblock). From any
// cell-contained object to the NetworkId of the POB building that contains
// it -- the .ws node id wsSetNodeTemplateName takes, whose template carries
// the interiorLayoutFileName model-D re-points. The whole chain is inline /
// reference-returning -> shim mandatory (ABI RULE):
//   cell   = object's OWN CellProperty (a wall-click resolves to the CELL
//            object; Object::getParentCell [Object.cpp:1372] walks
//            ANCESTORS, so on a cell it lands in the WORLD cell and would
//            wrongly report "not inside") else object->getParentCell()
//            (never null -- world-cell fallback);
//   portal = cell->getPortalProperty() [CellProperty.h:119/inline :270] --
//            NULL for the world cell = not inside a POB -> 0;
//   building = portal->getOwner() [Property.h:34/inline :57 via Container]
//            -> getNetworkId().getValue().
// The Object* is BORROWED consumer-held (their pick rows) -- null-checked
// only; lifetime discipline is theirs (the getTransformO2P precedent).
// CALLED, game-thread-only, per-frame-safe (pointer hops + a value read).
// ----------------------------------------------------------------------
extern "C" __int64 __cdecl engine_getContainingBuildingId(void * object)
{
	if (!object)
		return 0;

	const Object * const obj = static_cast<const Object *>(object);

	const CellProperty * cell = obj->getCellProperty();
	if (!cell)
		cell = obj->getParentCell();
	if (!cell)
		return 0;

	const PortalProperty * const portal = cell->getPortalProperty();
	if (!portal)
		return 0;

	return portal->getOwner().getNetworkId().getValue();
}

// ----------------------------------------------------------------------
// object::setParentCell -- cell reparenting (v26 -> v27, SWG-Toolkit change
// request 2026-08-02). VIRTUAL [Object.h:168] -> shim mandatory (ABI RULE).
//
// WHY: writing only the world transform leaves the object parented to the
// WORLD cell, so the engine evaluates the outdoor portal set from a position
// that is physically indoors -> interiors render see-through from the inside.
// Repairs itself on a portal crossing, which is why walking out the door
// "fixes" it. Also fires on a scene load, so it is not teleport-specific.
// The consumer's larger use is placement routing: the container is resolved
// from the PLACEMENT POINT, never the player (standing in a cantina doorway
// must be able to place OUT to .ws and IN to .ilf) -- the doorway is the
// acceptance test in both directions.
//
// The null-cell guard is LOAD-BEARING, not cosmetic: Object::setParentCell
// NOT_NULLs its argument [Object.cpp:1389], so a null cell is a FATAL, not a
// graceful refusal. To reparent OUT to the exterior the caller must pass the
// world cell explicitly via cellProperty::getWorldCellProperty -- null is NOT
// how you say "outside". Unchanged-cell is a no-op engine-side [:1392].
//
// TRANSFORM: write o2w and do NOT convert. Two independent mechanisms keep
// world semantics -- attachToObject_w [Object.cpp:1956] derives o2p from the
// CURRENT world frame (rotate_w2o/rotateTranslate_w2o), and setTransform_o2w
// [Object.cpp:1450] is itself cell-aware (world-cell parent -> direct o2p,
// else it inverts the cell owner's o2w and back-converts).
//
// ORDERING (rationale CORRECTED 2026-08-02 by the consumer's trace -- the
// original reasoning here was wrong even though the conclusion was right):
// setParentCell does NOT run the portal sweep. It fires only cellChanged(false)
// [:1408], and CellPropertyNamespace::Notification overrides only getPriority/
// positionChanged/positionAndRotationChanged [CellProperty.cpp:42-66] -- NOT
// cellChanged. The sweep is a side effect of the TRANSFORM WRITE, and only the
// transform write. (Object::cellChanged is non-virtual [Object.h:366] so the
// observer set is closed: all of them re-derive from current state and none
// caches a position, which is why the "stale position" argument this comment
// used to make was cosmetic.)
//
// So the real question is whether the sweep runs before or after the
// authoritative cell assignment, and that answers cleanly:
//   transform-first -> the sweep runs against the OLD cell and whatever it
//                      picks is UNCONDITIONALLY overwritten by the trailing
//                      setParentCell. Final cell is deterministic.
//   reparent-first  -> the sweep runs LAST, so a cell it picks is final and
//                      uncorrected: a silent permanent mis-parent, and it is
//                      exactly the value getContainingBuildingId then feeds to
//                      placement routing.
// Transform-first strictly dominates. WRITE THE TRANSFORM FIRST, THEN REPARENT.
//
// BETTER STILL (v28): use the engine's OWN idiom now that
// cellProperty::setPortalTransitionsEnabled is advertised --
// GroundScene.cpp:1492-1497 does reparent -> suppress -> write o2p ->
// unsuppress -> collisionWorld::objectWarped. With the sweep suppressed the
// ordering question does not arise at all. Their cell-first choice is
// CONDITIONAL on that suppression; do not copy it without the bracket.
//
// Both pointers are BORROWED consumer-held; null-checked only, lifetime
// discipline is theirs. CALLED, game-thread-only. 1 = ok, 0 = refused.
// ----------------------------------------------------------------------
extern "C" int __cdecl engine_setParentCell(void * object, void * cellProperty)
{
	if (!object || !cellProperty)
		return 0;

	Object * const obj = static_cast<Object *>(object);
	CellProperty * const cell = static_cast<CellProperty *>(cellProperty);

	obj->setParentCell(cell);
	return 1;
}

// ----------------------------------------------------------------------
// clientWorld::findCellAtWorldPosition -- "which cell contains world point P"
// (v28). THE placement-routing primitive: the container must be resolved from
// the PLACEMENT POINT, and a coordinate-only destination (bookmark, scripted
// placement) has no object to pick, so the collideScreenRayObject ->
// getParentCell path cannot serve it.
//
// Wraps ClientWorld::findClosestCellObjectFromWorldPosition [ClientWorld.h:227
// / .cpp:1649] and folds in the getCellProperty() hop, so the consumer never
// dereferences an engine type. That function is ALSO the client's own
// containment heuristic (its other caller is SwgCuiQuestHelper.cpp:997), so
// tool and engine cannot disagree about which cell a doorway point belongs to.
//
// Never returns null in practice -- the engine falls back to the world cell's
// owner -- but we null-check every hop anyway and fall back explicitly to
// CellProperty::getWorldCellProperty(), so the result is always a valid
// argument for object::setParentCell (which FATALs on null).
// Shim mandatory: takes Vector const&, returns Object const*.
// ----------------------------------------------------------------------
extern "C" void * __cdecl engine_findCellAtWorldPosition(float x, float y, float z)
{
	Object const * const cellObject = ClientWorld::findClosestCellObjectFromWorldPosition(Vector(x, y, z));
	if (cellObject)
	{
		CellProperty const * const cell = cellObject->getCellProperty();
		if (cell)
			return const_cast<CellProperty *>(cell);
	}

	return CellProperty::getWorldCellProperty();
}

// ----------------------------------------------------------------------
// object::getAttachedTo -- child-object guard (v28). SAFETY, not convenience.
//
// setParentCell on a MOUNTED player silently corrupts its pose in Release: the
// DEBUG_FATAL(isChildObject(), ...) at Object.cpp:1396 is #if 0'd out, so
// nothing stops it. Traced path: isInWorldCell() returns true THROUGH the
// mount so the detach at :1400 is skipped; attachToObject_w computes the right
// m_objectToParent at :1968-1969; then attachToObject_p re-enters
// detachFromObject(DF_none) (:1913-1914) whose :2002
// `m_objectToParent = getTransform_o2w()` overwrites it with a mount-composed
// value.
//
// Both Object::getAttachedTo [Object.h:628/640] and Object::isChildObject
// [:1289] are INLINE -> no PMF address -> shim. One row covers both needs:
// non-null == attached to a parent object (mounted/child) == do not reparent.
// Returns the parent as an opaque void*; borrowed, no consumer dereference.
// ----------------------------------------------------------------------
extern "C" void * __cdecl engine_getAttachedTo(void * object)
{
	if (!object)
		return 0;

	return const_cast<Object *>(static_cast<const Object *>(object)->getAttachedTo());
}

// ----------------------------------------------------------------------
// object::isChildObject -- THE mount discriminator (v28 -> v29).
//
// CORRECTS THE v28 HANDBACK. We advertised object::getAttachedTo as the mount
// guard and said "non-null means do not reparent". That is FALSE and was
// falsified live by the consumer: it refused every teleport made from INSIDE a
// building. Cell attachment and mount attachment share the SAME field --
// Object::setParentCell attaches via attachToObject_w(&cellProperty->getOwner(),
// false) [Object.cpp:1404-1405], so any player standing in a POB has a non-null
// getAttachedTo (the cell owner). getAttachedTo cannot tell "mounted" from
// "indoors", and indoors is the entire editing workflow.
//
// m_childObject is the real discriminator: it is set ONLY from the
// asChildObject argument [Object.cpp:1931], and cells are attached with
// asChildObject=FALSE, so it is false for cell parentage and true for a genuine
// child/mount attachment. It is exactly what the compiled-out
// DEBUG_FATAL(isChildObject(), ...) at Object.cpp:1396 tests -- the assert that
// would have caught the mounted-player pose corruption if it were still armed.
//
// Object::isChildObject is INLINE [Object.h:1289] -> no PMF address -> shim.
// 1 = child/mounted (do NOT reparent) · 0 = not a child, or null.
// ----------------------------------------------------------------------
extern "C" int __cdecl engine_isChildObject(void * object)
{
	if (!object)
		return 0;

	return static_cast<const Object *>(object)->isChildObject() ? 1 : 0;
}

// ----------------------------------------------------------------------
// cellProperty::getCellName -- the cell NAME behind a CellProperty* (v32).
//
// The consumer's .ilf writer stores the cell name as a LITERAL asciiz string in every
// decoration row, and until now they wrote an OPERATOR-TYPED name rather than one derived
// from the placement point -- so a wrong-cell row looked perfectly healthy on disk. Their
// primary target has 262 rows across 11 distinct cells, so "which cell" is a real 11-way
// choice, not a formality.
//
// Nothing is conceptually missing from the engine: CellProperty::getCellName()
// [CellProperty.h:120, inline at :249-252] already returns exactly the right string. It is
// INLINE, so there is no out-of-line symbol to advertise -- purely an ABI-visibility gap,
// the same shape as object::isChildObject (v29).
//
// COPY-OUT rather than returning the pointer, deliberately, and it is the shape the
// consumer offered. m_cellName is a `const char *` assigned from cellTemplate.getName()
// [CellProperty.cpp:456], so it points into the TEMPLATE and its lifetime is the template's,
// not the cell's. Handing that across the ABI would make the consumer reason about a
// lifetime they cannot see; copying sidesteps it entirely. wsGetSavePath convention:
// returns the needed length INCLUDING the NUL, so a caller can size a buffer from a
// too-small first call. 0 = null input / no name.
//
// The WORLD cell returns "world", not null (set at CellProperty.cpp:225) -- the consumer
// asked; treat it as "not an interior cell, do not write an .ilf row".
//
// Borrowed pointer (theirs, from clientWorld::findCellAtWorldPosition), game-thread-only.
// ----------------------------------------------------------------------
extern "C" int __cdecl engine_getCellName(void * cellProperty, char * buf, int cap)
{
	if (!cellProperty)
		return 0;

	const CellProperty * const cell = static_cast<const CellProperty *>(cellProperty);
	const char * const name = cell->getCellName();
	if (!name)
		return 0;

	int const needed = static_cast<int>(strlen(name)) + 1;

	if (buf && cap >= needed)
		memcpy(buf, name, static_cast<size_t>(needed));

	return needed;
}

// Shared ray core for the two pick shims below (v22 refactor -- identical ray,
// two return shapes). Returns false on no-camera/degenerate-ray/miss.
static bool engine_screenRayCollide(int screenX, int screenY, int objectsOnly, CollisionInfo & info)
{
	const Camera * const camera = Game::getConstCamera();
	if (!camera)
		return false;

	const Vector worldStart = camera->getPosition_w();
	Vector viewDirection = camera->rotate_o2w(camera->reverseProjectInScreenSpace(screenX, screenY));
	if (!viewDirection.normalize())
		return false;

	const Vector worldEnd = worldStart + viewDirection * ConfigClientGame::getTargetingRange();

	uint16 flags = ClientWorld::CF_terrain
	             | ClientWorld::CF_terrainFlora
	             | ClientWorld::CF_tangible
	             | ClientWorld::CF_tangibleNotTargetable
	             | ClientWorld::CF_tangibleFlora
	             | ClientWorld::CF_interiorObjects
	             | ClientWorld::CF_interiorGeometry
	             | ClientWorld::CF_skeletal
	             | ClientWorld::CF_childObjects;
	if (objectsOnly)
		flags = static_cast<uint16>(flags & ~(ClientWorld::CF_terrain | ClientWorld::CF_terrainFlora | ClientWorld::CF_interiorGeometry));

	return ClientWorld::collide(Game::getConstCamera()->getParentCell(), worldStart, worldEnd, CollideParameters::cms_default, info, flags, Game::getPlayer());
}

extern "C" int __cdecl engine_collideScreenRay(int screenX, int screenY, int objectsOnly, __int64 * outHitObjectId, float * outPoint3)
{
	if (!outHitObjectId || !outPoint3)
		return 0;

	*outHitObjectId = 0;
	outPoint3[0] = outPoint3[1] = outPoint3[2] = 0.0f;

	CollisionInfo info;
	if (!engine_screenRayCollide(screenX, screenY, objectsOnly, info))
		return 0;

	const Vector & point = info.getPoint();
	outPoint3[0] = point.x;
	outPoint3[1] = point.y;
	outPoint3[2] = point.z;

	const Object * const hitObject = info.getObject();
	if (hitObject && hitObject != static_cast<const Object *>(TerrainObject::getInstance()))
	{
		for (const Object * o = hitObject; o; o = o->getParent())
		{
			const NetworkId & id = o->getNetworkId();
			if (id != NetworkId::cms_invalid)
			{
				*outHitObjectId = id.getValue();
				break;
			}
		}
	}

	return 1;
}

// ----------------------------------------------------------------------
// clientWorld::collideScreenRayObject -- borrowed-Object* pick (v21 -> v22,
// 2026-07-19 change request #3; the fallback pre-described in the
// hybrid-incell ANSWERS addendum). The consumer's CONSULT-69 layer probe
// MEASURED that pure .ilf decorations never reach the hud pick
// (cuiHud::getTarget = null for an id-less table the ray demonstrably hits)
// -- so id-keyed AND watcher-keyed selection both miss that layer. This row
// returns the RAW nearest-hit Object* from the same ray as collideScreenRay:
// NO ancestor walk, NO id resolution -- the hit may be an .ilf decoration,
// a networked tangible, a child part, or the BUILDING itself when the ray
// hits cell geometry. Consumers pair it with collideScreenRay for layer
// triage (id != 0 there = networked; wsGetNodeInfo(id) hit = snapshot).
// BORROWED pointer, game-thread-only: valid until the owning building
// leaves world (the single delete site, TangibleObject.cpp:502); consumers
// clear on cell/zone change and must never cache across a zone. Gizmo-ing
// a SERVER-streamed hit desyncs from the server -- consumer warns/refuses.
// ----------------------------------------------------------------------
extern "C" void * __cdecl engine_collideScreenRayObject(int screenX, int screenY, int objectsOnly)
{
	CollisionInfo info;
	if (!engine_screenRayCollide(screenX, screenY, objectsOnly, info))
		return 0;

	return const_cast<Object *>(info.getObject());
}

// ----------------------------------------------------------------------
// game::getSceneId -- current scene id copy-out (v20 -> v21, 2026-07-19
// change request #2: one-click "Reload current scene" + auto-naming the
// imported .ws). Game::getSceneId() is INLINE and returns const
// std::string& -- doubly un-advertisable (no ODR address; class type across
// the boundary, the v14 sysmsg lesson) -> copy-out shim, the
// wsGetSavePath convention: returns needed length INCLUDING the NUL
// (size-first: call with null/0 to size, then with a buffer); 0 = no scene
// loaded (ms_sceneId empty -- charselect/pre-world). The id is the SAME
// string WorldSnapshot::load()/wsSaveSnapshot() key the .ws filename on
// (e.g. "tatooine"), so unload() + load(getSceneId()) is the correct
// one-click reload and picks up a just-saved override .ws.
// CALLED, game-thread-only, trivially per-frame-safe.
// ----------------------------------------------------------------------
extern "C" int __cdecl engine_getSceneId(char * buf, int cap)
{
	const std::string & sceneId = Game::getSceneId();
	if (sceneId.empty())
		return 0;

	const int needed = static_cast<int>(sceneId.size()) + 1;
	if (buf && cap > 0)
		std::memcpy(buf, sceneId.c_str(), static_cast<size_t>(cap < needed ? cap : needed));

	return needed;
}

// ----------------------------------------------------------------------
// client::advertisedArchBits -- the consumer's fail-closed arch assert (v34,
// the 2026-08-15 dual-arch wave; consumer CHANGE-REQUEST ask #5). Returns the
// provider's pointer width in bits (32 / 64), a compile-time per-arch constant.
// WHY: the consumer's export-absent fallback path seeds x86 SWGEmu RVA
// literals; on x64 there is no legacy client, so an x64 agent must FAIL CLOSED
// when the surface is missing -- and when it IS present, this row lets the
// agent assert "this table is the x64 contract" from the table itself instead
// of inferring it from the PE header. CALLED, thread-safe anywhere (pure
// constant), pre-CRT-safe like the rest of the fill.
// ----------------------------------------------------------------------
extern "C" int __cdecl engine_getAdvertisedArchBits(void)
{
	return static_cast<int>(sizeof(void *) * 8);
}

// ----------------------------------------------------------------------
// The advertised table. CANONICAL FORM (pinned 2026-06-21): NO null-pair
// sentinel terminator row; count = sizeof/sizeof (NO -1). 37-02/03 MUST NOT
// reintroduce a sentinel. Per-row symbol kind is noted in the comment.
// ----------------------------------------------------------------------
// NOTE: NON-const -- the 29 function-call rows are { name, 0 } compile-time-constant
// placeholders (so the whole array is image-valid at module load, no static-init tail
// deferral), completed at runtime by ensureDynamicRowsFilled() on the reader's thread.
static EngineHookPoint s_engineHookPoints[] =
{
	// -- config (sharedFoundation, all static; ConfigFile.h) -------------------
	{ "config::loadOverrideConfig",   (void *)&engine_loadOverrideConfig },        // EPA-02 crash-fixer thunk (installConfigFileOverride, not the buffer-loader)
	{ "config::loadConfigFileBuffer", (void *)&ConfigFile::loadFromBuffer },       // static bool loadFromBuffer(char const*,int) [ConfigFile.h:136]
	{ "config::loadConfigFileString", (void *)&ConfigFile::loadFile },             // static bool loadFile(char const*) [ConfigFile.h:135] (MISMATCH: spec "loadFromString" -> ours is loadFile)
	{ "config::setModalChat",         (void *)&CuiPreferences::setModalChat },     // 38-02 (37-02 CORRECTION): plain &fn -- a PUBLIC CuiPreferences static, NOT config/ConfigFile [CuiPreferences.h:95, def CuiPreferences.cpp:1267]; contract name stays config::setModalChat
	{ "config::getModalChat",         (void *)&CuiPreferences::getModalChat },     // 38-02 (37-02 CORRECTION): plain &fn -- a PUBLIC CuiPreferences static, NOT config/ConfigFile [CuiPreferences.h:94, def CuiPreferences.cpp:1655]

	// -- client (exe entry + win32 exe-statics; ClientMain.h / Os.cpp / DebugHelp.cpp) --
	{ "client::clientMain",           (void *)&ClientMain },                       // int ClientMain(HINSTANCE,HINSTANCE,LPSTR,int) __cdecl [ClientMain.h:13]
	{ "client::wndProc",              (void *)&engine_osWindowProc },              // 38-02: external __stdcall/CALLBACK shim in Os.cpp over the PRIVATE Os::WindowProc [Os.h:138] (friend-granted member access); CALLBACK preserved
	{ "client::writeMiniDump",        (void *)&engine_writeMiniDump },             // 38-02: external shim in DebugHelp.cpp over DebugHelp::writeMiniDump [DebugHelp.h:36] (win32-private header not on the exe include path)
	{ "client::advertisedArchBits",   (void *)&engine_getAdvertisedArchBits },     // v34 NEW: int (void) -- provider pointer width in bits (32/64), compile-time per-arch. The consumer's fail-closed arch assert (their x64 agent must never fall back to x86 RVA literals); shim above
	// OMIT (38-02, D-04 / Pitfall 5): client::writeCrashLog + client::setupStartDataInstall are NONEXISTENT in this tree (grep = 0 source hits). The crash .txt is written INLINE by SetupSharedFoundation's exception handler [SetupSharedFoundation.cpp:92 sprintf] -- no named writeCrashLog function; setupStartDataInstall is a SWGEmu Pre-CU concept with no from-source twin. NOT advertised (never guessed); FLAGGED for the EPA-08 handback.

	// -- game (clientGame, all static; Game.h) ---------------------------------
	{ "game::install",                (void *)&Game::install },                    // static void install(Application) [Game.h:94]
	{ "game::quit",                   (void *)&Game::quit },                       // static void quit() [Game.h:98]
	{ "game::mainLoop",               (void *)&Game::runGameLoopOnce },            // 24-4a RE-POINT (was &Game::run): Game::run [Game.h:96] is the OUTER once-per-process loop (`while(!isOver()) runGameLoopOnce(false,NULL,0,0)` Game.cpp:1029); the PER-FRAME tick is static void runGameLoopOnce(bool presentToWindow,HWND,int width,int height) [Game.h:103/Game.cpp:1059] -- EXACT __cdecl signature match to Utinni hkMainLoop. (Game::run available as a separate row on request.)
	{ "game::setupScene",             (void *)&engine_gameSetupScene },             // 2026-06-25 RE-MAP (was the strings-overload &Game::setScene): __cdecl(void*) thunk over Game::_setScene(Scene*) -- the editor's build-then-set path passes a groundScene::ctor GroundScene*, NOT filenames; the old overload crashed (0xC0000005 in GroundScene::GroundScene). Same name -> address re-map only, no version bump.
	{ "game::loadScene",              (void *)&engine_gameLoadScene },             // CONSULT 2026-06-25 NAME ADD: __cdecl(const char* terrain, const char* player) FULL scene-load via the SceneCreator lifecycle -> Game::setScene(true, terrain, player, nullptr). The editor passes FILENAMES (not a pre-built GroundScene); fixes the half-integrated-scene next-frame throw that setupScene/_setScene(Scene*) leaves. immediately=true + customizedPlayer=nullptr mirror the engine's own by-name loaders (SwgCuiSceneSelection/Locations/CommandParserScene).
	{ "game::cleanupScene",           (void *)&Game::cleanupScene },               // static void cleanupScene() [Game.h:101]
	{ "game::getPlayer",              (void *)&Game::getPlayer },                  // static Object* getPlayer() [Game.h:150]
	{ "game::getPlayerCreatureObject",(void *)&Game::getPlayerCreature },          // static CreatureObject* getPlayerCreature() [Game.h:157] (MISMATCH name)
	{ "game::getCamera",              (void *)&Game::getCamera },                  // static Camera* getCamera() [Game.h:173] -- NOT overloaded (const sibling is getConstCamera); NO cast
	{ "game::getConstCamera",         (void *)&Game::getConstCamera },             // static Camera const* getConstCamera() [Game.h:174]
	{ "game::isViewFirstPerson",      (void *)&Game::isViewFirstPerson },          // static bool isViewFirstPerson() [Game.h:185]
	{ "game::isHudSceneTypeSpace",    (void *)&Game::isHudSceneTypeSpace },        // static bool isHudSceneTypeSpace() [Game.h:215]
	{ "game::g_runningFlags",         (void *)&Game::isOver },                     // ACCESSOR (sec 8 #3): ms_done is private; isOver() is the NON-inline accessor [Game.h:134 / Game.cpp:1021] -- call-not-read
	{ "game::g_mainLoopCounter",      (void *)&Game::getMainLoopCount },           // 24-4b NEW: ACCESSOR for the per-frame counter ms_loops (private, ++ at Game.cpp:1248). getLoopCount() [Game.h:190] is INLINE (no ODR addr); getMainLoopCount() is the NEW out-of-line twin [Game.h / Game.cpp] mirroring isOver() -- call-not-read, replaces the consumer's hardcoded 0x1908830 read
	{ "game::registerTickCallback",   (void *)&Game::setExternalTickCallback },    // v35 NEW (toolkit x64 round-2 ask #3): void (void (__cdecl* fn)(void)) -- fn fires once at the TOP of Game::runGameLoopOnce, game thread, OUTSIDE any render call chain (previous frame fully presented). The consumer's deferred-command drain point (e.g. scene swaps that must never run inside its frame callback). game::mainLoop stays advertised unchanged for detour-based consumers. Single-slot, null clears
	{ "game::getShutdownPhase",       (void *)&Game::getShutdownPhase },           // v26 NEW: ACCESSOR for the process-wide monotonic shutdown phase (0=running 1=requested 2=unwinding). Out-of-line forwarder [Game.h / Game.cpp] to ExitChain::getShutdownPhase(); state raised in Game::quit() + the loop-exit ms_done site (->1) and at the top of ExitChain::run() (->2, after the re-entrancy guard, before the first dtor). NOTE: ExitChain::isRunning() is NOT usable for this -- it reads PerThreadData (per-THREAD), so a consumer polling from its own thread always sees false. Unlike game::g_runningFlags (&Game::isOver) this stays safe to call DURING teardown -- isOver() dereferences IoWinManager/Os state. call-not-read

	// -- graphics (clientGraphics facade, all static; Graphics.h) [EPA-03] -----
	{ "graphics::install",            (void *)&Graphics::install },                // static bool install() [Graphics.h:70] -- EPA-03 DX11 overlay kickoff row
	{ "graphics::update",             (void *)&Graphics::update },                 // static void update(float) [Graphics.h:153]
	{ "graphics::beginScene",         (void *)&Graphics::beginScene },             // static void beginScene() [Graphics.h:155]
	{ "graphics::endScene",           (void *)&Graphics::endScene },               // static void endScene() [Graphics.h:156]
	{ "graphics::present",            (void *)static_cast<bool(*)()>(&Graphics::present) },              // OVERLOADED [Graphics.h:161] -> no-arg present()
	{ "graphics::presentWindow",      (void *)static_cast<bool(*)(HWND, int, int)>(&Graphics::present) },// OVERLOADED [Graphics.h:162] -> present(HWND,int,int)
	{ "graphics::resize",             (void *)&Graphics::resize },                 // static void resize(int,int) [Graphics.h:130]
	{ "graphics::flushResources",     (void *)&Graphics::flushResources },         // static void flushResources(bool) [Graphics.h:81]
	{ "graphics::screenshot",         (void *)&Graphics::screenShot },             // static bool screenShot(const char*) [Graphics.h:170] (MISMATCH name)
	{ "graphics::useHardwareCursor",  (void *)&Graphics::setHardwareMouseCursorEnabled }, // static void setHardwareMouseCursorEnabled(bool) [Graphics.h:177] (MISMATCH name)
	{ "graphics::showMouseCursor",    (void *)&Graphics::showMouseCursor },        // static bool showMouseCursor(bool) [Graphics.h:180]
	{ "graphics::setSystemMouseCursorPosition", (void *)&Graphics::setSystemMouseCursorPosition }, // static void(int,int) [Graphics.h:181]
	{ "graphics::setStaticShader",    (void *)&Graphics::setStaticShader },        // static void setStaticShader(const StaticShader&,int) [Graphics.h:175]
	{ "graphics::g_renderTargetWidth",  (void *)&Graphics::getCurrentRenderTargetWidth },  // ACCESSOR (sec 8 #3): RT width behind a static getter [Graphics.h:103] -- call-not-read
	{ "graphics::g_renderTargetHeight", (void *)&Graphics::getCurrentRenderTargetHeight }, // ACCESSOR (sec 8 #3): RT height behind a static getter [Graphics.h:104] -- call-not-read
	{ "graphics::registerFrameCallback",  (void *)&Graphics::setFrameCallback },  // v35 NEW (toolkit x64 round-2 ask #1): void (void (__cdecl* fn)(void)) -- fn fires on the render thread in the D3D11 present path, AFTER the provider's last back-buffer write (post-BCG/gamma), BEFORE the swap-chain Present. The no-detour overlay draw point (retires the consumer's DXGI vtable patch on BOTH arches). Single-slot, null clears; D3D9 plugins accept-and-ignore (overlay is D3D11-only); register from a live session (post graphics install)
	{ "graphics::registerResizeCallback", (void *)&Graphics::setResizeCallback }, // v35 NEW (ask #2): void (void (__cdecl* fn)(int phase, int width, int height)) -- fires around the D3D11 back-buffer resize: phase 0 BEFORE ResizeBuffers (consumer MUST release every back-buffer-referencing view NOW -- an outstanding reference fails ResizeBuffers), phase 1 after the new views exist (rebuild). Both phases carry the new client size. Same slot rules as the frame callback

	// -- scene::groundScene (clientGame; GroundScene.h) -- 38-01, EPA-05 ---------
	// GroundScene is MULTIPLE-INHERITANCE (NetworkScene : public Scene, public
	// MessageDispatch::Receiver [NetworkScene.h:28-30]) -> every PMF is inflated,
	// so NONE of these use pmfToVoid(&GroundScene::member) (would trip the C2338
	// sizeof guard). 3 PUBLIC methods + 1 ctor are __fastcall thunks defined above;
	// the 4 PRIVATE methods are __fastcall forwarders defined in GroundScene.cpp
	// (member access) and declared in engine_groundScene_forward.h.
	{ "groundScene::ctor",                 (void *)&engine_groundSceneCtor },                    // placement-new MI ctor thunk -> GroundScene(const char*,const char*,CreatureObject*) [GroundScene.h:199] (NOT the NetworkId overload :200)
	{ "groundScene::init",                 (void *)&engine_groundSceneInit },                    // PRIVATE [GroundScene.h:173] -> in-TU GroundScene.cpp forwarder (member access)
	{ "groundScene::reloadTerrain",        (void *)&engine_groundSceneReloadTerrain },           // public [GroundScene.h:215] -> MI __fastcall thunk
	{ "groundScene::changeCamera",         (void *)&engine_groundSceneChangeCamera },            // public [GroundScene.h:207] MISMATCH: ours setView(int,float) -> MI __fastcall thunk
	{ "groundScene::getCurrentCamera",     (void *)&engine_groundSceneGetCurrentCamera },        // public [GroundScene.h:212] (non-const overload; const sibling :213) -> MI __fastcall thunk
	{ "groundScene::update", 0 },               // REAL ENTRY (detoured by Utinni; delta==0 verified) -- 38-05. PRIVATE GroundScene::update(float) [GroundScene.h:103]; real-entry accessor in GroundScene.cpp (was a call-through forwarder -> silently dead for a detour)
	{ "groundScene::handleInputMapUpdate", (void *)&engine_groundSceneHandleInputMapUpdate },    // PRIVATE [GroundScene.h:170] -> in-TU GroundScene.cpp forwarder (CALLED/unused row -- forwarder is correct here, NOT detoured)
	{ "groundScene::handleInputMapEvent", 0 },  // REAL ENTRY (detoured by Utinni; delta==0 verified) -- 38-05. PRIVATE GroundScene::handleInputMapEvent(IoEvent*) [GroundScene.h:194]; real-entry accessor in GroundScene.cpp (was a call-through forwarder -> silently dead for a detour)
	// SKIP: groundScene::draw -- VIRTUAL [GroundScene.h:204] (also in the VIRTUAL SKIPS block); Utinni resolves off the live vtable. Not advertised.
	// OMIT: groundScene::g_instance -- no dedicated GroundScene singleton; reached via INLINE Game::getScene() [Game.h:306] (no ODR address) cast to GroundScene, and Game::ms_scene is private [Game.h:271]. OMITTED (graceful degradation); FLAGGED for the EPA-08 handback -- if Utinni's groundScene editor strictly needs the raw singleton pointer, add a non-inline Game accessor in a follow-up.

	// -- cui::chatWindow (swgClientUserInterface; SwgCuiChatWindow.h) -- 38-03, EPA-05 --
	// SwgCuiChatWindow is TRIPLE-INHERITANCE (public SwgCuiLockableMediator, public
	// UINotification, public MessageDispatch::Receiver [SwgCuiChatWindow.h:58-61]) ->
	// every PMF is inflated, so NONE of these use pmfToVoid(&SwgCuiChatWindow::member)
	// (would trip the C2338 sizeof guard). All 4 target PUBLIC non-virtual methods, so
	// they are reached directly by the __fastcall thunks defined above (no friend decl).
	// Contract names are the SPEC names; the in-tree method NAME MISMATCH is in each comment.
	{ "cuiChatWindow::enableTextInput", 0 },  // REAL ENTRY (detoured by Utinni hkEnableTextInput; delta==0 verified) -- 38-05. PUBLIC non-virtual acceptTextInput(bool,bool,bool) [SwgCuiChatWindow.h:112], MISMATCH contract enableTextInput. Was a call-through MI thunk -> silently dead for a detour.
	{ "cuiChatWindow::writeToAllTabs",    (void *)&engine_chatWindowAppendToAllTabs },         // CALLED row -- public [SwgCuiChatWindow.h:172] MISMATCH: ours appendToAllTabs(const Unicode::String&) -> MI __fastcall thunk. ABI-UNSAFE TO CALL from the consumer (const Unicode::String& param -- the rev-2 sysmsg crash class); consumer-blocked on advertised (rev-2 §3, latent). utf8 shim (engine_chatWriteToAllTabsUtf8) on request.
	{ "cuiChatWindow::writeToCurrentTab", (void *)&engine_chatWindowAppendTextToCurrentTab },  // CALLED row -- public [SwgCuiChatWindow.h:174] MISMATCH: ours appendTextToCurrentTab(const Unicode::String&) -> MI __fastcall thunk. ABI-UNSAFE TO CALL (same rev-2 crash class as writeToAllTabs above); utf8 shim on request.
	{ "cuiChatWindow::chatEnterHandler", 0 },  // REAL ENTRY (detoured by Utinni hkChatEnter; delta==0 verified) -- 38-05. PUBLIC non-virtual CLEAN-ENTRY performEnterKey() [SwgCuiChatWindow.h:214], MISMATCH contract chatEnterHandler. Was a call-through MI thunk -> silently dead for a detour. Issue #11 mid-function NOP remains a SEPARATE SWGEmu-only joint decision (no offset arithmetic in the contract).
	{ "cuiChatWindow::createNewWindow", 0 },          // 24-4d: the requested cuiChatWindow::ctor REAL ENTRY is INFEASIBLE -- you cannot take &Class::Class in C++ (no ctor PMF -> pmfRealEntry has no input; a placement-new thunk is detour-dead). Instead advertise the SOLE construction funnel: PRIVATE static SwgCuiChatWindow* createNewWindow(UIPage&,Game::SceneType,std::string const&) [SwgCuiChatWindow.h:258], the only path to `new SwgCuiChatWindow` [SwgCuiChatWindow.cpp:1549]. PRIVATE -> address taken via a friend accessor compiled in SwgCuiChatWindow.cpp (engine_chatWindow_forward.h), same mechanism as the GroundScene private-method real-entry accessors. Static fn -> plain real entry (no MI inflation). CONSUMER: detour this to track construction (same coverage as the ctor). See handback 2026-06-24.

	// -- cui::manager (clientUserInterface, all static; CuiManager.h) -----------
	{ "cuiManager::render",           (void *)&CuiManager::render },               // static void render() [CuiManager.h:88]
	{ "cuiManager::setSize",          (void *)&CuiManager::setSize },              // static void setSize(int,int) [CuiManager.h:129]
	{ "cuiManager::togglePointer",    (void *)&CuiManager::setPointerToggledOn },  // static void setPointerToggledOn(bool) [CuiManager.h:107] (MISMATCH name)
	{ "cuiManager::restartMusic",     (void *)&CuiManager::restartMusic },         // static void restartMusic(bool) [CuiManager.h:97]
	{ "cuiManager::g_instance",       (void *)&CuiManager::getIoWin },             // ACCESSOR: CuiManager is all-static (no instance); getIoWin() returns the CuiIoWin singleton [CuiManager.h:100]

	// -- cui::io (clientUserInterface; CuiIoWin.h) ------------------------------
	// NOTE: cui::io::processEvent (CuiIoWin.h:62) and ::draw (:61) are VIRTUAL overrides of
	// IoWin -- SKIP: Utinni resolves off the live vtable (spec sec 6). Not advertised as &fn
	// (a &CuiIoWin::processEvent would be a vtable-dispatch thunk, not the impl).
	{ "cuiIo::setKeyboardInputActive", 0 }, // member PMF __thiscall, single-inheritance [CuiIoWin.h:66]
	{ "cuiIo::requestKeyboard", 0 },     // member PMF __thiscall [CuiIoWin.h:102]
	{ "cuiIo::g_instance",            (void *)&CuiManager::getIoWin },             // ACCESSOR: the CuiIoWin singleton accessor (CuiManager owns ms_theIoWin) [CuiManager.h:100]

	// -- consoleHelper (clientUserInterface; CuiConsoleHelper.h) ----------------
	{ "consoleHelper::sendInput",     (void *)&engine_consoleHelperSendInput },     // WR-05 fix: __fastcall(pThis,edx,istr)==__thiscall thunk -> processInput(istr, getRecurseStackForCommandBeingParsed()) [CuiConsoleHelper.h:76]; a raw &processInput would fault on the missing recursion-stack arg2 (spec "sendInput" -> SwgCuiConsoleHelper::sendInput has no from-source twin; engine-layer processInput is the equivalent). ABI-UNSAFE TO CALL from the consumer (const Unicode::String& istr param -- the rev-2 sysmsg crash class); flagged in the rev-2 audit, utf8 shim on request.

	// -- commandParser (sharedCommandParser; CommandParser.h) -------------------
	{ "commandParser::addSubCommand", 0 },  // bit_cast member PMF, __thiscall [CommandParser.h:149]

	// ======================================================================
	// 37-03 FULL CATALOG. Per-row symbol kind in the comment. Every & resolved
	// against the cited header this wave. Symbols the plan named but that do
	// NOT exist / are virtual / inline are OMITTED (see the OMIT block below).
	// ======================================================================

	// -- extent (sharedCollision; BaseExtent.h) §8 #2 --------------------------
	// The NON-virtual BaseExtent::intersect(begin,end) const overload via PMF +
	// explicit overload static_cast. Single-inheritance base -> PMF not inflated.
	// Collapses the UtinniCore retail(0x0126AF70)/SWGEmu(0x0125FA10) RVA split.
	{ "extent::intersect", 0 }, // OVERLOADED non-virtual [BaseExtent.h:47]

	// -- object (sharedObject; Object.h) -- NON-VIRTUAL only -------------------
	// VIRTUAL skips (NOT advertised; Utinni resolves off the live vtable, spec §6):
	//   Object::addToWorld / removeFromWorld [Object.h:120-121], setParentCell [:165].
	// move_o is INLINE [Object.h:1216] -> OMITTED (no ODR-emitted address, Pitfall 2).
	{ "object::getObjectType", 0 },         // Tag getObjectType() const NON-virtual [Object.h:152]
	{ "object::getObjectTemplate", 0 },     // const ObjectTemplate* getObjectTemplate() const [Object.h:150]
	{ "object::getObjectTemplateName", 0 }, // const char* getObjectTemplateName() const [Object.h:151]
	{ "object::getNetworkId", 0 },          // const NetworkId& getNetworkId() const [Object.h:162]
	{ "object::getParentCell", 0 },         // CellProperty* getParentCell() const NON-virtual [Object.h:166]
	{ "object::getTransform_o2w", 0 },      // Transform const& getTransform_o2w() const DLLEXPORT [Object.h:243]
	{ "object::setTransform_o2w", 0 },      // void setTransform_o2w(const Transform&) [Object.h:248]
	{ "object::getPosition_w", 0 },         // const Vector getPosition_w() const [Object.h:245]
	{ "object::setPosition_w", 0 },         // void setPosition_w(const Vector&) [Object.h:247]
	{ "object::getAppearance", 0 }, // OVERLOADED (const/non-const) [Object.h:170-171]
	{ "object::setAppearance", 0 },         // void setAppearance(Appearance*) [Object.h:174]
	{ "object::setScale", 0 },              // v34 NEW (consumer D-09 gap): void setScale(const Vector&) NON-virtual non-inline [Object.h:228 / Object.cpp:2205]. Vector is a 3-float POD by const& -- same boundary shape as setPosition_w/move_p
	{ "object::move_p", 0 },                // void move_p(const Vector&) NON-virtual non-inline [Object.h:251]

	// -- objectTemplate (sharedObject base static + sharedGame SharedObjectTemplate) --
	{ "objectTemplate::createObject", (void *)static_cast<Object * (*)(const char *)>(&ObjectTemplate::createObject) }, // OVERLOADED: static createObject(const char*) [ObjectTemplate.h:32] vs virtual createObject() const [:50]
	{ "objectTemplate::getAppearanceFilename", 0 },   // const std::string& (bool=false) const NON-virtual [SharedObjectTemplate.h:353]. ABI-UNSAFE RETURN for the consumer (returns our v145 std::string by const& -- rev-2 audit; consumer must not read its fields); const char* shim on request.
	{ "objectTemplate::getPortalLayoutFilename", 0 }, // [SharedObjectTemplate.h:354] -- same ABI-UNSAFE RETURN as getAppearanceFilename above
	{ "objectTemplate::getClientDataFile", 0 },       // [SharedObjectTemplate.h:355] -- same ABI-UNSAFE RETURN as getAppearanceFilename above

	// -- worldSnapshot (clientGame; WorldSnapshot.h) -- ALL STATIC in this tree -
	{ "worldSnapshot::load",          (void *)&WorldSnapshot::load },              // static void load(char const*) [WorldSnapshot.h:44]
	{ "worldSnapshot::addObject",     (void *)&WorldSnapshot::addObject },         // static Object* addObject(int64,int64,CrcString const&,Transform const&,float,uint32,int,std::string const&="") [WorldSnapshot.h:33]. ABI-UNSAFE TO CALL from the consumer (CrcString const& + std::string const& params -- rev-2 audit; the consumer cannot construct either with our layout); POD shim on request.
	{ "worldSnapshot::removeObject",  (void *)&WorldSnapshot::removeObject },      // static void removeObject(int64) [WorldSnapshot.h:49]
	{ "worldSnapshot::moveObject",    (void *)&WorldSnapshot::moveObject },        // static void moveObject(int64,Transform const&) [WorldSnapshot.h:48]
	{ "worldSnapshot::getLoadingPercent",   (void *)&WorldSnapshot::getLoadingPercent },   // static int getLoadingPercent() [WorldSnapshot.h:52]
	{ "worldSnapshot::detailLevelChanged",  (void *)&WorldSnapshot::detailLevelChanged },  // static void detailLevelChanged() [WorldSnapshot.h:56]

	// -- camera (clientGraphics; Camera.h) -- NON-VIRTUAL non-inline setters ----
	// (getViewport*/getViewportWidth etc. are INLINE [Camera.h:210-258] -> OMITTED.)
	{ "camera::setViewport", 0 }, // OVERLOADED [Camera.h:172]
	{ "camera::setNearPlane", 0 },          // void setNearPlane(real) [Camera.h:174]
	{ "camera::setFarPlane", 0 },           // void setFarPlane(real) [Camera.h:175]
	{ "camera::setHorizontalFieldOfView", 0 }, // void(real) [Camera.h:178]
	{ "camera::reverseProjectInViewportSpace", 0 }, // OVERLOADED non-inline [Camera.h:160]

	// -- misc statics (memory/audio/file/report) -------------------------------
	{ "memory::allocate",             (void *)&MemoryManager::allocate },          // static void* allocate(size_t,uint32,bool,bool) DLLEXPORT [MemoryManager.h:58]
	{ "memory::free",                 (void *)&MemoryManager::free },              // static void free(void*,bool) DLLEXPORT [MemoryManager.h:59] (spec "deallocate" -> ours free)
	{ "audio::setMasterVolume",       (void *)&Audio::setMasterVolume },           // static void setMasterVolume(float) [Audio.h:165]
	{ "audio::getMasterVolume",       (void *)&Audio::getMasterVolume },           // static float getMasterVolume() [Audio.h:166]
	{ "treeFile::open",               (void *)&TreeFile::open },                   // static AbstractFile* open(const char*,PriorityType,bool) DLLEXPORT [TreeFile.h:85]
	{ "treeFile::searchTree",         (void *)&TreeFile::addSearchTree },          // 24-4c NEW: the real search-PATH REGISTRATION fn (resolves the open/searchTree collision). static void addSearchTree(const char* fileName, int priority) [TreeFile.h:78] -- registers a .tre at a priority. CONSUMER ABI NOTE: TreeFile is ALL-STATIC -> this is __cdecl with NO pThis (NOT __thiscall), and arg order is REVERSED vs SWGEmu (ours: fileName,priority -- SWGEmu: priority,treeFilename). The loose-dir sibling addSearchPath(path,priority) [TreeFile.h:76] is available on request.
	{ "treeFile::enumerateFiles",     (void *)&TreeFile::enumerateFiles },         // ENUM-A NEW: flat file enumeration for the editor pickers (Repository). static void enumerateFiles(void(__cdecl* cb)(const char* fileName, void* ctx), void* ctx) [TreeFile.h] -- invokes cb once per filename across all registered SearchTree/SearchTOC TOC name tables (real strings, e.g. "terrain/tatooine.trn"). __cdecl, no pThis. NOTE: cb runs UNDER ms_criticalSection -> must not re-enter TreeFile (consumer append is safe).
	{ "report::print",                (void *)&Report::puts },                     // static void puts(const char*) [Report.h:41] (spec "Report::print" -> ours puts; printf is variadic)

	// -- commandParser ctors (free-function __thiscall thunks; NEVER &Class::Class) --
	{ "commandParser::ctor1",         (void *)&engine_commandParserCtor1 },        // __fastcall(pThis,edx,..)==__thiscall thunk -> CommandParser(const char*,size_t,const char*,const char*,CommandParser*) [CommandParser.h:130] (matches UtinniCore pCtor1)
	{ "commandParser::ctor2",         (void *)&engine_commandParserCtor2 },        // __fastcall(pThis,edx,..)==__thiscall thunk -> CommandParser(const CmdInfo&,CommandParser*) [CommandParser.h:128] (matches UtinniCore pCtor2)

	// ======================================================================
	// VIRTUAL SKIPS (37-03) -- advertised as NOTHING. &fn on a virtual yields a
	// vtable-dispatch thunk, not the impl; Utinni resolves these off the live
	// vtable (spec §6). They are intentionally NOT in the .inc required set
	// (a skipped virtual is vtable-resolved, NOT a "missing" required row).
	//   SKIP: virtual -- Object::addToWorld          [Object.h:120]
	//   SKIP: virtual -- Object::removeFromWorld      [Object.h:121]
	//   SKIP: virtual -- Object::setParentCell        [Object.h:165]
	//   SKIP: virtual -- Appearance::render           (Appearance.h, pure/virtual)
	//   SKIP: virtual -- Appearance::collide          (Appearance.h, virtual)
	//   SKIP: virtual -- GroundScene::draw            (Scene/IoWin override; deferred from 37-02)
	//   SKIP: virtual -- RenderWorld::render          (RenderWorld.h, static-or-virtual; vtable path)
	//   SKIP: virtual -- CuiIoWin::processEvent       [CuiIoWin.h:62] (from 37-02)
	//   SKIP: virtual -- BaseExtent::realIntersect    [BaseExtent.h:69] protected-virtual (the §8 #2 non-virtual BaseExtent::intersect IS advertised above)
	// (getObjectType/move_p/getParentCell are NON-virtual -> advertised in Task 1, NOT skipped.)

	// -- globals (§8 #3): advertised via accessor (call-not-read) where the data
	//    is private/file-static; raw &g only where genuinely accessible. ---------
	// The MVP already advertises the canonical accessors (game::g_runningFlags ->
	// Game::isOver; graphics RT W/H -> getCurrentRenderTarget*; cui singletons ->
	// CuiManager::getIoWin). The remaining spec globals (player health/stats, hud
	// view-distance, terrain singleton/weather/filename, static-shader) are
	// private members with NO non-inline ODR-emitted accessor confirmable this
	// wave -> OMITTED (see OMIT/DEFER block + 37-03-SUMMARY). A genuinely
	// accessible engine singleton accessor advertised raw as &g:
	{ "graphics::g_frameNumber",     (void *)&Graphics::getFrameNumber },          // ACCESSOR (§8 #3): the frame counter behind a genuinely-accessible non-private static getter [Graphics.h:83] -- call-not-read, &g-style global row

	// ======================================================================
	// BUCKET-4 REMAINING ADDRESSABLE FULL-SET -- 38-03 confirm-or-OMIT ledger
	// (D-04: a wrong & is worse than a missing row; never add on spec faith).
	// Every spec §6 "remaining ~30 subsystems" candidate NOT already advertised
	// above was source-confirmed in THIS tree this wave -> NET NEW ROWS THIS
	// PASS: ZERO. The plain-&fn bulk was already taken in 37-03; the remainder
	// is virtual / inline / protected / MI-ctor / private-no-accessor / NONEXISTENT.
	// Each candidate is accounted for (none silently dropped):
	//   OMIT  inline    -- Object::move_o                  [Object.h:1216 `inline void Object::move_o`]
	//   SKIP  virtual   -- Object::addToWorld/removeFromWorld [Object.h:120-121] / setParentCell [:168 `virtual void setParentCell`]
	//   OMIT  protected -- CommandParser::createDelegateCommands [CommandParser.h:180 (protected: block opens :178)] (would need an in-TU forwarder; Utinni's TJT path uses the already-advertised ctors/addSubCommand)
	//   OMIT  inline    -- Camera::getViewportX0/Y0/Width/Height [Camera.h:210/226/242/.. `inline int Camera::getViewport*`] (no ODR address). The two out-of-line getViewport(int&,...)/(float&,...) overloads [Camera.h:170-171] ARE addressable, but the spec lists the camera getters generically under the inline-OMIT bucket with no distinct contract slot -> OMIT (adding a speculative overloaded camera::getViewport row would be a spec-faith add; flagged for EPA-08 if Utinni names a concrete slot).
	//   SKIP  virtual   -- Appearance::render/collide, RenderWorld::render, clientWorld::collide overloads, proceduralTerrain::* (vtable-resolved; already in the VIRTUAL SKIPS block / 37-03)
	//   DEFER MI ctor   -- hud/loginScreen/gameMenu/radialMenu + ~20 low-level UI control ctors (each needs its own placement-new __fastcall thunk + injector-supplied args; Utinni resolves via RVA today -- same rationale as the cuiChatWindow::ctor DEFER above)
	//   OMIT  private    -- read-only globals (player health/stats, hud view-distance, terrain singleton/weather/filename, static-shader): private members with NO non-inline ODR accessor confirmable this wave (§8 #3: never take a private-member address)
	//   NONE  (absent)   -- memory::deallocateString / math::vectorNormalize / network idManager* / crcString::calculateCrc / client::writeCrashLog / client::setupStartDataInstall: 0 source twin in this tree (MemoryManager has free() not deallocate; the rest grep to 0). NONEXISTENT -> OMIT, flagged for EPA-08.
	// (No new #include / no new vcxproj include dir needed this pass -- zero adds.)
	// ======================================================================
	// 24-§2.B BUCKET B (Effects editor live preview) -- render/appearance group +
	// the cooperative particle retrigger. v6 -> v7 (5 NAME ADDs). Source-confirmed
	// this wave via parallel subsystem reads. Requested-but-NOT-advertisable rows are
	// OMITTED/SKIPPED in the ledger below (a wrong & is worse than a missing row; a
	// skipped virtual is consumer-vtable-resolved, NOT missing -- handoff §2.C).
	// ----------------------------------------------------------------------
	// -- skeletalAppearance (clientSkeletalAnimation; SkeletalAppearance2.h) --
	{ "skeletalAppearance::getDisplayLodSkeleton", 0 }, // bit_cast PMF (non-virtual, single-inheritance SkeletalAppearance2 -- name +2); overload-disambiguated (const) in dyn[] below
	// -- renderWorld (clientGraphics, all static; RenderWorld.h) --
	{ "renderWorld::addObjectNotifications", (void *)&RenderWorld::addObjectNotifications }, // static void addObjectNotifications(Object&) [RenderWorld.h:62] -- constant &fn
	// -- bloom (clientGame post-processing, all static; Bloom.h) --
	{ "bloom::preSceneRender",  (void *)&Bloom::preSceneRender },  // static void preSceneRender()  [Bloom.h:25] -- constant &fn
	{ "bloom::postSceneRender", (void *)&Bloom::postSceneRender }, // static void postSceneRender() [Bloom.h:26] -- constant &fn
	// -- particlePreview (Utinni cooperative retrigger; ClientEffectManager.cpp friend) --
	{ "particlePreview::retrigger", (void *)&engine_retriggerClientEffect }, // 24-§2.B-ii: friend free fn over the PRIVATE m_particleSystems -> enumerate + ParticleEffectAppearance::restart() matching live particle instances (+ a balanced AppearanceTemplateList::fetch refresh). __cdecl(const char*) -> constant &fn (NOT a dyn[] row). Game-thread, once-per-save, allocation-free.
	// -- particlePreview B-2 (Utinni cooperative re-PLAY; ClientEffectManager.cpp free fn, public APIs only) --
	{ "particlePreview::replayClientEffect", (void *)&engine_replayClientEffect }, // B-2 (v7->v8): re-plays a .cef FRESH on Game::getPlayer() via the public ClientEffectManager::playClientEffect (transient muzzle/hit/explosion case -- restart() above only covers sustained live instances). A not-currently-cached .cef is a guaranteed cache-MISS -> ClientEffectTemplateList::fetch reloads it from disk + re-fetches referenced .prt/appearance/sound templates -> edit visible. __cdecl(const char*)->bool -> constant &fn (NOT a dyn[] row; not a friend -- public APIs only). Game-thread, once-per-preview, allocation-free. false (no crash) if no player/scene.

	// ======================================================================
	// BUCKET A (2026-06-28) -- per-editor real-entry detour rows (ledger §2.A). v8 -> v9.
	// 6 ADVERTISED of 14 requested; the other 8 are OMIT (nonexistent / un-addressable
	// ctor) or SKIP (virtual -> consumer vtable-resolves) in the Bucket A ledger below.
	// Source-mapped this wave via parallel subsystem reads. A wrong & is worse than a
	// missing row -- nonexistent/virtual rows are NOT guessed.
	// ----------------------------------------------------------------------
	// -- constant &fn rows (static methods / free fn; no dyn[] needed) --
	{ "cuiRadialMenuManager::update", (void *)&CuiRadialMenuManager::update }, // static void update() [CuiRadialMenuManager.h:46] -- all-static facade. Radial-menu editing.
	{ "cuiMenu::infoTypesFindDefaultCursor", (void *)&Cui::MenuInfoTypes::findDefaultCursor }, // FREE FN UICursor* const findDefaultCursor(ClientObject&) in namespace Cui::MenuInfoTypes [CuiMenuInfoTypes.h:199 / .cpp:404] -- NOT a CuiMenu member (no such class); constant &fn (resolves the ledger's confirm-or-OMIT). Menu cursor behavior.
	// -- systemMessageManager SEND (v15): the INJECT half of the sysmsg pair (2026-07-02 request, rev-2 2026-07-03) --
	{ "systemMessageManager::sendMessageUtf8", (void *)&engine_sendFakeSystemMessage }, // rev-2 REPLACE (v14->v15): extern "C" void __cdecl(const char* utf8Msg, bool chatBoxOnly) shim -> widens on OUR side -> sendFakeSystemMessage [CuiSystemMessageManager.h:38]. The v14 direct-&fn row sendMessage is REMOVED (name-REPLACE, not re-point, so a version-skewed pairing misses by name and degrades instead of mis-calling across a changed ABI): it passed const Unicode::String& across the boundary and CRASHED live smoke -- consumer WString models the 2002 3-pointer layout, ours is v145 SSO (see the shim comment). CALLED endpoint, game-thread-only. SEND half ONLY -- RECEIVE stays OMIT (A-2.1 note below).
	// -- game lookAt-target READ (v15->v16, 2026-07-09 request) --
	{ "game::getPlayerLookAtTargetId", (void *)&engine_getPlayerLookAtTargetId }, // v16 NAME ADD: extern "C" __int64 __cdecl(void) shim DEFINED IN CreatureObject.cpp (exe TU cannot include CreatureObject.h -- the setTarget accessor precedent; declared in engine_creatureObject_forward.h). Returns the PLAYER's lookAt/selection-target NetworkId VALUE (full 64 bits; 0 = no player/no target) -- the READ twin of creatureObject::setTarget (same m_lookAtTarget slot; NOT getIntendedTarget). Shim per the rev-2 ABI RULE: getLookAtTarget() is INLINE [CreatureObject.h:882] + returns const CachedNetworkId& (embeds a Watcher the consumer does not model) -> primitive in EDX:EAX instead. CALLED, game-thread-only, on-demand (target-change callback + panel refresh, not per-frame). Consumer resolves the id via the v12 network::getObjectById row; a null resolve (unloaded/out-of-range) is a normal staleness outcome. constant &fn.
	// (systemMessageManager::receiveMessage REVERTED v10->v11 -- see the A-2.1 OMIT note below; it CRASHED world-load. SEND (above) is unaffected -- it is an advertisable static behind a POD-only shim.)
	// -- worldSnapshot editor READ wave (v16->v17, Goal B Wave 1; rev-3 freeze 2026-07-15) --
	// Id-keyed read/browse of the CURRENT scene's live snapshot (the Utinni snapshot editor,
	// the last SWGEmu-only editor). All 7 are extern "C" __cdecl shims DEFINED in clientGame
	// WorldSnapshot.cpp (ms_reader + its bookkeeping are file-scope there; declared in
	// engine_worldSnapshot_forward.h) -> constant &fn rows, image-valid at load (no dyn[]).
	// FROZEN contracts (rev-3 §1): enumeration live + AUTHORED-ONLY (tombstones AND
	// buildout-provenance rows never enumerate; id-keyed reads answer miss for them); node
	// reads force-finish the CONSULT-60 incremental parse; wsGetGeneration is a PURE counter
	// read (no parse force) bumping on load/unload ONLY; wsGetNodeInfo fills the FROZEN
	// 80-byte EngineWsNodeInfo (engine_hookpoints.h) under the size-first protocol.
	// CALLED endpoints, game-thread-only, primitives/pointers-only boundary (ABI RULE).
	{ "worldSnapshot::wsForgetNode",          (void *)&engine_wsForgetNode },           // v32 NEW: int (__int64 id) -- drop a node from the snapshot WITHOUT despawning the live Object. 1 forgotten / 0 not found. wsRemoveNode is TEARDOWN (subtree removeFromWorld + delete), which made the object the modder just placed VANISH at Persist; this is the missing half -- the DATA leaves the .ws (removeObject: sphere handle dropped, removeNode tombstones so every later saveFiltered skips it) and the OBJECT stays put. No occupancy guard needed (nothing is deleted). Forgetting does NOT free the id: wsAllocateIdRange tests NetworkIdManager, and the Object is still registered
	{ "clientInteriorLayoutManager::refreshInteriorLayout", (void *)&engine_refreshInteriorLayout }, // v32 NEW: int (__int64 buildingId) -- re-apply a changed .ilf to ONE building, NO scene reload. 1 ok / 0 no such object|not a POB|not a building template / -1 layout reload failed. Deletes ONLY the client-only interior-layout objects (NOT every client-cached object in the cells -- that would sweep the consumer's unpersisted wsAddObject placements), reloads the TEMPLATE's cached InteriorLayoutReaderWriter (the layout is cached on ClientBuildingObjectTemplate, so a latch/cursor reset alone would rebuild the PRE-EDIT .ilf) with TreeFile::forgetMissingFile first, then re-arms each cell so the budgeted update() re-creates under maxInteriorCreatesPerFrame
	{ "cellProperty::getCellName",            (void *)&engine_getCellName },            // v32 NEW: int (void* cellProperty, char* buf, int cap) -- COPY-OUT of CellProperty::getCellName [inline, CellProperty.h:249 -> shim]. Returns needed length INCL NUL (wsGetSavePath convention); 0 = null input/no name. Copy-out not pointer-return because m_cellName points into the TEMPLATE (CellProperty.cpp:456), whose lifetime the consumer cannot see. World cell returns "world", not null (CellProperty.cpp:225). The .ilf row stores the cell name as a literal string, so the CRC twin is not a substitute
	{ "worldSnapshot::wsIsParsePending",      (void *)&engine_wsIsParsePending },      // v28 NEW: int (void) -- 1 = phased parse in flight (world still rebuilding), 0 = idle/complete. PURE, NON-forcing: the ONLY ws* row without a finishLoadNow() prologue, so a consumer can WAIT rather than call a forcing row for its side effect (which pays the whole remaining ~3.1s synchronous parse). NOT getLoadingPercent -- that returns 0 while parsing and then reports preload percent, so 0 is ambiguous
	{ "worldSnapshot::wsGetNodeCount",        (void *)&engine_wsGetNodeCount },        // int (void) -- top-level authored non-tombstone count; 0 = empty/no snapshot
	{ "worldSnapshot::wsGetTopNodeIdAt",      (void *)&engine_wsGetTopNodeIdAt },      // __int64 (int index) -- id of the index-th enumerable top-level node; 0 = out-of-range
	{ "worldSnapshot::wsGetChildCount",       (void *)&engine_wsGetChildCount },       // int (__int64 id) -- enumerable direct-child count; 0 = miss/tombstone/leaf
	{ "worldSnapshot::wsGetChildIdAt",        (void *)&engine_wsGetChildIdAt },        // __int64 (__int64 id, int index) -- id of the index-th enumerable child; 0 = miss/out-of-range
	{ "worldSnapshot::wsGetNodeInfo",         (void *)&engine_wsGetNodeInfo },         // int (__int64 id, EngineWsNodeInfo* out) -- POD-out, size-first; 1 ok, 0 miss/tombstone
	{ "worldSnapshot::wsGetNodeTemplateName", (void *)&engine_wsGetNodeTemplateName }, // int (__int64 id, char* buf, int cap) -- copy-out; returns needed length INCLUDING the NUL; 0 = miss
	{ "worldSnapshot::wsGetGeneration",       (void *)&engine_wsGetGeneration },       // int (void) -- bumps on load/unload ONLY (consumer cache/undo invalidation)
	// -- worldSnapshot editor MUTATION wave (v17->v18, Goal B Wave 2; frozen 2026-07-18) --
	// LIVE-ONLY mutation, explicitly non-persistent (persistence = Wave 3). Shims in
	// WorldSnapshot.cpp (same TU/discipline as Wave 1); constant &fn rows. Semantics per the
	// accepted ANSWERS 5.2/5.3/5.5 + Wave-2 deltas -- see the .inc block + shim comments.
	// CALLED endpoints, game-thread-only, primitives/pointers-only boundary (ABI RULE).
	{ "worldSnapshot::wsAddObject",           (void *)&engine_wsAddObject },           // __int64 (const char* tmpl, const float* transform12, __int64 containedById) -- pre-validates ALL before minting; id..id+cellCount + atomic POB cells; FULL streamed-create bookkeeping; spawns immediately; 0 = fail-closed nothing mutated
	{ "worldSnapshot::wsAddNodeAt",           (void *)&engine_wsAddNodeAt },           // int (__int64 explicitId, __int64 containedById, const char* tmpl, int cellIndex, const float* transform12, float radius, unsigned int pobCrc) -- undo-replay data re-add at the EXPLICIT id; top-level = sphere handle + diff-sentinel dirty; child under live parent = immediate spawn; 1 ok, 0 fail-closed
	{ "worldSnapshot::wsRemoveNode",          (void *)&engine_wsRemoveNode },          // int (__int64 id) -- 7-step subtree teardown, OCCUPANCY-GUARDED (isClientCachedOnly recursive; Container dtor cascade-deletes contents): 1 removed / 0 miss / -1 occupied
	{ "worldSnapshot::wsSetNodeRadius",       (void *)&engine_wsSetNodeRadius },       // int (__int64 id, float radius) -- 1 ok, 0 miss/tombstone; re-seats the sphere-tree extent (the moveObject pattern)
	{ "worldSnapshot::wsConfigureIdAllocator",(void *)&engine_wsConfigureIdAllocator },// int (__int64 floor, __int64 ceiling) -- one-time optional allocator band (0 = keep default per param; default ceiling 0x1000000 = consumer server-id convention); 1 accepted, 0 rejected VISIBLY
	// -- worldSnapshot editor PERSISTENCE wave (v18->v19, Goal B Wave 3; frozen 2026-07-18) --
	// The disk half. Shims in WorldSnapshot.cpp; semantics per ANSWERS 5.1(a-d). Save is
	// authored-only + tombstone-skip (recursive), absolute destination in the winning loose
	// SearchPath, negative-cache invalidation, post-write shadow verification. Typed result
	// enum published in the Wave-3 handback (0 ok / 1 no-snapshot / 2 no-loose-search-path /
	// 3 destination-shadowed / 4 id-int32-overflow / 5 buildout-set-integrity / 6 write-failure).
	{ "worldSnapshot::wsSaveSnapshot",        (void *)&engine_wsSaveSnapshot },        // int (void) -- save the CURRENT scene's authored .ws; typed result per the enum above
	{ "worldSnapshot::wsGetSavePath",         (void *)&engine_wsGetSavePath },         // int (char* buf, int cap) -- resolved save ROOT copy-out; needed length INCLUDING NUL; 0 = no loose SearchPath (save would fail closed too)
	{ "worldSnapshot::wsUnloadSnapshot",      (void *)&engine_wsUnloadSnapshot },      // void (void) -- unload + reset the sticky ms_sceneName (else advertised load(currentScene) early-outs and reload returns EMPTY); bumps the generation
	// -- Wave-3 rider 4B: world-pick/target filter (2026-07-18). The NGE gate the consumer's
	// SWGEmu RVA patch (0x00BD3FA3) was aiming at: CuiPreferences::allowTargetAnything -- read at
	// SwgCuiHud.cpp:198/365 (world-pick) AND CuiRadialMenuManager.cpp:977/2714 (radial on
	// non-ClientObjects). PUBLIC out-of-line statics, primitives-only -- plain constant &fn, the
	// config::setModalChat precedent. Contract name mirrors the ENGINE name (allowTargetAnything),
	// not the consumer's patchAllowTargetEverything working title.
	{ "cuiPreferences::setAllowTargetAnything", (void *)&CuiPreferences::setAllowTargetAnything }, // static void (bool) [CuiPreferences.h:149 / .cpp:1051]
	{ "cuiPreferences::getAllowTargetAnything", (void *)&CuiPreferences::getAllowTargetAnything }, // static bool (void) [CuiPreferences.h:148 / .cpp:1733]
	// -- Wave-3 rider 4C: gizmo camera matrices (2026-07-18). Copy-out accessors defined above
	// in this TU; replaces the consumer's raw struct-offset camera reads (NGE layout != SWGEmu ->
	// garbage matrix -> execute-of-heap crash, cdb-confirmed their side).
	{ "camera::getProjectionMatrix",          (void *)&engine_getCameraProjectionMatrix }, // int (float* out16) -- GlMatrix4x4 verbatim (row-major 4x4); 1 ok / 0 no camera
	{ "camera::getTransformO2W",              (void *)&engine_getCameraTransformO2W },     // int (float* out12) -- row-major 3x4, position column 3 (the EngineWsNodeInfo convention); 1 ok / 0 no camera
	// -- Live World Editor ray-pick + pre-approved radial clear (v19->v20, 2026-07-19 change request) --
	{ "clientWorld::collideScreenRay",        (void *)&engine_collideScreenRay },          // int (int screenX, int screenY, int objectsOnly, __int64* outHitObjectId, float* outPoint3) -- copy-out cursor ray-cast (shim above); 1 hit / 0 miss; terrain hit = id 0 + valid point; objectsOnly=1 drops terrain/terrainFlora/interiorGeometry
	{ "cuiRadialMenuManager::clear",          (void *)&CuiRadialMenuManager::clear },      // static void () [CuiRadialMenuManager.h:47] -- pre-approved rider (2026-07-18 positionchanged ANSWER); plain &fn, the update-row sibling
	// -- current-scene-id copy-out (v20->v21, 2026-07-19 change request #2: one-click reload) --
	{ "game::getSceneId",                     (void *)&engine_getSceneId },                // int (char* buf, int cap) -- copy-out (shim above); needed length INCLUDING NUL; 0 = no scene loaded. Game::getSceneId() is inline + const std::string& -> shim mandatory (ABI RULE)
	// -- borrowed-Object* pick (v21->v22, 2026-07-19 change request #3: pure-.ilf decoration selection) --
	{ "clientWorld::collideScreenRayObject",  (void *)&engine_collideScreenRayObject },    // void* (int screenX, int screenY, int objectsOnly) -- RAW nearest-hit Object* (no ancestor walk, no id resolution; null = miss/no camera); BORROWED, game-thread-only, cleared by consumer on cell/zone change; pair with collideScreenRay for layer triage
	// -- in-place .ws template re-point (v22->v23, 2026-07-19 change request #4: model-D lossless per-instance rebind) --
	{ "worldSnapshot::wsSetNodeTemplateName", (void *)&engine_wsSetNodeTemplateName },     // int (__int64 id, const char* name) -- re-point an authored node's template NAME in place (OTNL intern; subtree/id/transform/crc untouched; data-only, reload spawns from the new template); 1 ok / 0 miss / -1 refused (empty name, buildout, or template unresolvable NOW -- forgetMissingFile+exists pre-check); caller follows with wsSaveSnapshot
	// -- object o2p copy-out (v23->v24, 2026-07-19 change request #5: the .ilf persist read) --
	{ "object::getTransformO2P",              (void *)&engine_getObjectTransformO2P },     // int (void* object, float* out12) -- row-major 3x4, position column 3 (the camera::getTransformO2W / .ilf convention); 1 ok / 0 null; borrowed consumer-held Object*, game-thread-only
	// -- containing-building id copy-out (v24->v25, 2026-07-30 change request #6: the model-D Arm step) --
	{ "object::getContainingBuildingId",      (void *)&engine_getContainingBuildingId },   // __int64 (void* object) -- NetworkId value of the containing POB building (== the .ws node id); works for an .ilf decoration, a wall-click CELL object, or the player; 0 = null / not inside a POB; borrowed consumer-held Object*, game-thread-only
	{ "object::setParentCell",                (void *)&engine_setParentCell },             // v27 NEW: int (void* object, void* cellProperty) -- cell reparent; VIRTUAL [Object.h:168] so the shim is mandatory. 1 ok / 0 refused (null either side). Null cell would FATAL (NOT_NULL Object.cpp:1389) -- pass cellProperty::getWorldCellProperty to reparent OUT, never null. Write o2w FIRST then reparent (cellChanged fires inside setParentCell); do NOT convert to o2p -- attachToObject_w preserves world. Borrowed pointers, game-thread-only
	{ "cellProperty::getWorldCellProperty",   (void *)&CellProperty::getWorldCellProperty },// v27 NEW: CellProperty* (void) -- the world-cell sentinel, out-of-line [CellProperty.h:78 / CellProperty.cpp:308] so a plain constant &fn row, NO shim (the cuiPreferences::getAllowTargetAnything pattern). Required to express "reparent to the exterior" -- setParentCell cannot take null
	{ "cellProperty::setPortalTransitionsEnabled", (void *)&CellProperty::setPortalTransitionsEnabled }, // v28 NEW: void (bool) -- suppress the portal transition sweep across a teleport write. Public out-of-line static [CellProperty.h:73 / CellProperty.cpp:336] so a plain &fn row, NO shim. THIS is the row that makes the o2w-vs-o2p / ordering analysis moot: use the engine's OWN idiom, GroundScene.cpp:1492-1497 -- setParentCell(C); setPortalTransitionsEnabled(false); setTransform_o2p(...); setPortalTransitionsEnabled(true); CollisionWorld::objectWarped(player). ALWAYS re-enable (it is global state, not scoped)
	{ "collisionWorld::objectWarped",         (void *)&CollisionWorld::objectWarped },     // v28 NEW: void (Object*) -- reconcile collision after a discontinuous move; out-of-line static [CollisionWorld.h:82 / .cpp:1334], plain &fn row. Without it CollisionWorld::update reconciles against a stale last-position/cell pair after a tool-driven teleport. Completes the GroundScene.cpp:1497 idiom above
	{ "clientWorld::findCellAtWorldPosition", (void *)&engine_findCellAtWorldPosition },   // v28 NEW: void* (float x, float y, float z) -- CellProperty* containing world point P; THE placement-routing primitive (a coordinate-only destination has no object to pick). Wraps ClientWorld::findClosestCellObjectFromWorldPosition [ClientWorld.cpp:1649] + the getCellProperty hop; the client's own containment heuristic, so tool and engine agree about a doorway. NEVER null -- falls back to the world cell, so it is always a legal setParentCell argument
	{ "object::getAttachedTo",                (void *)&engine_getAttachedTo },             // v28: void* (void* object) -- parent object or 0. ⚠ NOT the mount guard: cell parentage and mount attachment SHARE m_attachedToObject (setParentCell -> attachToObject_w, Object.cpp:1404-1405), so a player merely standing in a POB reports non-null. The v28 claim "non-null = do not reparent" was FALSIFIED live (refused every teleport from indoors). Use object::isChildObject instead; this row remains for reading the actual parent
	{ "playerCreatureController::warpClient", (void *)&engine_warpPlayer },              // v30 NEW: int (float x, float y, float z) WORLD coords -- CLIENT-INITIATED teleport through the controller. warpClient [PlayerCreatureController.h:120, engine's own DebugPortalCamera.cpp:314 caller] stamps m_previousTransform_p, mints a CLIENT sequence number, and sends CM_netUpdateTransform SEND|RELIABLE|DEST_AUTH_SERVER|DEST_AUTH_CLIENT -- so the SERVER is told and the local apply runs via handleNetUpdateTransform, bringing objectWarped + FreeChaseCamera retarget for free. A raw setTransform_o2w is UNSEQUENCED and gets overwritten by the next server update. Does NOT reparent: findCellAtWorldPosition -> setParentCell FIRST. 1 ok / 0 no player / -1 no controller
	{ "object::isChildObject",                (void *)&engine_isChildObject },             // v29 NEW: int (void* object) -- 1 = genuine child/mount attachment, 0 = not a child (incl. cell parentage) or null. THE mount discriminator: m_childObject is set only from the asChildObject arg (Object.cpp:1931) and cells attach with FALSE, so it is exactly what the compiled-out DEBUG_FATAL(isChildObject()) at Object.cpp:1396 tests. Inline -> shim. Guard setParentCell with this to avoid the silent Release pose corruption on a mounted player
	// -- real-entry / PMF rows (completed in ensureDynamicRowsFilled() -- {name,0} placeholders) --
	{ "creatureObject::setTarget", 0 },         // MISMATCH name: no CreatureObject::setTarget exists; the "current target" setter is setLookAtTarget(const NetworkId&) [CreatureObject.h:311] (m_lookAtTarget = "this creature's current target"). CreatureObject is MI (TangibleObject : ClientObject, CallbackReceiver) -> pmfRealEntry (own method, delta==0). dyn[] below. MAINTAINER: verify consumer typedef vs setLookAtTarget; alts setIntendedTarget/setLookAtAndIntendedTarget.
	{ "messageQueue::appendMessage", 0 },       // non-virtual overloaded [MessageQueue.h:51], flat class -> pmfToVoid; 3-arg (int,float,uint32) overload. dyn[] below. INPUT-path diag.
	{ "messageQueue::appendMessageData", 0 },   // MISMATCH name: no appendMessageData exists; maps to the DATA overload appendMessage(int,float,Data*,uint32) [MessageQueue.h:52], flat class -> pmfToVoid. dyn[] below. INPUT-path diag.

	// -- Bucket A-2 (2026-06-28): world-pick / HUD-target (closes the §2.A getTarget gap) --
	{ "cuiHud::getTarget",  (void *)&engine_hudGetLastSelectedObject },           // MISMATCH name + REAL ENTRY of SwgCuiHud::getLastSelectedObject() const [SwgCuiHud.h:95] (m_lastSelectedObject = world-picked object). SwgCuiHud is MI -> __fastcall call-through thunk (CALLED row -- consumer READS the pick on the live hud). Supersedes the Bucket A OMIT. World-object pick.
	{ "cuiHud::g_instance", (void *)&SwgCuiHudFactory::findMediatorForCurrentHud }, // ACCESSOR: static SwgCuiHud* findMediatorForCurrentHud() [SwgCuiHudFactory.h:24] -- resolves the LIVE ground/space hud (concrete HudGround/HudSpace, both : SwgCuiHud). The instance the consumer calls cuiHud::getTarget on (mirrors cuiIo::g_instance -> CuiManager::getIoWin). constant &fn.

	// -- Bucket A-3 (2026-06-28): network id->Object resolver (unblocks the creatureObject::setTarget callback) --
	{ "network::getObjectById", (void *)&NetworkIdManager::getObjectById }, // static Object* NetworkIdManager::getObjectById(const NetworkId&) [NetworkIdManager.h:22 / .cpp:72] -- the real NetworkId->Object* lookup (SWGEmu idManagerGetObjectById 0x00B380E0). Consumer typedef Object*(__cdecl*)(const NetworkId&) -> constant &fn (true static; no instance accessor needed -- the singleton ms_instance is internal). Without this the consumer's hkSetTarget resolved id->Object* via a stale hardcoded RVA and crashed (same class as the A-2.1 wrong-&). Group is "network" to match the consumer's Network::getObjectById wrapper.

	// ======================================================================
	// FREE-CAM editor-unlock wave (2026-06-29) -- v12 -> v13. 6 NAME ADDs, all CALLED accessors
	// that encapsulate fragile NGE byte-offsets (consumer will not hardcode offsets that drift).
	// Consumer "free camera" == OUR DebugPortalCamera (cm_Free 5 == CI_debugPortal). NONE detoured.
	// ----------------------------------------------------------------------
	// -- constant &thunk / &fn rows (no dyn[] needed) --
	{ "groundScene::isFreeCameraActive",               (void *)&engine_groundSceneIsFreeCameraActive },               // __fastcall thunk: getCurrentView()==CI_debugPortal (==5==consumer cm_Free) [GroundScene.h:84-95,207]. Replaces the currentView field read.
	{ "groundScene::getDebugPortalCameraMessageQueue", (void *)&engine_groundSceneGetDebugPortalCameraMessageQueue },  // friend forwarder (GroundScene.cpp): m_debugPortalCameraInputMap->getMessageQueue() [GroundScene.h:111] -- the MQ the consumer read at the hardcoded InputMap+0xC. __fastcall == __thiscall.
	{ "gameCamera::getMessageQueue",                   (void *)&engine_gameCameraGetMessageQueue },                   // __fastcall thunk: getController()->getMessageQueue() [Object.h:190 / Controller.h:67] -- the camera's movement MQ (was GameCamera+0x248). ALIASES the debugPortal MQ while free-cam active (init wires camera->setMessageQueue(inputMap->getMessageQueue()), GroundScene.cpp:803).
	{ "object::isActive",                              (void *)&engine_objectIsActive },                             // external-linkage __fastcall shim: Object::isActive() is NON-virtual but INLINE [Object.h:158/1328] -> no PMF address. CALLED. Consumer typedef bool(__thiscall*)(const Object*).
	// -- read-side messageQueue PMF rows (completed in ensureDynamicRowsFilled() -- {name,0} placeholders) --
	{ "messageQueue::getCount",   0 },          // MISMATCH name: no MessageQueue::getCount exists; the real entry is int getNumberOfMessages() const [MessageQueue.h:42]. Flat single-inheritance class -> pmfToVoid (4-byte PMF == real entry); non-overloaded. dyn[] below. (appendMessage is already advertised; this is the read-COUNT side.)
	{ "messageQueue::getMessage", 0 },          // the 4-arg READ overload getMessage(int index,int* message,float* value,uint32* flags=0) const [MessageQueue.h:43] -- EXACTLY the consumer's requested (i,outType,outValue,outFlags) signature (they pass nullptr for flags). Overloaded with the Data** variant [:44] -> static_cast to disambiguate. Flat class -> pmfToVoid. dyn[] below. (The consumer's stale io_win.h declares a 3-arg wrapper; their paired wave aligns it to this 4-arg real entry.)

	// ----------------------------------------------------------------------
	// BUCKET A OMIT/SKIP ledger -- the §2.A rows NOT advertised (each accounted for;
	// none silently dropped; consumer alternatives -> HANDBACK):
	//   OMIT ctor     -- cuiChatWindow::ctor: cannot take &Class::Class; the SOLE construction funnel (createNewWindow, sole `new SwgCuiChatWindow` @ SwgCuiChatWindow.cpp:1549; createInto routes through it) is ALREADY advertised (cuiChatWindow::createNewWindow, friend accessor). Detour that to cover construction.
	//   OMIT ctor     -- cuiLoginScreen::ctor [SwgCuiLoginScreen.h:34]: un-addressable ctor AND no construction funnel -- 0 `new SwgCuiLoginScreen`; built only via the generic CuiMediatorFactory::Constructor<T> template (`new T(page)` CuiMediatorFactory_Constructor.h:49). Consumer resolves via RVA (already DEFER'd, see the 24 ctor note). A login-specific placement-new __fastcall thunk + injector-supplied UIPage& is the only source hook -- not justified now.
	//   OMIT absent   -- cuiManager::findObjectUnderCursor: NO such member on the all-static CuiManager [CuiManager.h:26] (retail collapses this + cuiHud::getTarget onto one world-pick RVA with no 1:1 named twin here). The world-pick value is delivered by the Bucket A-2 cuiHud::getTarget + cuiHud::g_instance rows above (SwgCuiHud::getLastSelectedObject on the live hud); no separate findObjectUnderCursor symbol exists, so this name stays OMIT.
	//   ADVERTISED A-2 -- cuiHud::getTarget: was OMIT in Bucket A (no SwgCuiHud::getTarget); NOW advertised as a __fastcall thunk over SwgCuiHud::getLastSelectedObject() const [SwgCuiHud.h:95] + the cuiHud::g_instance accessor (rows above). Closes the §2.A world-pick gap.
	//   OMIT wrong-& -- systemMessageManager::receiveMessage: advertised in Bucket A (v9) as the static CuiSystemMessageManager::receiveSystemMessage(const ChatSystemMessage&), REVERTED v10->v11 (A-2.1) -- it CRASHED world-load (c0000005). The consumer's hkReceiveMessage is written for the MessageDispatch::Receiver::receiveMessage(const Emitter&, const MessageBase&) byte-stream pattern (it does Archive::AutoByteStream deserialization), NOT a 1-arg static UI handler. The real receiver here is the FILE-LOCAL anon-namespace Listener::receiveMessage [CuiSystemMessageManager.cpp:51] (deserializes ChatSystemMessage, forwards to the static) -> NO external symbol -> un-advertisable. Pointing the 2-arg network-receiver detour at the 1-arg static misread args -> garbage exec -> AV in sendFakeSystemMessage->receiveSystemMessage->hkReceiveMessage on region-enter (ClientRegionManager::checkCurrentRegion). A wrong & is worse than a missing row. CONSUMER alt: vtable-resolve the ChatSystemMessage Listener, or CALL the advertisable static sendFakeSystemMessage to INJECT (offered on request; not the observe-hook the consumer asked for).
	//   SKIP virtual  -- cuiHud::actionPerformAction -> SwgCuiHudAction::performAction [SwgCuiHudAction.h:24 `virtual bool performAction(...) const`] -> consumer vtable-resolves; &fn would be a vtable stub.
	//   SKIP virtual  -- cuiHud::update -> SwgCuiHud::update [SwgCuiHud.h:63 `virtual void update(float)`, overrides CuiMediator::update CuiMediator.h:186] -> consumer vtable-resolves.
	//   SKIP virtual  -- cuiLoginScreen::activate -> the login-specific work is SwgCuiLoginScreen::performActivate [SwgCuiLoginScreen.h:42 `virtual void performActivate()`] -> consumer vtable-resolves. The non-virtual CuiMediator::activate [CuiMediator.h:100] is generic (all mediators), not login-specific -- not bound.
	//   SKIP virtual  -- debugCamera::alter -> FreeCamera::alter [FreeCamera.h:61] / GameCamera::alter [GameCamera.h:40] / Object::alter [Object.h:135], virtual at every level; no non-virtual real-entry helper (per-frame body is in the virtual FreeCamera::alter FreeCamera.cpp:254). Consumer vtable-resolves off the live FreeCamera instance. Free-cam target class = FreeCamera.
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// 24-§2.B OMIT/SKIP ledger -- handoff §2.B-i rows that are NOT advertisable in this
	// tree (each accounted for; none silently dropped; offered alternatives -> HANDBACK):
	//   SKIP virtual  -- particleEffectAppearance::render [ParticleEffectAppearance.h:77 `virtual void render() const`] -> consumer vtable-resolves (handoff §2.C); &fn would be a vtable-dispatch stub, not the impl.
	//   SKIP virtual  -- skeletalAppearance::render [SkeletalAppearance2.h:120 `virtual void render() const`] -> consumer vtable-resolves.
	//   OMIT ctor     -- particleEffectAppearance::ctor [ParticleEffectAppearance.h:61]: cannot take &Class::Class and there is NO single construction funnel (only inline `new ParticleEffectAppearance(tmpl)`); a placement-new thunk is detour-dead (same disposition as cuiChatWindow::ctor). The §2.B-ii retrigger supplies the editor's live-preview value instead.
	//   OMIT absent   -- skeletalAppearance::addShaderPrimitives: NOT a member of SkeletalAppearance2 (the name is Skeleton::addShaderPrimitives(const SkeletalAppearance2&) / CompositeMesh -- different class+signature). Never guessed; FLAGGED for the handback.
	//   OMIT absent   -- renderWorld::render: NO such method on the all-static RenderWorld. Nearest = RenderWorld::drawScene(const RenderWorldCamera&) [RenderWorld.h:97]; offered in the handback, not bound on spec faith.
	//   OMIT wildcard -- shaderPrimitiveSorter::* : the handoff names no concrete method. Canonical submission funnel = ShaderPrimitiveSorter::add(const ShaderPrimitive&) [ShaderPrimitiveSorter.h:82] (static, overloaded). Offered in the handback for the consumer to name a concrete slot; not bound speculatively.
	//   COORDINATE    -- render globals (static-shader/transform/scale/extent SWGEmu RVAs): consumer drives the draw via the already-advertised graphics::* statics (handoff's PREFERRED shape). No raw private-global rows added (§8 #3: never take a private-member address).
	// ----------------------------------------------------------------------
	// PINNED: NO null-pair sentinel -- count is sizeof/sizeof, the static_assert has NO -1.
};

static const EngineHookPoints s_table =
{
	ENGINE_HOOKPOINTS_VERSION,
	(unsigned int)(sizeof s_engineHookPoints / sizeof s_engineHookPoints[0]),   // NO -1 (no sentinel)
	s_engineHookPoints
};

// ----------------------------------------------------------------------
// Coverage self-check (EPA-04 seed -- 37-02/37-03 inherit it). THREE parts.
// The count static_assert is only a cheap drift SMOKE; the name-set-equality
// runtime check is the actual zero-missing gate.
// ----------------------------------------------------------------------

// (a) Compile-time count smoke: expand the .inc to a +1 count and assert the
// table row count equals the .inc required-set count. NO -1 (no sentinel).
// Mirrors the Direct3d11_ConstantBuffer.h static_assert table-validation idiom.
enum
{
	ENGINE_REQUIRED_COUNT = 0
#define ENGINE_HOOKPOINT(g, n) + 1
#include "engine_hookpoints.inc"
#undef ENGINE_HOOKPOINT
};
static_assert((sizeof s_engineHookPoints / sizeof s_engineHookPoints[0]) == ENGINE_REQUIRED_COUNT,
              "hookpoint table row count != .inc required-set count (drift)");

// (b) Name-set source-of-truth: the machine-generated required-name set,
// emitted from the SAME .inc via the X-macro -- no hand-typed name strings.
static const char * const s_requiredNames[] =
{
#define ENGINE_HOOKPOINT(g, n) #g "::" #n,
#include "engine_hookpoints.inc"
#undef ENGINE_HOOKPOINT
};

// (c) Runtime self-check. Never crashes (graceful degradation, EPA-04):
// returns false / logs on ANY of: a null addr, a duplicate name, OR a name-set
// mismatch vs s_requiredNames[] (every required name present exactly once and
// no extras -- the zero-missing check the count static_assert cannot provide).
static bool engine_strEq(const char * a, const char * b)
{
	if (a == b)
		return true;
	if (!a || !b)
		return false;
	while (*a && (*a == *b))
	{
		++a;
		++b;
	}
	return *a == *b;
}

// ----------------------------------------------------------------------
// Static-init race fix (2026-06-25). GetEngineHookPoints() is read by the injected
// consumer (Utinni) from a REMOTE thread while this exe's main thread is still
// SUSPENDED -- BEFORE the CRT runs _initterm (our static initializers). Rows whose addr
// is a compile-time constant ((void*)&Symbol / static_cast<fn>) are image-valid at
// module load; rows whose addr is a runtime CALL (pmfToVoid / pmfRealEntry / the
// real-entry accessors) are dynamically initialized at static-init and are NULL until
// the main thread resumes -- and MSVC defers the WHOLE array tail from the first such
// row, so even the constant rows after it read null early. The consumer saw a half-built
// table (the 40/96 it reported -- the prefix up to the first call-row).
//
// FIX: the 29 call-rows are now { name, 0 } constant placeholders, so the ENTIRE array is
// image-valid at load (no dynamic init, no tail deferral). We complete them HERE, on
// first read, on the READER's thread. Guarded by a PLAIN static bool (constant-init at
// load -> NO MSVC magic-static guard; that guard would EnterCriticalSection on the CRT's
// not-yet-initialized thread-safe-static lock on the pre-resume remote thread). Every
// producer is pure address arithmetic (bit_cast / PMF-byte memcpy / &function) -- no
// heap, no TLS, no CRT -- so it is safe pre-_initterm. Idempotent (same addresses) so a
// benign concurrent double-fill is harmless. Patched by NAME, not index, so it cannot
// silently drift if the table is reordered. (Option A of the 2026-06-25 request -- the
// only race-proof option: constexpr can't extract an MI-PMF entry, early ctors still race.)
// ----------------------------------------------------------------------
static void ensureDynamicRowsFilled()
{
	static bool s_filled = false;   // constant-init at load; plain flag, no magic-static guard
	if (s_filled)
		return;

	struct DynRow { const char * name; void * addr; };
	const DynRow dyn[] =
	{
		{ "groundScene::update",                     engine_groundSceneUpdateRealEntry() },
		{ "groundScene::handleInputMapEvent",        engine_groundSceneHandleInputMapEventRealEntry() },
		{ "cuiChatWindow::enableTextInput",          pmfRealEntry(&SwgCuiChatWindow::acceptTextInput) },
		{ "cuiChatWindow::chatEnterHandler",         pmfRealEntry(&SwgCuiChatWindow::performEnterKey) },
		{ "cuiChatWindow::createNewWindow",          engine_chatWindowCreateNewWindowEntry() },
		{ "cuiIo::setKeyboardInputActive",           pmfToVoid(&CuiIoWin::setKeyboardInputActive) },
		{ "cuiIo::requestKeyboard",                  pmfToVoid(&CuiIoWin::requestKeyboard) },
		{ "commandParser::addSubCommand",            pmfToVoid(&CommandParser::addSubCommand) },
		{ "extent::intersect",                       pmfToVoid(static_cast<bool (BaseExtent::*)(Vector const &, Vector const &) const>(&BaseExtent::intersect)) },
		{ "object::getObjectType",                   pmfToVoid(&Object::getObjectType) },
		{ "object::getObjectTemplate",               pmfToVoid(&Object::getObjectTemplate) },
		{ "object::getObjectTemplateName",           pmfToVoid(&Object::getObjectTemplateName) },
		{ "object::getNetworkId",                    pmfToVoid(&Object::getNetworkId) },
		{ "object::getParentCell",                   pmfToVoid(&Object::getParentCell) },
		{ "object::getTransform_o2w",                pmfToVoid(&Object::getTransform_o2w) },
		{ "object::setTransform_o2w",                pmfToVoid(&Object::setTransform_o2w) },
		{ "object::getPosition_w",                   pmfToVoid(&Object::getPosition_w) },
		{ "object::setPosition_w",                   pmfToVoid(&Object::setPosition_w) },
		{ "object::getAppearance",                   pmfToVoid(static_cast<Appearance * (Object::*)()>(&Object::getAppearance)) },
		{ "object::setAppearance",                   pmfToVoid(&Object::setAppearance) },
		{ "object::setScale",                        pmfToVoid(&Object::setScale) },   // v34 (D-09)
		{ "object::move_p",                          pmfToVoid(&Object::move_p) },
		{ "objectTemplate::getAppearanceFilename",   pmfToVoid(&SharedObjectTemplate::getAppearanceFilename) },
		{ "objectTemplate::getPortalLayoutFilename", pmfToVoid(&SharedObjectTemplate::getPortalLayoutFilename) },
		{ "objectTemplate::getClientDataFile",       pmfToVoid(&SharedObjectTemplate::getClientDataFile) },
		{ "camera::setViewport",                     pmfToVoid(static_cast<void (Camera::*)(int, int, int, int)>(&Camera::setViewport)) },
		{ "camera::setNearPlane",                    pmfToVoid(&Camera::setNearPlane) },
		{ "camera::setFarPlane",                     pmfToVoid(&Camera::setFarPlane) },
		{ "camera::setHorizontalFieldOfView",        pmfToVoid(&Camera::setHorizontalFieldOfView) },
		{ "camera::reverseProjectInViewportSpace",   pmfToVoid(static_cast<const Vector (Camera::*)(int, int) const>(&Camera::reverseProjectInViewportSpace)) },
		// 24-§2.B Bucket B: non-virtual single-inheritance member PMF (4-byte; SkeletalAppearance2
		// is single-inheritance so pmfToVoid's sizeof guard passes). OVERLOADED [SkeletalAppearance2.h
		// :137 const / :138 non-const] -> disambiguate to the const "LOD read" overload.
		{ "skeletalAppearance::getDisplayLodSkeleton", pmfToVoid(static_cast<const Skeleton * (SkeletalAppearance2::*)() const>(&SkeletalAppearance2::getDisplayLodSkeleton)) },
		// Bucket A (2026-06-28) per-editor real-entry / PMF rows:
		// creatureObject::setTarget -> setLookAtTarget (MISMATCH name). CreatureObject is MI (TangibleObject : ClientObject,
		// CallbackReceiver) -> inflated PMF -> real entry via the CreatureObject.cpp accessor (delta==0 gated inside). The exe
		// TU cannot include CreatureObject.h (sharedSkillSystem not on its path), so the address provider lives in the class's TU.
		{ "creatureObject::setTarget",         engine_creatureSetTargetRealEntry() },
		// messageQueue::appendMessage[Data] -- MessageQueue is a flat single-inheritance class -> pmfToVoid (4-byte PMF == real entry); overloaded -> static_cast to disambiguate.
		{ "messageQueue::appendMessage",       pmfToVoid(static_cast<void (MessageQueue::*)(int, float, uint32)>(&MessageQueue::appendMessage)) },
		{ "messageQueue::appendMessageData",   pmfToVoid(static_cast<void (MessageQueue::*)(int, float, MessageQueue::Data *, uint32)>(&MessageQueue::appendMessage)) },
		// FREE-CAM wave (v13) read-side MessageQueue rows (flat single-inheritance class -> pmfToVoid):
		{ "messageQueue::getCount",            pmfToVoid(&MessageQueue::getNumberOfMessages) },                                                            // MISMATCH name -> getNumberOfMessages [MessageQueue.h:42]; non-overloaded const member.
		{ "messageQueue::getMessage",          pmfToVoid(static_cast<void (MessageQueue::*)(int, int *, float *, uint32 *) const>(&MessageQueue::getMessage)) }, // 4-arg overload [MessageQueue.h:43]; static_cast disambiguates from the Data** overload [:44].
	};

	const unsigned int dynCount = (unsigned int)(sizeof dyn / sizeof dyn[0]);
	const unsigned int rowCount = (unsigned int)(sizeof s_engineHookPoints / sizeof s_engineHookPoints[0]);
	for (unsigned int d = 0; d < dynCount; ++d)
		for (unsigned int i = 0; i < rowCount; ++i)
			if (engine_strEq(s_engineHookPoints[i].name, dyn[d].name))
			{
				s_engineHookPoints[i].addr = dyn[d].addr;
				break;
			}

	s_filled = true;
}

bool engine_verifyNoNullNoDup()
{
	ensureDynamicRowsFilled();   // complete the call-rows before checking (verify runs at static-init, after the consumer's early read)

	bool ok = true;
	const unsigned int count = s_table.count;

	// (a) no null addr
	for (unsigned int i = 0; i < count; ++i)
	{
		if (s_engineHookPoints[i].addr == 0)
		{
			REPORT_LOG(true, ("engine_verifyNoNullNoDup: NULL addr for '%s'\n",
				s_engineHookPoints[i].name ? s_engineHookPoints[i].name : "(null)"));
			ok = false;
		}
	}

	// (b) no duplicate name
	for (unsigned int i = 0; i < count; ++i)
	{
		for (unsigned int j = i + 1; j < count; ++j)
		{
			if (engine_strEq(s_engineHookPoints[i].name, s_engineHookPoints[j].name))
			{
				REPORT_LOG(true, ("engine_verifyNoNullNoDup: DUPLICATE name '%s'\n",
					s_engineHookPoints[i].name ? s_engineHookPoints[i].name : "(null)"));
				ok = false;
			}
		}
	}

	// (c) name-set equality vs the X-macro-generated required set:
	//     every required name appears in the table exactly once...
	const unsigned int requiredCount = (unsigned int)(sizeof s_requiredNames / sizeof s_requiredNames[0]);
	for (unsigned int r = 0; r < requiredCount; ++r)
	{
		unsigned int hits = 0;
		for (unsigned int i = 0; i < count; ++i)
		{
			if (engine_strEq(s_requiredNames[r], s_engineHookPoints[i].name))
				++hits;
		}
		if (hits != 1)
		{
			REPORT_LOG(true, ("engine_verifyNoNullNoDup: required name '%s' present %u times (expected 1)\n",
				s_requiredNames[r], hits));
			ok = false;
		}
	}

	//     ...and the table adds no extras outside the required set.
	for (unsigned int i = 0; i < count; ++i)
	{
		bool found = false;
		for (unsigned int r = 0; r < requiredCount; ++r)
		{
			if (engine_strEq(s_engineHookPoints[i].name, s_requiredNames[r]))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			REPORT_LOG(true, ("engine_verifyNoNullNoDup: table name '%s' not in required set\n",
				s_engineHookPoints[i].name ? s_engineHookPoints[i].name : "(null)"));
			ok = false;
		}
	}

	REPORT_LOG(true, ("engine_verifyNoNullNoDup: %s (%u rows, %u required)\n",
		ok ? "PASS" : "FAIL", count, requiredCount));
	return ok;
}

// ----------------------------------------------------------------------
// The export. dllexport ALONE on an extern "C" __cdecl function forces the
// undecorated public name -- NO .def, NO /EXPORT pragma, NO ModuleDefinitionFile
// (proven by the shipped gl11 GetHookPoints twin, dumpbin-confirmed). Returns a
// pointer to the process-lifetime static; Utinni only reads it.
// ----------------------------------------------------------------------
extern "C" __declspec(dllexport) const EngineHookPoints * __cdecl GetEngineHookPoints();

const EngineHookPoints * GetEngineHookPoints()
{
	ensureDynamicRowsFilled();   // complete the 29 call-rows on THIS (the reader's) thread, before returning the table -- the static-init race fix (2026-06-25)
	return &s_table;
}
