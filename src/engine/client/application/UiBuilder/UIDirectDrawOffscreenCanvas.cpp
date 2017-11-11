#include "FirstUiBuilder.h"
#include "UIDirectDrawOffscreenCanvas.h"

UIDirectDrawOffscreenCanvas::UIDirectDrawOffscreenCanvas( const UISize &Size ) : UIDirectDrawCanvas( Size )
{
}

UIDirectDrawOffscreenCanvas::~UIDirectDrawOffscreenCanvas()
{
}

bool UIDirectDrawOffscreenCanvas::IsA( const UITypeID Type ) const
{
	return (Type == TUIDirectDrawOffscreenCanvas) || UIDirectDrawCanvas::IsA( Type );
}

bool UIDirectDrawOffscreenCanvas::Generate( void ) const
{
	if( !mSurface )
	{
		LPDIRECTDRAWSURFACE7 CreatedSurface;
		DDSURFACEDESC2			 ddsd = { sizeof( ddsd ) };
		HRESULT							 hr;

		ddsd.dwFlags				 = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.dwWidth				 = mSize.x;
		ddsd.dwHeight				 = mSize.y;

		ddsd.ddsCaps.dwCaps	 = DDSCAPS_OFFSCREENPLAIN;
		ddsd.ddsCaps.dwCaps2 = 0;

		hr = gDirectDraw->CreateSurface( &ddsd, &CreatedSurface, 0 );

		if( FAILED( hr ) )
			return false;
		
		const_cast<UIDirectDrawOffscreenCanvas *>( this )->SetSurface( CreatedSurface );
	}
	else if( mSurface->IsLost() )
	{
		if( FAILED( mSurface->Restore() ) )
			return false;
	}

	return UICanvas::Generate();
}