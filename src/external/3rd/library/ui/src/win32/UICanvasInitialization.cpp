#include "_precompile.h"

#if WIN32

#ifdef DEFINE_GUIDS
#define INITGUID
#endif

#include <cassert>
#include <ddraw.h>
#include <d3d.h>

#include "UIDirect3DTextureCanvas.h"
#include "UIBitmapCanvasGenerator.h"
#include "UIDDSCanvasGenerator.h"
#include "UITGACanvasGenerator.h"
#include "UITextStyle.h"

LPDIRECTDRAW7 gDirectDraw = 0;
LPDIRECT3D7   gDirect3D   = 0;

extern UIDirect3DPrimaryCanvas *gPrimaryDisplay;

bool InitializeCanvasSystem( void *arg )
{
	HWND		hwnd = reinterpret_cast<HWND>( arg );
	HRESULT	hr;	

	hr = DirectDrawCreateEx( 0, (void **)&gDirectDraw, IID_IDirectDraw7, 0 );

	if( FAILED(hr) )
	{
		assert( false );
		return false;
	}
		
	hr = gDirectDraw->QueryInterface( IID_IDirect3D7, (void **)&gDirect3D );
	
	if( FAILED(hr) )
	{
		assert( false );

		gDirectDraw->Release();
		return false;
	}

	hr = gDirectDraw->SetCooperativeLevel( hwnd, DDSCL_NORMAL );

	if( FAILED( hr ) )
	{
		assert( false );

		gDirectDraw->Release();
		gDirect3D->Release();
		return false;
	}
	return true;
}

void ShutdownCanvasSystem( void *arg )
{
	gDirectDraw->Release();
}

UICanvas *CreateUICanvasFromResource( const UIString &Name )
{
	UIString								 NewName;
	UIDirect3DTextureCanvas	*c;
	UICanvasGenerator				*g;
	UISize									 s;
			
	NewName = Name;
	NewName.append( ".dds" );
	g = new UIDDSCanvasGenerator( NewName );

	if( !g->GetSize( s ) )
	{
		delete g;

		NewName = Name;
		NewName.append( ".bmp" );
		g = new UIBitmapCanvasGenerator( NewName );
		
		if( !g->GetSize( s ) )
		{
			delete g;

			NewName = Name;
			NewName.append( ".tga" );
			g = new UITGACanvasGenerator( NewName );

			if( !g->GetSize( s ) )
			{
				delete g;
				return 0;
			}
		}
	}
	
	c = new UIDirect3DTextureCanvas( s );
	
	c->SetRenderCanvas( gPrimaryDisplay );
	c->SetGenerator( g );	
	c->SetName( Name );

	return c;
}

#endif // WIN32