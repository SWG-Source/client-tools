// ======================================================================
//
// Direct3d9_Metrics.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_Metrics.h"

#ifdef _DEBUG

#include "sharedFoundation/Clock.h"
#include "sharedDebug/DebugFlags.h"

// ======================================================================

namespace Direct3d9_MetricsNamespace
{
	float MB(int bytes);
	int   KB(int bytes);

	bool  ms_installed;
	bool  ms_debugReportDraw;
	bool  ms_debugReportDrawPerPass;
	bool  ms_debugReportTexture;
	bool  ms_debugReportVertexBuffer;
	bool  ms_debugReportIndexBuffer;
	bool  ms_debugReportMisc;
	bool  ms_debugReportTriangleRate;
	bool  ms_debugReportSetConstants;
}
using namespace Direct3d9_MetricsNamespace;

// draw statistics
int      Direct3d9_Metrics::vertices;
int      Direct3d9_Metrics::indices;
int      Direct3d9_Metrics::points;
int      Direct3d9_Metrics::lines;
int      Direct3d9_Metrics::triangles;
int      Direct3d9_Metrics::drawPrimitiveCalls;
int      Direct3d9_Metrics::drawIndexedPrimitiveCalls;
int      Direct3d9_Metrics::perPassDrawPrimitiveCalls[MAX_PASSES];
int      Direct3d9_Metrics::perPassDrawIndexedPrimitiveCalls[MAX_PASSES];
int      Direct3d9_Metrics::perPassTriangles[MAX_PASSES];

// texture statistics
int      Direct3d9_Metrics::setTextureCalls;
int      Direct3d9_Metrics::setTextureCacheMisses;
int      Direct3d9_Metrics::setTextureUniqueTextures;
int      Direct3d9_Metrics::textureMemoryTotal;
int      Direct3d9_Metrics::textureMemoryUsed;
int      Direct3d9_Metrics::textureMemoryCreated;
int      Direct3d9_Metrics::textureMemoryDestroyed;
int      Direct3d9_Metrics::textureMemoryModified;

// vertex buffer statistics
int      Direct3d9_Metrics::setVertexDeclarationCacheMisses;
int      Direct3d9_Metrics::setVertexDeclarationCalls;
int      Direct3d9_Metrics::setVertexBufferCalls;
int      Direct3d9_Metrics::setVertexBufferUniqueVertexBuffers;
int      Direct3d9_Metrics::setStreamSourceCalls;
int      Direct3d9_Metrics::setStreamSourceCacheMisses;
int      Direct3d9_Metrics::vertexBufferMemoryTotal;
int      Direct3d9_Metrics::vertexBufferMemoryUsed;
int      Direct3d9_Metrics::vertexBufferMemoryCreated;
int      Direct3d9_Metrics::vertexBufferMemoryDestroyed;
int      Direct3d9_Metrics::vertexBufferMemoryModified;
int      Direct3d9_Metrics::vertexBufferMemoryDynamic;
int      Direct3d9_Metrics::vertexBufferDiscards;

// index buffer statistics
int      Direct3d9_Metrics::setIndexBufferCalls;
int      Direct3d9_Metrics::setIndexBufferUniqueIndexBuffers;
int      Direct3d9_Metrics::setIndexBufferCacheMisses;
int      Direct3d9_Metrics::indexBufferMemoryTotal;
int      Direct3d9_Metrics::indexBufferMemoryUsed;
int      Direct3d9_Metrics::indexBufferMemoryCreated;
int      Direct3d9_Metrics::indexBufferMemoryDestroyed;
int      Direct3d9_Metrics::indexBufferMemoryModified;
int      Direct3d9_Metrics::indexBufferMemoryDynamic;
int      Direct3d9_Metrics::indexBufferDiscards;

// misc stats
int      Direct3d9_Metrics::setViewportCalls;
int      Direct3d9_Metrics::setTransformCalls;
int      Direct3d9_Metrics::setMaterialCalls;
int      Direct3d9_Metrics::setRenderStateCalls;
int      Direct3d9_Metrics::setRenderStateCacheMisses;
int      Direct3d9_Metrics::setSamplerStateCalls;
int      Direct3d9_Metrics::setSamplerStateCacheMisses;
int      Direct3d9_Metrics::setTextureStageStateCalls;
int      Direct3d9_Metrics::setTextureStageStateCacheMisses;
int      Direct3d9_Metrics::setVertexShaderCalls;
int      Direct3d9_Metrics::setVertexShaderCacheMisses;

int      Direct3d9_Metrics::setVertexShaderConstantsCalls;
int      Direct3d9_Metrics::setVertexShaderConstantsRegisters;
int      Direct3d9_Metrics::setPixelShaderConstantsCalls;
int      Direct3d9_Metrics::setPixelShaderConstantsRegisters;

int      Direct3d9_Metrics::reusedStaticShaders;
int      Direct3d9_Metrics::reusedShaderImplementations;

// ======================================================================

