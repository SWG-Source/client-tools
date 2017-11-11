// ======================================================================
//
// NormalMapUtility.cpp
// Copyright 2005 Sony Online Entertainment.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "NormalMapUtility.h"
#include "sharedImage/Image.h"
#include "sharedMath/Vector2d.h"
#include "sharedMath/Vector.h"

// ======================================================================

#define BIT_UNDER_ONE (1.0f-1.0f/65536.0f)

// ======================================================================

namespace NormalMapUtilityNamespace
{
	inline uint8 _rangeCompressNormalFloat(const float f)
	{
		float rc = f * 0.5f + 0.5f;
		const float byteMax = 256.0f * BIT_UNDER_ONE;
		int rci = static_cast<int>(floor(rc * byteMax));
		DEBUG_FATAL(rci<0 || rci>=256, ("Byte value out of range.\n"));
		return static_cast<uint8>(rci);
	}

	//----------------------------------------------------------
	// least squares line fit for three data points.
	// solve for best line of form y = mx
	float _bestSlope(
		float x1, float y1, 
		float x2, float y2, 
		float x3, float y3
	)
	{
		const float xSum = x1 + x2 + x3;
		const float ySum = y1 + y2 + y3;
		const float productOfSums = xSum * ySum;
		const float sumOfProducts = x1*y1 + x2*y2 + x3*y3;
		const float sumOfXSquares = x1*x1 + x2*x2 + x3*x3;
		const float sumOfXsSquared = xSum * xSum;

		const float denom = sumOfXSquares - sumOfXsSquared;
		if (denom==0)
		{
			return FLT_MAX;
		}
		const float num   = sumOfProducts - productOfSums;
		const float slope = num / denom;

		return slope;
	}
	//----------------------------------------------------------

}
using namespace NormalMapUtilityNamespace;

// ======================================================================

uint8 NormalMapUtility::rangeCompressNormalFloat(const float f)
{
	return _rangeCompressNormalFloat(f);
}

// ----------------------------------------------------------------------

void NormalMapUtility::packBgrNormal(uint8 *o_byteNormal, const float *i_floatNormal)
{
	o_byteNormal[2]=_rangeCompressNormalFloat(i_floatNormal[0]);
	o_byteNormal[1]=_rangeCompressNormalFloat(i_floatNormal[1]);
	o_byteNormal[0]=_rangeCompressNormalFloat(i_floatNormal[2]);
}

// ----------------------------------------------------------------------

void NormalMapUtility::packBgrNormal(uint8 *o_byteNormal, const Vector &i_floatNormal)
{
	o_byteNormal[2]=_rangeCompressNormalFloat(i_floatNormal.x);
	o_byteNormal[1]=_rangeCompressNormalFloat(i_floatNormal.y);
	o_byteNormal[0]=_rangeCompressNormalFloat(i_floatNormal.z);
}

// ----------------------------------------------------------------------

void NormalMapUtility::unpackBgrNormal(Vector &o_normal, const uint8 *i_bgrNormal)
{
	Vector normal;

	normal.x=float(i_bgrNormal[2]) / 255.0f * 2.0f - 1.0f;
	normal.y=float(i_bgrNormal[1]) / 255.0f * 2.0f - 1.0f;
	normal.z=float(i_bgrNormal[0]) / 255.0f * 2.0f - 1.0f;
	normal.normalize();

	o_normal=normal;
}

// ----------------------------------------------------------------------

void NormalMapUtility::processScratchMap(
	float *o_normals, 
	const byte *scratchMap, 
	int width, 
	int height, 
	float scale,
	bool wrapU,
	bool wrapV
)
{
	const float actualScale = scale / 32.0f;
   memset(o_normals,0,width*height*3*4);
   for (int y=0;y<height;y++)
   {
		int yp1, ym1;
		if (wrapV)
		{
			yp1 = (y+1==height) ? int(0) : y+1;
			ym1 = (y==0) ? int(height-1) : y-1;
		}
		else
		{
			yp1 = (y+1==height) ? height-1 : y+1;
			ym1 = (y==0) ? 0 : y-1;
		}

      for (int x=0;x<width;x++)
      {
			int xp1, xm1;

			if (wrapU)
			{
				xp1 = (x+1==width) ? int(0) : x+1;
				xm1 = (x==0) ? int(width-1) : x-1;
			}
			else
			{
				xp1 = (x+1==width) ? width-1 : x+1;
				xm1 = (x==0) ? 0 : x-1;
			}

			const float z      = actualScale * static_cast<float>(scratchMap[(y  *width)+  x]);
			const float northZ = actualScale * static_cast<float>(scratchMap[(yp1*width)+  x]);
			const float southZ = actualScale * static_cast<float>(scratchMap[(ym1*width)+  x]);
			const float eastZ  = actualScale * static_cast<float>(scratchMap[(y  *width)+xp1]);
			const float westZ  = actualScale * static_cast<float>(scratchMap[(y  *width)+xm1]);

			float m;

			//----------------------------------------------------------
			// solve for best line of form z = my
			m = _bestSlope(-1, southZ, 0, z, 1, northZ);
			Vector yNormal(0, -m, 1);
			yNormal.normalize();
			//----------------------------------------------------------

			//----------------------------------------------------------
			// solve for best line of form z = mx
			m = _bestSlope(-1, westZ, 0, z, 1, eastZ);
			Vector xNormal(-m, 0, 1);
			xNormal.normalize();
			//----------------------------------------------------------

			Vector normal = xNormal + yNormal;
			normal.normalize();

         int index=(y*width + x)*3; 
         o_normals[index+0]=normal.x;
         o_normals[index+1]=normal.y;
         o_normals[index+2]=normal.z;
      }
   }
}

