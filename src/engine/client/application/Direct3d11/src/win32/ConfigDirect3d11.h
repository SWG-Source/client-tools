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
	static bool        getAllowTearing();
	static int         getFullscreenRefreshRate();
	static int         getSwapChainBufferCount();
	static bool        getAntiAlias();
	static int         getAntiAliasSampleCount();
	static int         getShaderCapabilityOverride();
};

// ======================================================================

#endif
