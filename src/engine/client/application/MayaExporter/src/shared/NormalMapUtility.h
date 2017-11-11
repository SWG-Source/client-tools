#ifndef INCLUDED_NormalMapUtility_H
#define INCLUDED_NormalMapUtility_H

// ======================================================================
//
// NormalMapUtility.h
// Copyright 2005 Sony Online Entertainment.
// All Rights Reserved.
//
// ======================================================================

class Vector;
class Image;

// ======================================================================

class NormalMapUtility
{
public:

	static uint8 rangeCompressNormalFloat(const float f);
	static void  packBgrNormal(uint8 *o_byteNormal, const float *i_floatNormal);
	static void  packBgrNormal(uint8 *o_byteNormal, const Vector &i_floatNormal);
	static void  unpackBgrNormal(Vector &o_normal, const uint8 *i_bgrNormal);

	static void   processScratchMap(
		float *o_normals, 
		const byte *scratchMap, 
		int width, 
		int height, 
		float scale,
		bool wrapU,
		bool wrapV
	);

	static Image *createScratchNormalMap(
		const byte *scratchHeightMap, 
		int width, 
		int height, 
		float scale,
		bool wrapU,
		bool wrapV
	);

	static Image *createScratchNormalMap(
		Image &source, 
		float scale,
		bool wrapU,
		bool wrapV
	);

	static void skirtNormalMap(uint8 *normals, int width, int height, int bytesPerPixel);
	static void fillNormalMapEmptySpace(uint8 *const normals, const int width, const int height, const int bytesPerPixel);
};

// ======================================================================
#endif