// ----------------------------------------------------------------------

Image *NormalMapUtility::createScratchNormalMap(
	const byte *scratchHeightMap, 
	int width, 
	int height, 
	float scale,
	bool wrapU,
	bool wrapV
)
{
	Image *ret_val=0;
	ret_val=new Image;
	ret_val->setPixelInformation(0x00ff0000,0x0000ff00,0x000000ff,0x00000000);
	ret_val->setDimensions(width, height, 24, 3, 3*width);

	uint8 *pixels = ret_val->lock();

	float *normals = new float[width*height*3];
	processScratchMap(normals, scratchHeightMap, width, height, scale, wrapU, wrapV);

	for (int y=0;y<height;y++)
	{
		for (int x=0;x<width;x++)
		{
			uint8 *destPixel = pixels + (y*width + x)*3;
			float *srcPixel = normals + (y*width + x)*3;
			NormalMapUtility::packBgrNormal(destPixel, srcPixel);
		}
	}
	
	delete [] normals;
	ret_val->unlock();
	return ret_val;
}

// ----------------------------------------------------------------------

Image *NormalMapUtility::createScratchNormalMap(Image &source, float scale, bool wrapU, bool wrapV)
{
	Image::PixelFormat pixelFormat = source.getPixelFormat();
	if (pixelFormat!=Image::PF_w_8)
	{
		return 0;
	}
	const uint8 *pixels = source.lockReadOnly();
	if (!pixels)
	{
		return 0;
	}

	Image *ret_val = createScratchNormalMap(pixels, source.getWidth(), source.getHeight(), scale, wrapU, wrapV);
	source.unlock();
	return ret_val;
}

// ----------------------------------------------------------------------

void NormalMapUtility::skirtNormalMap(uint8 *const normals, const int width, const int height, const int bytesPerPixel)
{
   // 8 grid positions,
   int offx[8]={ 0,-1, 1, 0, 1,-1, 1,-1};
   int offy[8]={-1, 0, 0, 1, 1,-1,-1, 1};

   uint8 *source = new byte[width*height*bytesPerPixel];
   memcpy(source,normals,width*height*bytesPerPixel);

   //dilate filter
   //find a black pixel in the source image, and then write it with the color of some surronding pixel
   for (int x=1;x<width-1;x++)
   {
      for (int y=1;y<height-1;y++)
      {
         int index=(y*width + x)*bytesPerPixel;
         if (source[index+0]==0 && source[index+1]==0 && source[index+2]==0)
         {
            for (int j=0;j<8;j++)
            {
               int ti=((y+offy[j])*width + x+offx[j])*bytesPerPixel;
               if (source[ti+0]!=0 || source[ti+1]!=0 || source[ti+2]!=0)
               {
                  normals[index+0]=source[ti+0];
                  normals[index+1]=source[ti+1];
                  normals[index+2]=source[ti+2];
                  break;
               }
            }
         }
      }
   }
   delete [] source;
}

// ----------------------------------------------------------------------

void NormalMapUtility::fillNormalMapEmptySpace(uint8 *const normals, const int width, const int height, const int bytesPerPixel)
{
	uint8 *niter=normals;
	uint8 *const nstop=normals + width*height*bytesPerPixel;

	while (niter!=nstop)
	{
		if (niter[0]==0 && niter[1]==0 && niter[2]==0)
		{
			niter[2]=127;
			niter[1]=127;
			niter[0]=255;
		}
		niter+=bytesPerPixel;
	}
}
