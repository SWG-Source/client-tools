#include "FirstUiBuilder.h"
#include "UIDirectDrawCanvas.h"

#include "UICanvasGenerator.h"
#include "UIUtils.h"

#include <d3d.h>

LPDIRECTDRAW7 gDirectDraw = 0;
LPDIRECT3D7   gDirect3D   = 0;

//======================================================================================
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

//======================================================================================
void ShutdownCanvasSystem( void *arg )
{
	UI_UNREF (arg);
	gDirectDraw->Release();
}

//======================================================================================
static int GenerateShift( DWORD dwMask )
{
	if( dwMask == 0 )
		return 0;

	for( int i = 0; !(dwMask & 0x01); ++i, dwMask >>= 1 )
		;

	return i;
}

//======================================================================================
static int CountBits( DWORD dwMask )
{
	int bitcount = 0;

	for( int i = 0; i < 32; ++i, dwMask >>= 1 )
		bitcount += dwMask & 0x01;

	return bitcount;
}

//======================================================================================
UIDirectDrawCanvas::UIDirectDrawCanvas( UISize	Size ) : UICanvas( Size )
{
	mSurface = 0;
}

//======================================================================================
bool UIDirectDrawCanvas::IsA( const UITypeID Type ) const
{
	return (Type == TUIDirectDrawCanvas) || UICanvas::IsA( Type );
}

//======================================================================================
UIBaseObject *UIDirectDrawCanvas::Clone( void ) const
{
	return 0;
}

//======================================================================================
UIDirectDrawCanvas::~UIDirectDrawCanvas( void )
{
	if( mSurface )
		mSurface->Release();
}

//======================================================================================
void UIDirectDrawCanvas::DestoryAll( void )
{
	if( mSurface )
	{
		mSurface->Release();
		mSurface = 0;
	}
}

//======================================================================================
void UIDirectDrawCanvas::SetSurface( LPDIRECTDRAWSURFACE7 NewSurface )
{
	if( mSurface )
		mSurface->Release();

	mSurface = NewSurface;

	if( mSurface )
	{
		DDSURFACEDESC2	ddsd = { sizeof( ddsd ) };
		HRESULT					hr   = mSurface->GetSurfaceDesc( &ddsd );

		if( FAILED( hr ) )
			assert( false );

		mRedMask    = ddsd.ddpfPixelFormat.dwRBitMask;
		mRedShift   = GenerateShift( mRedMask );
		mRedScale   = 8 - CountBits( mRedMask );

		mGreenMask  = ddsd.ddpfPixelFormat.dwGBitMask;
		mGreenShift = GenerateShift( mGreenMask );
		mGreenScale = 8 - CountBits( mGreenMask );

		mBlueMask   = ddsd.ddpfPixelFormat.dwBBitMask;
		mBlueShift  = GenerateShift( mBlueMask );
		mBlueScale  = 8 - CountBits( mBlueMask );

		if( ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS )
		{
			mAlphaMask   = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
			mAlphaShift  = GenerateShift( mAlphaMask );
			mAlphaScale  = 8 - CountBits( mAlphaMask );
		}
		else
		{
			mAlphaMask   = 0;
			mAlphaShift  = 0;
			mAlphaScale  = 0;
		}
	}
}

//======================================================================================
LPDIRECTDRAWSURFACE7 UIDirectDrawCanvas::GetSurface( void ) const
{
	return mSurface;
}

//======================================================================================
bool UIDirectDrawCanvas::Prepare( void ) const
{
	if( !mSurface || mSurface->IsLost() )
		// The theory behind this is that generate doesn't change what's on
		// the canvas, it just ensures that its loaded and up to date.
		return const_cast<UIDirectDrawCanvas *>( this )->Generate();
	else
		return true;
}

//======================================================================================
void UIDirectDrawCanvas::SetSize( const UIPoint &NewSize )
{
	UICanvas::SetSize( NewSize );

	if( mSurface )
	{
		mSurface->Release();
		mSurface = 0;
	}
}

//======================================================================================
void UIDirectDrawCanvas::BltFrom( const UICanvas * const srcarg, const UIPoint &Source, const UIPoint &Destination, const UISize &Size )
{
	const UIDirectDrawCanvas *const src = reinterpret_cast<const UIDirectDrawCanvas * const>( srcarg );
	
	if( !Prepare() )
		return;

	if( src && !src->Prepare() )
		return;
	
	UIPoint MappedDestination = Destination + mState.Translation;
	RECT		rcSrc, rcDest;
	UIRect	DestRect;

	DestRect.top		= MappedDestination.y;
	DestRect.left		= MappedDestination.x;
	DestRect.bottom	= MappedDestination.y + Size.y;
	DestRect.right	= MappedDestination.x + Size.x;		

	if( !UIUtils::ClipRect( DestRect, mState.ClippingRect ) )
		return;

	// Thunk because UIRect is not the same type as RECT
	rcDest.top    = DestRect.top;
	rcDest.left   = DestRect.left;
	rcDest.bottom = DestRect.bottom;
	rcDest.right  = DestRect.right;

	rcSrc.top			= Source.y   + (DestRect.top    - MappedDestination.y);
	rcSrc.left		= Source.x   + (DestRect.left   - MappedDestination.x);
	rcSrc.bottom	= rcSrc.top  + (DestRect.bottom - DestRect.top);
	rcSrc.right		= rcSrc.left + (DestRect.right  - DestRect.left);

	if( src )
		mSurface->Blt( &rcDest, src->mSurface, &rcSrc, DDBLT_WAIT, 0 );
	else
	{
		DDBLTFX ddbltfx;

		ddbltfx.dwSize			= sizeof( ddbltfx );
		ddbltfx.dwFillColor = FormatUIColorForDirectDrawSurface( mState.Color );

		mSurface->Blt( &rcDest, 0, 0, DDBLT_COLORFILL, &ddbltfx );
	}
}

//======================================================================================
void UIDirectDrawCanvas::Refresh( void )
{
	if( mSurface )
	{
		mSurface->Release();
		mSurface = 0;
	}

	Generate();
}