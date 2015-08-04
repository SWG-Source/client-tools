// ======================================================================
//
// SetupClientGraphics.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupClientGraphics_H
#define INCLUDED_SetupClientGraphics_H

// ======================================================================

class SetupClientGraphics
{
public:

	struct Data
	{

		// graphics control
		bool            use3dSystem;

		int             rasterMajor;

		int             screenWidth;
		int             screenHeight;

		bool            windowed;
		bool            skipInitialClearViewport;

		int             colorBufferBitDepth;
		int             alphaBufferBitDepth;
		int             zBufferBitDepth;
		int             stencilBufferBitDepth;

		bool            preloadVertexColorShaderTemplates;
	};

public:

	static void setupDefaultGameData(Data &data);
	static void setupDefaultConsoleData(Data &data);
	static void setupDefaultMFCData(Data &data);

	static bool install(const Data &data);
};

// ======================================================================

#endif
