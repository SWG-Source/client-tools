// ======================================================================
//
// Direct3d11_Metrics.h
// copyright (c) 2026 Galaxies Reborn
//
// Always-on counters. Not #ifdef _DEBUG, and that is the entire point.
//
// The DX9 backend's equivalent (Direct3d9_Metrics.h) is wrapped in #ifdef _DEBUG
// from its first line to its last, its only report caller sits inside a #if 0 in
// the game loop, every NP_PROFILER_ macro compiles to nothing at PRODUCTION == 1,
// and there is not one CreateQuery call in the first-party tree. The practical
// consequence is that "no performance degradation" has never been a testable
// claim about this renderer, in either direction.
//
// So these counters exist in every configuration including PRODUCTION. They are
// plain integers incremented on paths that already touch memory; the cost is
// not measurable next to the D3D calls they accompany.
//
// The counters the port's gates are actually written against are the ones that
// must read ZERO after warm-up. A nonzero state-object, input-layout, constant
// buffer or shader creation count means something is being built inside the frame
// loop -- the single most common way a DX11 port ends up slower than the DX9 code
// it replaced. Same for a blocking staging map or a texture-bake readback.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_Metrics_H
#define INCLUDED_Direct3d11_Metrics_H

// ======================================================================

class Direct3d11_Metrics
{
public:

	static void install();
	static void remove();

	// Called at the top of each frame. Rolls the per-frame counters into the
	// per-run totals and zeroes them.
	static void beginFrame();

	// Report a frame that took far longer than a frame should, together with everything this
	// backend created or blocked on during it. Three things can stutter inside a building -- asset
	// loading, the server, or the renderer -- and a slow frame that created nothing and compiled
	// nothing rules the renderer out, which is worth more than another guess.
	static void reportHitches();

	// Bracket the part of the frame that is inside this backend. Not a ScopedTimer because the
	// interval spans two separate calls from the engine rather than a C++ scope.
	static void markSceneBegin();
	static void markSceneEnd();

	// Adds the elapsed time of the scope it is declared in to the counter it is given, in raw
	// QueryPerformanceCounter ticks. Nested instances aimed at the same counter would
	// double-count, so each counter has one scope.
	//
	// Ticks rather than microseconds because these counters are accumulated per draw. Converting
	// in the destructor meant an integer division per call, and every call below a microsecond --
	// which is every prepareToDraw and every Draw submit -- lost its remainder to truncation. At
	// a thousand draws a frame that discards most of the total and reports 0.0 ms for work that
	// is really there. One conversion at report time has no such error.
	class ScopedTimer
	{
	public:
		explicit ScopedTimer(long long &ticks);
		~ScopedTimer();
	private:
		ScopedTimer(ScopedTimer const &);
		ScopedTimer &operator =(ScopedTimer const &);
		long long &m_ticks;
		long long  m_start;
	};

	// Ticks to milliseconds, for the report. Public because the hitch line is the only consumer
	// and it lives in this class.
	static double ticksToMilliseconds(long long ticks);

	// Write the current frame's counters and the run totals to the log.
	static void report();

	// Zero-invariant check. Returns the number of things that were created
	// during the frame that just ended, which is required to be zero once the
	// caches are warm.
	static int  getInFrameCreationCount();

public:

	// Draw submission.
	static int  drawCalls;
	static int  drawIndexedCalls;
	static int  vertices;
	static int  triangles;
	static int  droppedDraws;              // gate: must be 0

	// Draws lost to a vertex buffer that does not carry an input its vertex shader reads.
	// Counted apart from droppedDraws because shipping DX9 does not draw these either: its
	// substitution of the bad-vertex-shader material is compiled out of every configuration
	// that links a client, leaving a DrawPrimitive that fails. Matching that is parity, so this
	// is a property of the assets and not a gate.
	static int  unsatisfiableInputLayouts;