void Direct3d9_Metrics::install()
{
	ms_installed = true;
	DebugFlags::registerFlag(ms_debugReportDraw,               "Direct3d9", "reportDraw",            debugReportDraw);
	DebugFlags::registerFlag(ms_debugReportDrawPerPass,        "Direct3d9", "reportDrawPerPass",     debugReportDrawPerPass);
	DebugFlags::registerFlag(ms_debugReportTexture,            "Direct3d9", "reportTexture",         debugReportTexture);
	DebugFlags::registerFlag(ms_debugReportVertexBuffer,       "Direct3d9", "reportVertexBuffer",    debugReportVertexBuffer);
	DebugFlags::registerFlag(ms_debugReportIndexBuffer,        "Direct3d9", "reportIndexBuffer",     debugReportIndexBuffer);
	DebugFlags::registerFlag(ms_debugReportMisc,               "Direct3d9", "reportMisc",            debugReportMisc);
	DebugFlags::registerFlag(ms_debugReportTriangleRate,       "Direct3d9", "reportTriangleRate",    debugReportTriangleRate);
	DebugFlags::registerFlag(ms_debugReportSetConstants,       "Direct3d9", "reportSetConstants",    debugReportSetConstants);
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::remove()
{
	if (ms_installed)
	{
		ms_installed = false;
		DebugFlags::unregisterFlag(ms_debugReportDraw);
		DebugFlags::unregisterFlag(ms_debugReportDrawPerPass);
		DebugFlags::unregisterFlag(ms_debugReportTexture);
		DebugFlags::unregisterFlag(ms_debugReportVertexBuffer);
		DebugFlags::unregisterFlag(ms_debugReportIndexBuffer);
		DebugFlags::unregisterFlag(ms_debugReportMisc);
		DebugFlags::unregisterFlag(ms_debugReportTriangleRate);
	}
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::reset()
{
	vertices = 0;
	indices = 0;
	points = 0;
	lines = 0;
	triangles = 0;
	drawPrimitiveCalls = 0;
	drawIndexedPrimitiveCalls = 0;
	for (int i = 0; i < MAX_PASSES; ++i)
		perPassDrawPrimitiveCalls[i] = 0;
	for (int j = 0; j < MAX_PASSES; ++j)
		perPassDrawIndexedPrimitiveCalls[j] = 0;
	for (int k = 0; k < MAX_PASSES; ++k)
		perPassTriangles[k] = 0;

	setTextureCalls = 0;
	setTextureCacheMisses = 0;
	setTextureUniqueTextures = 0;
	// do not clear textureMemoryTotal
	textureMemoryUsed = 0;
	textureMemoryCreated = 0;
	textureMemoryDestroyed = 0;
	textureMemoryModified = 0;

	setVertexDeclarationCacheMisses = 0;
	setVertexDeclarationCalls = 0;
	setVertexBufferCalls = 0;
	setVertexBufferUniqueVertexBuffers = 0;
	setStreamSourceCalls = 0;
	setStreamSourceCacheMisses = 0;
	// do not clear vertexBufferMemoryTotal
	vertexBufferMemoryUsed = 0;
	vertexBufferMemoryCreated = 0;
	vertexBufferMemoryDestroyed = 0;
	vertexBufferMemoryModified = 0;
	vertexBufferMemoryDynamic = 0;
	vertexBufferDiscards = 0;

	setIndexBufferCalls = 0;
	setIndexBufferUniqueIndexBuffers = 0;
	setIndexBufferCacheMisses = 0;
	// do not clear indexBufferMemoryTotal
	indexBufferMemoryUsed = 0;
	indexBufferMemoryCreated = 0;
	indexBufferMemoryDestroyed = 0;
	indexBufferMemoryModified = 0;
	indexBufferMemoryDynamic = 0;
	indexBufferDiscards = 0;

	setViewportCalls = 0;
	setTransformCalls = 0;
	setMaterialCalls = 0;
	setRenderStateCalls = 0;
	setRenderStateCacheMisses = 0;
	setSamplerStateCalls = 0;
	setSamplerStateCacheMisses = 0;
	setTextureStageStateCalls = 0;
	setTextureStageStateCacheMisses = 0;
	setVertexShaderCalls = 0;
	setVertexShaderCacheMisses = 0;

	setVertexShaderConstantsCalls = 0;
	setVertexShaderConstantsRegisters = 0;
	setPixelShaderConstantsCalls = 0;
	setPixelShaderConstantsRegisters = 0;

	reusedStaticShaders = 0;
	reusedShaderImplementations = 0;
}

// ----------------------------------------------------------------------

float Direct3d9_MetricsNamespace::MB(int a)
{
	return static_cast<float>(a) / (1024.0f * 1024.0f);
}

// ----------------------------------------------------------------------

int Direct3d9_MetricsNamespace::KB(int a)
{
	return a / 1024;
}

// ----------------------------------------------------------------------
/**
 * Debug some debugging information about the Gl.
 */

void Direct3d9_Metrics::debugReportDraw()
{
	DEBUG_REPORT_PRINT(true, ("d:%6d=verts   %6d=ind   %6d=points   %6d=lines    %6d=tris\n", vertices, indices, points, lines, triangles));
	DEBUG_REPORT_PRINT(true, ("d:%6d=drawPrim   %6d=drawIP   %6d=tri/s\n", drawPrimitiveCalls, drawIndexedPrimitiveCalls, static_cast<int>(static_cast<float>(triangles) / Clock::frameTime())));
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::debugReportDrawPerPass()
{
	for (int i = 0; i < MAX_PASSES; ++i)
	{
		DEBUG_REPORT_PRINT(true, ("p:%6d=pass%s   %6d=drawPrim   %6d=drawInd   %6d=tri\n", i, (i == MAX_PASSES-1) ? "+" : " ", perPassDrawPrimitiveCalls[i], perPassDrawIndexedPrimitiveCalls[i], perPassTriangles[i]));
	}
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::debugReportTexture()
{
	DEBUG_REPORT_PRINT(true, ("t:%6d=uniq    %6d=miss   %6d=set\n", setTextureUniqueTextures, setTextureCacheMisses, setTextureCalls));
	DEBUG_REPORT_PRINT(true, ("t:%5.1f=used  %5.1f=tot  %5.1f=create   %5.1f=mod   %5.1f=dest MB\n", MB(textureMemoryUsed), MB(textureMemoryTotal), MB(textureMemoryCreated), MB(textureMemoryModified), MB(textureMemoryDestroyed)));
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::debugReportVertexBuffer()
{
	DEBUG_REPORT_PRINT(true, ("v:%5d=uniq  %5d=setVB  %5d=missStream  %5d=setStream  %5d=discard\n", setVertexBufferUniqueVertexBuffers, setVertexBufferCalls, setStreamSourceCacheMisses, setStreamSourceCalls, vertexBufferDiscards));
	DEBUG_REPORT_PRINT(true, ("v:%5d=used  %5d=tot  %5d=create   %5d=mod   %5d=dest   %5d=dyn KB\n", KB(vertexBufferMemoryUsed), KB(vertexBufferMemoryTotal), KB(vertexBufferMemoryCreated), KB(vertexBufferMemoryModified), KB(vertexBufferMemoryDestroyed), KB(vertexBufferMemoryDynamic)));
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::debugReportIndexBuffer()
{
	DEBUG_REPORT_PRINT(true, ("i:%5d=uniq  %5d=miss  %5d=setIB  %5d=discards\n", setIndexBufferUniqueIndexBuffers, setIndexBufferCacheMisses, setIndexBufferCalls, indexBufferDiscards));
	DEBUG_REPORT_PRINT(true, ("i:%5d=used  %5d=tot  %5d=create   %5d=mod   %5d=dest   %5d=dyn KB\n", KB(indexBufferMemoryUsed), KB(indexBufferMemoryTotal), KB(indexBufferMemoryCreated),  KB(indexBufferMemoryModified), KB(indexBufferMemoryDestroyed), KB(indexBufferMemoryDynamic)));
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::debugReportMisc()
{
	DEBUG_REPORT_PRINT(true, ("m:%6d=view     %6d=trans   %6d=material   %6d=missVs   %6d=vs\n", setViewportCalls, setTransformCalls, setMaterialCalls, setVertexShaderCacheMisses, setVertexShaderCalls));
	DEBUG_REPORT_PRINT(true, ("m:%6d=missRs   %6d=rs      %6d=missTss   %6d=tss\n", setRenderStateCacheMisses, setRenderStateCalls, setTextureStageStateCacheMisses, setTextureStageStateCalls));
	DEBUG_REPORT_PRINT(true, ("m:%6d=hitShad  %6d=hitImp", reusedStaticShaders, reusedShaderImplementations));
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::debugReportSetConstants()
{
	DEBUG_REPORT_PRINT(true, ("c:%5d=vsCalls %5d=vsReg %5d=psCalls %5d=psReg\n", setVertexShaderConstantsCalls, setVertexShaderConstantsRegisters, setPixelShaderConstantsCalls, setPixelShaderConstantsRegisters));
}

// ----------------------------------------------------------------------

void Direct3d9_Metrics::debugReportTriangleRate()
{
	const int    MAX = 200;
	static int   counter = 0;
	static float trianglesHistory[MAX];
	static float timeHistory[MAX];
	
	trianglesHistory[counter] = static_cast<float>(triangles);
	timeHistory[counter] = Clock::frameTime();
	if (++counter == MAX)
		counter = 0;

	float totalTriangles = 0.0f;
	float totalTime = 0.0f;
	for (int i = 0; i < MAX; ++i)
	{
		totalTriangles += trianglesHistory[i];
		totalTime += timeHistory[i];
	}

	DEBUG_REPORT_PRINT(true, ("r:%6d=tri/s\n", static_cast<int>(totalTriangles / totalTime)));
}

// ======================================================================

#endif
