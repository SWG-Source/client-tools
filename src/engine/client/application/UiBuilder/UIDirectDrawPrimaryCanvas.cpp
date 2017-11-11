#include "FirstUiBuilder.h"
#include "UIDirectDrawPrimaryCanvas.h"

UIDirectDrawPrimaryCanvas::UIDirectDrawPrimaryCanvas( const UISize &Size, HWND NewWindow ) : UIDirectDrawCanvas( Size )
{
	mWindow = NewWindow;
}

UIDirectDrawPrimaryCanvas::~UIDirectDrawPrimaryCanvas()
{
}

bool UIDirectDrawPrimaryCanvas::IsA( const UITypeID Type ) const
{
	return (Type == TUIDirectDrawPrimaryCanvas) || UIDirectDrawCanvas::IsA( Type );
}

bool UIDirectDrawPrimaryCanvas::Generate( void ) const
{
	if( !mSurface )
	{		
		LPDIRECTDRAWSURFACE7 CreatedSurface;
		LPDIRECTDRAWCLIPPER	 SurfaceClipper;
		DDSURFACEDESC2			 ddsd = { sizeof( ddsd ) };
		HRESULT							 hr;
		
		ddsd.dwFlags				= DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		
		hr = gDirectDraw->CreateSurface( &ddsd, &CreatedSurface, 0 );

		if( FAILED( hr ) )
			return false;

		hr = gDirectDraw->CreateClipper( 0, &SurfaceClipper, 0 );		

		if( FAILED( hr ) )
		{
			assert( false );
			
			CreatedSurface->Release();
			return false;
		}

		hr = SurfaceClipper->SetHWnd( 0, mWindow );

		if( FAILED( hr ) )
		{
			assert( false );

			SurfaceClipper->Release();
			CreatedSurface->Release();
			return false;
		}

		hr = CreatedSurface->SetClipper( SurfaceClipper );

		if( FAILED( hr ) )
		{
			assert( false );

			SurfaceClipper->Release();
			CreatedSurface->Release();
			return false;
		}

		SurfaceClipper->Release();

		const_cast<UIDirectDrawPrimaryCanvas *>( this )->SetSurface( CreatedSurface );
	}
	else if( mSurface->IsLost() )
	{
		if( FAILED( mSurface->Restore() ) )
			return false;
	}
	return true;
}