	// Binds, counted as calls and as misses, so a cache's hit rate is a
	// measurement rather than an assumption. DX9's own vertex declaration cache
	// is a permanent 100% miss because forceVertexDeclaration never assigns the
	// shadow it compares against, which nobody noticed for twenty years for want
	// of exactly this pair of numbers.
	static int  inputLayoutBindCalls;
	static int  inputLayoutBindMisses;
	static int  vertexBufferBindCalls;
	static int  vertexBufferBindMisses;
	static int  indexBufferBindCalls;
	static int  indexBufferBindMisses;
	static int  vertexShaderBindCalls;
	static int  vertexShaderBindMisses;
	static int  pixelShaderBindCalls;
	static int  pixelShaderBindMisses;
	static int  blendStateBindCalls;
	static int  blendStateBindMisses;
	static int  depthStencilStateBindCalls;
	static int  depthStencilStateBindMisses;
	static int  rasterizerStateBindCalls;
	static int  rasterizerStateBindMisses;
	static int  samplerBindCalls;
	static int  samplerBindMisses;
	static int  shaderResourceBindCalls;
	static int  shaderResourceBindMisses;
	static int  viewportSetCalls;
	static int  scissorSetCalls;

	// Transform setters. Counted because the concatenation is deferred: a count far
	// above the draw count means the engine is resetting matrices it is not drawing
	// with, and a count of zero means nothing will transform at all.
	static int  setTransformCalls;

	// Constant traffic. Bytes as well as calls, because the failure this guards
	// against is uploading a whole register file per draw rather than the part
	// that changed.
	static int  constantBufferUpdates;
	static int  constantBufferBytes;

	// Dynamic buffer rings. A DISCARD renames the buffer and costs allocation;
	// NO_OVERWRITE appends and does not. More than one DISCARD per ring per frame
	// means the ring is too small.
	static int  ringDiscards;
	static int  ringNoOverwrites;
	static int  ringBytes;

	// Render target changes and the readbacks that stall.
	static int  renderTargetSwitches;
	static int  textureBakeReadbacks;      // gate: must be 0
	static int  backBufferMaps;            // gate: must be 0 in a normal frame
	static int  blockingStagingMaps;       // gate: must be 0

	// Creations. All four of these are required to be zero inside a frame.
	static int  stateObjectCreations;
	static int  inputLayoutCreations;
	static int  constantBufferCreations;
	static int  shaderCompiles;

	// Microseconds spent inside D3DCompile. Accumulated per frame like the counts, so the peak
	// frame's figure is the size of the hitch a first-use compile burst actually causes.
	static int  shaderCompileMicroseconds;

	// Programs served from the precompiled cache, and programs it did not have. A miss is not a
	// fault -- it compiles, exactly as it did before the cache existed -- but a run that is all
	// misses is a cache that needs rebaking, and that is worth being able to see.
	// Shaders this backend wrote and compiles itself, at install, outside the asset chokepoint:
	// the scene target's composite pass. Counted apart from shaderCompiles so that the asset
	// figure means "programs the precompiled cache did not cover" and can legitimately read zero,
	// and still folded into the in-frame creation count so that the gate would catch these if they
	// ever moved into a frame.
	static int  backendShaderCompiles;

	// Resources asset streaming creates. Counted because the hitch report was blind to them and
	// therefore said "created nothing" about frames that were streaming a mesh or a texture in.
	static int  vertexBufferCreations;
	static int  indexBufferCreations;
	static int  textureCreations;

	// Microseconds spent getting streamed resources onto the GPU, split by what is doing the work.
	// A single lumped figure was enough to show that a 138 ms frame spent 27 ms of it here, and not
	// enough to act on: it could not distinguish a driver allocating 1438 buffers from this backend
	// memsetting and converting mip data on the way to UpdateSubresource, and those have opposite
	// fixes -- pooling for the first, removing a redundant copy for the second.
	static long long bufferCreateTicks;
	static long long textureCreateTicks;
	static long long textureUploadTicks;   // lock + unlock: staging, conversion, UpdateSubresource

