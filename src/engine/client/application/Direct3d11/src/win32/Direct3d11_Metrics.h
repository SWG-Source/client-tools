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

	// Adds the elapsed time of the scope it is declared in to the counter it is given. Nested
	// instances aimed at the same counter would double-count, so each counter has one scope.
	class ScopedTimer
	{
	public:
		explicit ScopedTimer(int &microseconds);
		~ScopedTimer();
	private:
		ScopedTimer(ScopedTimer const &);
		ScopedTimer &operator =(ScopedTimer const &);
		int       &m_microseconds;
		long long  m_start;
	};

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

	// Microseconds spent inside ID3D11Device::Create* for streamed resources, and inside the mip
	// uploads that follow. Without this a hitch line says a frame created 184 textures but not
	// whether that is where its two and a half seconds went.
	static int  resourceCreationMicroseconds;

	// Microseconds spent inside prepareToDraw, which every draw funnels through: input layout
	// lookup, light selection, transform concatenation and the constant flush. A 40 ms frame of
	// 2000 draws is either this or it is upstream of this backend, and measuring is the only way
	// to tell which.
	static int  drawPrepareMicroseconds;

	// How many lights the engine handed this backend. Not reset per frame -- it is the last value
	// setLights was given, which is the state a frame was rendered under.
	static int  lightsInList;

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
