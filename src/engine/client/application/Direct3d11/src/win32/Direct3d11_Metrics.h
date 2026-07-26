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
