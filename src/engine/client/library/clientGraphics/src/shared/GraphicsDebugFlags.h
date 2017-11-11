// ======================================================================
//
// GraphicsDebugFlags.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GraphicsDebugFlags_H
#define INCLUDED_GraphicsDebugFlags_H

// ======================================================================

class GraphicsDebugFlags
{
public:

	static void install();

#ifdef _DEBUG
	static bool renderExtents;
	static bool renderHardpoints;
	static bool renderNormals;
	static bool renderVertexMatrices;
	static bool logCharacterSystem;
	static bool disablePrecalculatedLighting;
#endif
};

// ======================================================================

#endif
