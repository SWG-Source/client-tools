// ======================================================================
//
// ConfigDirect3d11.h
// copyright (c) 2026 Galaxies Reborn
//
// Every [Direct3d11] setting, read once during install.
//
// Only settings something in this backend actually consumes are declared. Keys
// arrive alongside the commit that reads them, so there is never a documented
// knob that quietly does nothing.
//
// Nothing that can change what a frame looks like belongs here. Those are
// DebugFlags, which are developer levers, rather than configuration a player or
// a benchmark run can set without saying so.
//
// ======================================================================

#ifndef INCLUDED_ConfigDirect3d11_H
#define INCLUDED_ConfigDirect3d11_H

// ======================================================================

class ConfigDirect3d11
{
public:

	enum DriverType
	{
		DT_hardware,
		DT_warp,
		DT_reference
	};

public:

	static void        install();

	static int         getAdapter();
	static DriverType  getDriverType();
	static int         getFeatureLevelCap();
	static bool        getDebugLayer();

	static int         getDebugScreenshotFrame();
	static int         getDebugRenderDocFrame();

	// Fetch precompiled DXBC instead of compiling, when a validated cache is present. On by
	// default: a missing or stale cache falls back to compiling, so there is nothing to opt into.
	static bool        getUseCompiledShaders();

	// Compile everything and write the blobs out, for tools/dxbcbake. Never both at once -- a run
	// that used the cache would only bake what missed, which is how a partial cache gets mistaken
	// for a complete one.
	static bool        getBakeCompiledShaders();
	static bool        getAllowTearing();

	// Whether to emit the per-frame hitch and distribution lines. Off by default: they are
	// diagnostics and writing them is main-thread file I/O.
	static bool        getReportFrameTiming();

	// Diagnostic: suppress fog entirely, so its contribution to distant terrain can be seen by
	// its absence.
	static bool        getDebugDisableFog();

	// Multiplier on the engine's fog density, from fogDensityPercent. 1.0 is parity; lower
	// stretches the fog curve over a longer draw distance rather than removing haze.
	static float       getFogDensityScale();
	static int         getFullscreenRefreshRate();
	static int         getSwapChainBufferCount();
	static bool        getAntiAlias();
	static int         getAntiAliasSampleCount();
	static int         getShaderCapabilityOverride();
};

// ======================================================================

#endif
