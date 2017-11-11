#include "FirstUiBuilder.h"
#include "UIDirect3DTextureCanvas.h"

#include "UIDirect3DPrimaryCanvas.h"

#include <math.h>

UIDirect3DTextureCanvas::UIDirect3DTextureCanvas( const UISize &Size, const bool InSystemMemory ) : 
UIDirectDrawCanvas( Size ),
mHasShader (false)
{
	mRenderCanvas		= 0;
	mInSystemMemory = InSystemMemory;
}

//----------------------------------------------------------------------

UIDirect3DTextureCanvas::~UIDirect3DTextureCanvas()
{
	SetRenderCanvas( 0 );
}

//----------------------------------------------------------------------

bool UIDirect3DTextureCanvas::IsA( const UITypeID Type ) const
{
	return (Type == TUIDirect3DTextureCanvas) || UIDirectDrawCanvas::IsA( Type );
}

//----------------------------------------------------------------------

UIBaseObject *UIDirect3DTextureCanvas::Clone( void ) const
{
	return 0;
}

//----------------------------------------------------------------------

void UIDirect3DTextureCanvas::SetRenderCanvas( UIDirect3DPrimaryCanvas *NewRenderCanvas )
{
	if( NewRenderCanvas )
		NewRenderCanvas->Attach( this );

	if( mRenderCanvas )
		mRenderCanvas->Detach( this );

	mRenderCanvas = NewRenderCanvas;
}

//----------------------------------------------------------------------

UIDirect3DPrimaryCanvas *UIDirect3DTextureCanvas::GetRenderCanvas( void ) const
{
	return mRenderCanvas;
}

//----------------------------------------------------------------------

void UIDirect3DTextureCanvas::NotifyRenderCanvasChanged( void )
{
	DestoryAll();
}

//----------------------------------------------------------------------

bool UIDirect3DTextureCanvas::Generate( void ) const
{
	assert( mRenderCanvas );

	if( !mSurface )
	{
		LPDIRECTDRAWSURFACE7 CreatedSurface;
		DDSURFACEDESC2			 ddsd = { sizeof( ddsd ) };
		HRESULT							 hr;

		// Round texture size up to a power of two
		const_cast<UIDirect3DTextureCanvas *>( this )->mSize.x = (long)(pow( 2.0, ceil( log( (double) mSize.x ) / log( 2.0 ) ) ));
		const_cast<UIDirect3DTextureCanvas *>( this )->mSize.y = (long)(pow( 2.0, ceil( log( (double) mSize.y ) / log( 2.0 ) ) ));

		// Make texture a square
		if( mSize.x < mSize.y )
			const_cast<UIDirect3DTextureCanvas *>( this )->mSize.x = mSize.y;
		else
			const_cast<UIDirect3DTextureCanvas *>( this )->mSize.y = mSize.x;

		ddsd.dwFlags				 = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.dwWidth				 = GetWidth();
		ddsd.dwHeight				 = GetHeight();

		mRenderCanvas->GetPreferedPixelFormat( &ddsd.ddpfPixelFormat );

		if( mInSystemMemory )
			ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
		else
			ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;

		ddsd.ddsCaps.dwCaps2 = 0;	

		hr = gDirectDraw->CreateSurface( &ddsd, &CreatedSurface, 0 );

		if( FAILED( hr ) )
			return false;
		
		const_cast<UIDirect3DTextureCanvas *>( this )->SetSurface( CreatedSurface );
	}
	else if( mSurface->IsLost() )
	{
		if( FAILED( mSurface->Restore() ) )
			return false;
	}

	if( !UICanvas::Generate() )
	{
		if( mSurface )
			mSurface->Release();

		const_cast<UIDirect3DTextureCanvas *>( this )->mSurface = 0;
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool UIDirect3DTextureCanvas::CreateFromSurfaceDescription( LPDDSURFACEDESC2 pddsd )
{
	SetSurface( 0 );

	LPDIRECTDRAWSURFACE7 CreatedSurface;
	HRESULT							 hr;
		
	hr = gDirectDraw->CreateSurface( pddsd, &CreatedSurface, 0 );

	if( FAILED( hr ) )
		return false;
	
	SetSurface( CreatedSurface );
	return true;
}

//----------------------------------------------------------------------

void UIDirect3DTextureCanvas::SetHasShader                 (bool b)
{
	mHasShader = b;
}

//----------------------------------------------------------------------
