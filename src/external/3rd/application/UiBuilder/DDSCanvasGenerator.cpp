#include "FirstUiBuilder.h"
#include "DDSCanvasGenerator.h"

#include "UIDirect3DTextureCanvas.h"
#include "UIDirect3DPrimaryCanvas.h"
#include "UIOutputStream.h"

#include <cstdio>

#include "FileLocator.h"

DDSCanvasGenerator::DDSCanvasGenerator( const UINarrowString &Filename ) :
mFilename (Filename)
{

#if SWG_HACK
	if (_strnicmp ("../texture/", mFilename.c_str (), 4) != 0)
		mFilename = "../texture/" + mFilename;
#endif

	if (_strnicmp (".dds", mFilename.substr (mFilename.length () - 4).c_str (), 4) != 0)
		mFilename += ".dds";

	UINarrowString result;

	if (FileLocator::gFileLocator ().findFile (mFilename.c_str (), result))
	{
		mFilename = result;
	}
}

DDSCanvasGenerator::~DDSCanvasGenerator()
{
}

bool DDSCanvasGenerator::GenerateOnto( UICanvas &DestinationCanvas ) const
{
	FILE					*fp;
	unsigned char *p;

	fp = fopen( mFilename.c_str(), "rb" );

	if( !fp )
	{
		*GetUIOutputStream() << "Error opening '" << mFilename.c_str() << "'.\n";
		return false;
	}

	fseek( fp, 0, SEEK_END );
	long len = ftell(fp);

	p = new unsigned char[len];

	fseek( fp, 0, SEEK_SET );

	if( !p )
	{
		fclose( fp );	
		return false;
	}

	fread( p, 1, len, fp );
	fclose( fp );

	bool rv = InitializeCanvasFromMemory( DestinationCanvas, p );

	if (!rv)
		*GetUIOutputStream() << "FAILED TO InitializeCanvasFromMemory '" << mFilename.c_str() << "'\n";

	delete p;
	return rv;
}

#if WIN32

#include <windows.h>
#include "UIDirectDrawCanvas.h"

bool DDSCanvasGenerator::GetSize( UISize &Out ) const
{
	DWORD           dwMagic;
	DDSURFACEDESC2  ddsd;
	FILE					 *fp;	
	
	fp = fopen( mFilename.c_str(), "rb" );
	
	if( !fp )
	{
		*GetUIOutputStream() << "Error opening '" << mFilename.c_str() << "'\n";
		return false;
	}
	
	fread( &dwMagic, sizeof(DWORD), 1, fp );
	if( dwMagic != MAKEFOURCC('D','D','S',' ') )
	{
		*GetUIOutputStream() << "File '" << mFilename.c_str() << "' is not an S3 compressed texture.\n";
		fclose( fp );
		return false;
	}
	
	if( !fread( &ddsd, sizeof( ddsd ), 1, fp ) )
	{
		*GetUIOutputStream() << "Error reading '" << mFilename.c_str() << "'\n";
		fclose( fp );
		return false;
	}
	
	fclose( fp );
	Out.x = ddsd.dwWidth;
	Out.y = ddsd.dwHeight;
	return true;
}

bool DDSCanvasGenerator::InitializeCanvasFromMemory( UICanvas &DestinationCanvas, unsigned char *pbase ) const
{
	assert( DestinationCanvas.IsA( TUIDirect3DTextureCanvas ) );
	
	DWORD										 dwMagic = *(DWORD *)pbase;
	DDSURFACEDESC2					 ddsd    = *(LPDDSURFACEDESC2)(pbase + sizeof( DWORD ));
	void										*pdata   = pbase + sizeof( DWORD ) + sizeof( DDSURFACEDESC2 );	
	UIDirect3DTextureCanvas  InMemoryCanvas( UISize( ddsd.dwWidth, ddsd.dwHeight ), true );
	UIDirect3DTextureCanvas *pDestCanvas = reinterpret_cast<UIDirect3DTextureCanvas *>( &DestinationCanvas );
	
	if( dwMagic != MAKEFOURCC('D','D','S',' ') )
	{
		*GetUIOutputStream() << "  :::   NOT A DDS\n";
		return false;
	}
	
	ddsd.ddsCaps.dwCaps  |= DDSCAPS_SYSTEMMEMORY;
	ddsd.dwMipMapCount    = 0;  
	ddsd.ddsCaps.dwCaps  &= ~( DDSCAPS_MIPMAP | DDSCAPS_COMPLEX );
	ddsd.dwFlags         &= ~(DDSD_PITCH | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE);
	
	if( !InMemoryCanvas.CreateFromSurfaceDescription( &ddsd ) )
	{
		*GetUIOutputStream() << "  :::   FAILED TO CREATE FROM SURFACE DESCRIPTION\n";
		return false;
	}
	
	if( FAILED( InMemoryCanvas.GetSurface()->Lock( 0, &ddsd, DDLOCK_WAIT, 0 ) ) )
	{
		*GetUIOutputStream() << "  :::   FAILED TO LOCK";
		return false;
	}
	
	if( ddsd.dwFlags & DDSD_LINEARSIZE )
		memcpy( ddsd.lpSurface, pdata, ddsd.dwLinearSize );
	else
	{
		BYTE* pDest   = (BYTE*)ddsd.lpSurface;
		BYTE* pSource = (BYTE*)pdata;
		DWORD dwBytesPerRow = ddsd.dwWidth * ddsd.ddpfPixelFormat.dwRGBBitCount / 8;
		
		for( DWORD yp = 0; yp < ddsd.dwHeight; yp++ )
		{
			memcpy( pDest, pSource,dwBytesPerRow );
			pDest   += ddsd.lPitch;
			pSource += dwBytesPerRow;
		}
	}
	
	if( FAILED( InMemoryCanvas.GetSurface()->Unlock( NULL ) ) )
	{
		*GetUIOutputStream() << "  :::   FAILED TO UNLOCK SURFACE\n";
		return false;
	}
	
	DDSURFACEDESC2 ddsdHardwareSurface = ddsd;
	
	if( !pDestCanvas->GetRenderCanvas()->MatchPixelFormat( &ddsd.ddpfPixelFormat, &ddsdHardwareSurface.ddpfPixelFormat ) )
	{
		*GetUIOutputStream() << "  :::   FAILED TO MATCH PIXEL FORMAT\n";
		return false;
	}
	
	ddsdHardwareSurface.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
	ddsdHardwareSurface.ddsCaps.dwCaps |=  DDSCAPS_VIDEOMEMORY;
	
	if( !pDestCanvas->CreateFromSurfaceDescription( &ddsdHardwareSurface ) )
	{
		*GetUIOutputStream() << "  :::   FAILED TO CREATE FROM SURFACE DESCRIPTION\n";
		return false;
	}
	
	pDestCanvas->BltFrom( &InMemoryCanvas, UIPoint(0,0), UIPoint(0,0), UISize( ddsd.dwWidth, ddsd.dwHeight ) );
	return true;
}

#endif