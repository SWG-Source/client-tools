// ======================================================================
//
// GraphicsDebugFlags.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/GraphicsDebugFlags.h"

#include "sharedDebug/DebugFlags.h"

// ======================================================================

#ifdef _DEBUG

bool GraphicsDebugFlags::renderExtents;
bool GraphicsDebugFlags::renderHardpoints;
bool GraphicsDebugFlags::renderNormals;
bool GraphicsDebugFlags::renderVertexMatrices;
bool GraphicsDebugFlags::logCharacterSystem;
bool GraphicsDebugFlags::disablePrecalculatedLighting;

#endif

// ======================================================================

void GraphicsDebugFlags::install()
{
#ifdef _DEBUG

	DebugFlags::registerFlag(renderExtents,                  "ClientGraphics", "renderExtents");
	DebugFlags::registerFlag(renderHardpoints,               "ClientGraphics", "renderHardpoints");
	DebugFlags::registerFlag(renderNormals,                  "ClientGraphics", "renderNormals");
	DebugFlags::registerFlag(renderVertexMatrices,           "ClientGraphics", "renderVertexMatrices");
	DebugFlags::registerFlag(logCharacterSystem,             "ClientGraphics", "logCharacterSystem");
	DebugFlags::registerFlag(disablePrecalculatedLighting,   "ClientGraphics", "disablePrecalculatedLighting");

#endif
}

// ======================================================================