	// Microseconds spent inside prepareToDraw, which every draw funnels through: input layout
	// lookup, light selection, transform concatenation and the constant flush. A 40 ms frame of
	// 2000 draws is either this or it is upstream of this backend, and measuring is the only way
	// to tell which.
	static long long drawPrepareTicks;

	// Microseconds inside Draw and DrawIndexed themselves. A 36 ms scene of 261 draws that spent
	// 0.1 ms in prepareToDraw, with the GPU idle at 0.8 ms and nothing created or streamed, means
	// one call inside the scene blocked -- and scene time on its own cannot say whether that was a
	// submit or one of the binds around it. This is what separates them: submits here, everything
	// else in the difference between this plus prepareToDraw and the scene total.
	static long long drawSubmitTicks;

	// The rest of the scene. Every bind this backend does goes through the static shader's apply,
	// and every dynamic buffer unlock does a Map/memcpy/Unmap on a ring. Neither was timed, which
	// left a 52 ms scene with nothing to attribute it to once prepareToDraw and the submits had
	// been ruled out.
	static long long shaderApplyTicks;
	static long long ringMapTicks;

	// Where a frame's wall time goes, which the counters above cannot answer. A 51 ms frame that
	// created nothing, streamed nothing and spent 0.0 ms in prepareToDraw has still spent 51 ms
	// somewhere, and until the frame is split there is no evidence for which side of the DLL
	// boundary it went to. sceneMicroseconds is beginScene to endScene; presentMicroseconds is
	// Present, which blocks on vsync and on a GPU that is behind; and the wall time left over
	// after subtracting both is engine work this backend never sees.
	static int  sceneMicroseconds;
	static int  presentMicroseconds;

	// The two gaps that make the accounting add up instead of leaving a residual. A frame is
	// beginScene..endScene, then whatever the engine does before it presents, then Present, then
	// whatever it does before the next beginScene -- and those last two are both engine work but
	// not the same engine work. A 45 ms frame that streamed nothing, created nothing and left the
	// GPU idle at 0.4 ms spent its time in one of these, and which one narrows the search from
	// "the client" to a specific half of the game loop.
	static int  beforePresentMicroseconds;   // endScene -> Present
	static int  afterPresentMicroseconds;    // Present -> next beginScene

	// Called by the present wrapper either side of Present. Bracketing it explicitly rather than
	// timing the wrapper's scope is what lets endScene..Present be measured too.
	static void markPresentBegin();
	static void markPresentDone();

	// What the engine handed this backend through setLights, which it calls once per batch. The
	// MAX over the frame, not the last value: a batch with no lights is ordinary (a UI pass, a
	// self-illuminated pass), so the last call says nothing about whether the scene is lit.
	static int  maxLightsInList;
	static int  lightBatches;
	static int  lightBatchesWithLights;

	// The scene's own ambient, over batches that have lights. Ambient is accumulated out of the
	// light list, so an unlit batch has zero ambient by construction and would only dilute this.
	// Kept after the 0.3 floor was removed because it is what says whether an interior's ambient is
	// plausible, which is the question the floor used to hide.
	static int  litBatches;
	static int  maxAmbientMilli;           // thousandths, so this stays an int like its neighbours

	static int  shaderCacheHits;
	static int  shaderCacheMisses;

	// Composite. Confirms the colour correction pass is genuinely skipped at
	// identity settings rather than running a curve that happens to look right.
	static int  compositesCopied;
	static int  compositesCorrected;

	static int  presents;
	static int  presentFailures;

private:

	Direct3d11_Metrics();
	Direct3d11_Metrics(Direct3d11_Metrics const &);
	Direct3d11_Metrics &operator =(Direct3d11_Metrics const &);
};

// ======================================================================

#endif
