// ======================================================================
//
// ConfigDirect3d11.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "ConfigDirect3d11.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrashReportInformation.h"

// ======================================================================

namespace ConfigDirect3d11Namespace
{
	int                            ms_adapter;
	ConfigDirect3d11::DriverType   ms_driverType = ConfigDirect3d11::DT_hardware;
	int                            ms_featureLevelCap;
	bool                           ms_debugLayer;
	bool                           ms_useCompiledShaders;
	bool                           ms_bakeCompiledShaders;

	bool                           ms_allowTearing;
	int                            ms_fullscreenRefreshRate;
	int                            ms_swapChainBufferCount;

	bool                           ms_antiAlias;
	int                            ms_antiAliasSampleCount;

	int                            ms_shaderCapabilityOverride;
}
using namespace ConfigDirect3d11Namespace;

// ======================================================================

#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("Direct3d11", #a, b))
#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("Direct3d11", #a, b))

// ======================================================================

void ConfigDirect3d11::install()
{
	// -1 selects the adapter DXGI enumerates first.
	KEY_INT (adapter, -1);

	// 0 = hardware, 1 = WARP, 2 = reference. Software rasterisation is never
	// chosen silently: it costs an order of magnitude of frame time, and a
	// benchmark run that fell back to it without saying so would be recorded as
	// a performance regression in whatever changed most recently.
	int const driverType = ConfigFile::getKeyInt("Direct3d11", "driverType", 0);
	FATAL(driverType < 0 || driverType > 2, ("[Direct3d11] driverType %d is invalid [0..2: hardware, warp, reference]", driverType));
	ms_driverType = static_cast<DriverType>(driverType);
	WARNING(ms_driverType != DT_hardware, ("[Direct3d11] driverType %d selects a software rasteriser. Frame times from this run are not comparable to anything.", driverType));

	// 0 leaves the feature level uncapped. Otherwise 110 or 111, matching
	// D3D_FEATURE_LEVEL_11_0 and 11_1, for reproducing a lower-tier machine.
	KEY_INT (featureLevelCap, 0);
	FATAL(ms_featureLevelCap != 0 && ms_featureLevelCap != 110 && ms_featureLevelCap != 111, ("[Direct3d11] featureLevelCap %d is invalid [0, 110, 111]", ms_featureLevelCap));

	// The D3D11 debug layer plus an info queue that breaks on corruption and
	// errors. Default follows the build: on for a developer build, off for
	// Release, because it costs real frame time and would contaminate a
	// measurement.
#if DEBUG_LEVEL == 0
	KEY_BOOL(debugLayer, false);
#else
	KEY_BOOL(debugLayer, true);
#endif

	// Present without waiting for vblank where the swap chain supports it. The
	// benchmark profile sets this: with it false the present interval pins the
	// frame rate to the refresh rate and every percentile becomes meaningless.
	KEY_BOOL(allowTearing, false);
	KEY_BOOL(useCompiledShaders, true);
	KEY_BOOL(bakeCompiledShaders, false);
	KEY_INT (fullscreenRefreshRate, 0);

	// Three buffers is the flip-model default: two leaves the CPU waiting on the
	// presented buffer, more than three only adds latency.
	KEY_INT (swapChainBufferCount, 3);
	FATAL(ms_swapChainBufferCount < 2 || ms_swapChainBufferCount > 4, ("[Direct3d11] swapChainBufferCount %d is invalid [2..4]", ms_swapChainBufferCount));

	// Multisampling stays off by default, which is what the DX9 build
	// effectively does. Turning it on is a deliberate, separately measured
	// change, not something a default should decide.
	KEY_BOOL(antiAlias, false);
	KEY_INT (antiAliasSampleCount, 1);
	FATAL(ms_antiAliasSampleCount != 1 && ms_antiAliasSampleCount != 2 && ms_antiAliasSampleCount != 4 && ms_antiAliasSampleCount != 8, ("[Direct3d11] antiAliasSampleCount %d is invalid [1, 2, 4, 8]", ms_antiAliasSampleCount));

	// 0 reports the capability the backend actually implements. Any other value
	// is a diagnostic override, and it is loud: the value is compared for exact
	// equality against what shader assets were authored for, so a wrong one does
	// not degrade the image, it stops shader implementations being selected at
	// all.
	KEY_INT (shaderCapabilityOverride, 0);
	WARNING(ms_shaderCapabilityOverride != 0, ("[Direct3d11] shaderCapabilityOverride is set to 0x%04x. Shader implementation selection tests this for exact equality.", ms_shaderCapabilityOverride));

	CrashReportInformation::addStaticText("D3d11 adapter: %d driverType: %d featureLevelCap: %d\n", ms_adapter, static_cast<int>(ms_driverType), ms_featureLevelCap);
	CrashReportInformation::addStaticText("D3d11 allowTearing: %d bufferCount: %d antiAlias: %d/%d\n", ms_allowTearing ? 1 : 0, ms_swapChainBufferCount, ms_antiAlias ? 1 : 0, ms_antiAliasSampleCount);
}

// ----------------------------------------------------------------------

int ConfigDirect3d11::getAdapter()
{
	return ms_adapter;
}

// ----------------------------------------------------------------------

ConfigDirect3d11::DriverType ConfigDirect3d11::getDriverType()
{
	return ms_driverType;
}

// ----------------------------------------------------------------------

int ConfigDirect3d11::getFeatureLevelCap()
{
	return ms_featureLevelCap;
}

// ----------------------------------------------------------------------

bool ConfigDirect3d11::getDebugLayer()
{
	return ms_debugLayer;
}

// ----------------------------------------------------------------------

bool ConfigDirect3d11::getAllowTearing()
{
	return ms_allowTearing;
}

// ----------------------------------------------------------------------

bool ConfigDirect3d11::getUseCompiledShaders()
{
	return ms_useCompiledShaders;
}

// ----------------------------------------------------------------------

bool ConfigDirect3d11::getBakeCompiledShaders()
{
	return ms_bakeCompiledShaders;
}

// ----------------------------------------------------------------------

int ConfigDirect3d11::getFullscreenRefreshRate()
{
	return ms_fullscreenRefreshRate;
}

// ----------------------------------------------------------------------

int ConfigDirect3d11::getSwapChainBufferCount()
{
	return ms_swapChainBufferCount;
}

// ----------------------------------------------------------------------

bool ConfigDirect3d11::getAntiAlias()
{
	return ms_antiAlias;
}

// ----------------------------------------------------------------------

int ConfigDirect3d11::getAntiAliasSampleCount()
{
	return ms_antiAliasSampleCount;
}

// ----------------------------------------------------------------------

int ConfigDirect3d11::getShaderCapabilityOverride()
{
	return ms_shaderCapabilityOverride;
}

// ======================================================================
