// ======================================================================
//
// engine_worldSnapshot_forward.h -- exe-local declarations of the Goal B
// Wave-1 snapshot-editor READ shims (hookpoints v17; rev-3 freeze 2026-07-15).
//
// The shims are extern "C" __cdecl and DEFINED in clientGame
// WorldSnapshot.cpp -- the TU that owns the file-scope ms_reader singleton
// and its bookkeeping (the sysmsg/lookAtTarget shim pattern at TU scale).
// engine_advertise.cpp includes this header to take constant &fn addresses.
//
// EngineWsNodeInfo (the wsGetNodeInfo POD-out) is defined in the shared
// contract header engine_hookpoints.h; an incomplete declaration suffices
// here (the advertiser only takes the function address).
//
// ======================================================================

#ifndef INCLUDED_engine_worldSnapshot_forward_H
#define INCLUDED_engine_worldSnapshot_forward_H

struct EngineWsNodeInfo;

extern "C" int     __cdecl engine_wsForgetNode(__int64 networkIdInt);   // v32: drop a node from the snapshot WITHOUT despawning the live Object (1 forgotten / 0 not found). The half wsRemoveNode cannot give -- that one is teardown
extern "C" int     __cdecl engine_refreshInteriorLayout(__int64 buildingNetworkId);   // v32: re-apply a changed .ilf to ONE building, no scene reload. DEFINED in ClientInteriorLayoutManager.cpp (1 ok / 0 miss / -1 layout reload failed)
extern "C" int     __cdecl engine_wsIsParsePending(void);   // v28: PURE, NON-forcing parse-completion read (1 = rebuilding). The only ws* row with no finishLoadNow() prologue -- lets a consumer WAIT instead of FORCE a ~3.1s synchronous parse
extern "C" int     __cdecl engine_wsGetNodeCount(void);
extern "C" __int64 __cdecl engine_wsGetTopNodeIdAt(int index);
extern "C" int     __cdecl engine_wsGetChildCount(__int64 networkIdInt);
extern "C" __int64 __cdecl engine_wsGetChildIdAt(__int64 networkIdInt, int index);
extern "C" int     __cdecl engine_wsGetNodeInfo(__int64 networkIdInt, EngineWsNodeInfo* out);
extern "C" int     __cdecl engine_wsGetNodeTemplateName(__int64 networkIdInt, char* buf, int cap);
extern "C" int     __cdecl engine_wsGetGeneration(void);

// -- Goal B Wave 2 (v18; frozen 2026-07-18): LIVE-ONLY mutation --------------
extern "C" __int64 __cdecl engine_wsAddObject(const char* sharedTemplateFilename, const float* transform12, __int64 containedById);
extern "C" int     __cdecl engine_wsAddNodeAt(__int64 explicitId, __int64 containedById, const char* templateFilename, int cellIndex, const float* transform12, float radius, unsigned int portalLayoutCrc);
extern "C" int     __cdecl engine_wsRemoveNode(__int64 networkIdInt);
extern "C" int     __cdecl engine_wsSetNodeRadius(__int64 networkIdInt, float radius);
extern "C" int     __cdecl engine_wsConfigureIdAllocator(__int64 floorId, __int64 ceilingId);

// -- Goal B Wave 3 (v19; frozen 2026-07-18): PERSISTENCE ---------------------
extern "C" int     __cdecl engine_wsSaveSnapshot(void);
extern "C" int     __cdecl engine_wsGetSavePath(char* buf, int cap);
extern "C" void    __cdecl engine_wsUnloadSnapshot(void);
extern "C" int     __cdecl engine_wsSetNodeTemplateName(__int64 networkIdInt, const char* name);   // v23: in-place template re-point (model-D lossless rebind)

#endif // INCLUDED_engine_worldSnapshot_forward_H
