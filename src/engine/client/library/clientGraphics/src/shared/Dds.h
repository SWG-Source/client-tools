// dds.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_dds_H
#define	_INCLUDED_dds_H																				 

namespace DDS
{

//-----------------------------------------------------------------------
// This is based heavily upon DirectX8 samples DxTex 
// It defines constants and structures used when reading
// DDS files. 

struct DDS_PIXELFORMAT
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwABitMask;
};

inline const unsigned long MakeFourCC(char a, char b, char c, char d)
{
	const unsigned long r1 = static_cast<unsigned long>(static_cast<unsigned char>(a)) << 0;
	const unsigned long r2 = static_cast<unsigned long>(static_cast<unsigned char>(b)) << 8;
	const unsigned long r3 = static_cast<unsigned long>(static_cast<unsigned char>(c)) << 16;
	const unsigned long r4 = static_cast<unsigned long>(static_cast<unsigned char>(d)) << 24;

	return r1 | r2 | r3 | r4;
}

const long int DDS_ALPHA  = 0x00000001;
const long int DDS_FOURCC = 0x00000004;  // DDPF_FOURCC
const long int DDS_RGB    = 0x00000040;  // DDPF_RGB
const long int DDS_LUMINANCE = 0x00000080;
const long int DDS_INTENSITY = 0x00000100;

const DDS_PIXELFORMAT DDSPF_DXT1 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MakeFourCC('D','X','T','1'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT2 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MakeFourCC('D','X','T','2'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT3 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MakeFourCC('D','X','T','3'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT4 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MakeFourCC('D','X','T','4'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT5 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MakeFourCC('D','X','T','5'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_PNM1 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MakeFourCC('P','N','M','1'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_A8R8G8B8 = { sizeof(DDS_PIXELFORMAT), DDS_RGB | DDS_ALPHA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };
const DDS_PIXELFORMAT DDSPF_A1R5G5B5 = { sizeof(DDS_PIXELFORMAT), DDS_RGB | DDS_ALPHA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };
const DDS_PIXELFORMAT DDSPF_A4R4G4B4 = { sizeof(DDS_PIXELFORMAT), DDS_RGB | DDS_ALPHA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };
const DDS_PIXELFORMAT DDSPF_A8R3G3B2 = { sizeof(DDS_PIXELFORMAT), DDS_RGB | DDS_ALPHA, 0, 16, 0x000000c0, 0x00000016, 0x00000003, 0x0000ff00 };
const DDS_PIXELFORMAT DDSPF_R8G8B8 = { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_R5G6B5 = { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_R3G3B2 = { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0,  8, 0x00000000, 0x000000c0, 0x00000016, 0x00000003 };
const DDS_PIXELFORMAT DDSPF_A8 = { sizeof(DDS_PIXELFORMAT), DDS_ALPHA, 0, 8, 0x00000000, 0x00000000, 0x000000000, 0x000000ff };
const DDS_PIXELFORMAT DDSPF_I8 = { sizeof(DDS_PIXELFORMAT), DDS_INTENSITY, 0, 8, 0x000000ff, 0x00000000, 0x000000000, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_L8 = { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0, 8, 0x000000ff, 0x00000000, 0x000000000, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_L8A8 = { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE | DDS_ALPHA, 0, 8, 0x000000ff, 0x00000000, 0x000000000, 0x0000ff00 };
const DDS_PIXELFORMAT DDSPF_L4A4 = { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE | DDS_ALPHA, 0, 16, 0x0000000f, 0x00000000, 0x000000000, 0x000000f0 };
const DDS_PIXELFORMAT DDSPF_R4G4 = { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 8, 0x000000f0, 0x0000000f, 0x00000000, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_R8G8 = { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0000ff00, 0x000000ff, 0x00000000, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_A5R5G5 = { sizeof(DDS_PIXELFORMAT), DDS_RGB | DDS_ALPHA, 0, 16, 0x000003e0, 0x0000001f, 0x00000000, 0x00007c00 };
const DDS_PIXELFORMAT DDSPF_A6R5G5 = { sizeof(DDS_PIXELFORMAT), DDS_RGB | DDS_ALPHA, 0, 16, 0x000003e0, 0x0000001f, 0x00000000, 0x0000fc00 };
const DDS_PIXELFORMAT DDSPF_A8R8G8 = { sizeof(DDS_PIXELFORMAT), DDS_RGB | DDS_ALPHA, 0, 24, 0x0000ff00, 0x000000ff, 0x00000000, 0x00ff0000 };
 
const long int DDS_HEADER_FLAGS_TEXTURE    = 0x00001007;  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
const long int DDS_HEADER_FLAGS_MIPMAP     = 0x00020000;  // DDSD_MIPMAPCOUNT
const long int DDS_HEADER_FLAGS_VOLUME     = 0x00800000;  // DDSD_DEPTH
const long int DDS_HEADER_FLAGS_PITCH      = 0x00000008;  // DDSD_PITCH
const long int DDS_HEADER_FLAGS_LINEARSIZE = 0x00080000;  // DDSD_LINEARSIZE

const long int DDS_SURFACE_FLAGS_TEXTURE = 0x00001000; // DDSCAPS_TEXTURE
const long int DDS_SURFACE_FLAGS_MIPMAP  = 0x00400008; // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
const long int DDS_SURFACE_FLAGS_COMPLEX = 0x00000008; // DDSCAPS_COMPLEX

const long int DDS_COMPLEX_FLAGS_CUBEMAP = 0x00000200;      // DDSCAPS2_CUBEMAP
const long int DDS_CUBEMAP_POSITIVEX = 0x00000600; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
const long int DDS_CUBEMAP_NEGATIVEX = 0x00000a00; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
const long int DDS_CUBEMAP_POSITIVEY = 0x00001200; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
const long int DDS_CUBEMAP_NEGATIVEY = 0x00002200; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
const long int DDS_CUBEMAP_POSITIVEZ = 0x00004200; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
const long int DDS_CUBEMAP_NEGATIVEZ = 0x00008200; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

const long int DDS_CUBEMAP_ALLFACES = ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX | DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY | DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ );

const long int DDS_COMPLEX_FLAGS_VOLUMEMAP = 0x00200000;    // DDSCAPS2_VOLUME


struct DDS_HEADER
{
    DWORD dwSize;
    DWORD dwHeaderFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    DWORD dwPitchOrLinearSize;
    DWORD dwDepth; // only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
    DWORD dwMipMapCount;
    DWORD dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    DWORD dwSurfaceFlags;
    DWORD dwComplexFlags;
    DWORD dwReserved2[3];
};

} // namespace DDS
//-----------------------------------------------------------------------

#endif	// _INCLUDED_dds_H
