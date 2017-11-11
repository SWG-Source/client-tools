#include "FirstUiBuilder.h"
#include "SelectionRect.h"

#include "UICanvas.h"

#include <windowsx.h>
#include <cmath>

const long		SelectionRect::LineWidth			= 1;
const long		SelectionRect::HandleSize			= 4;
const double	SelectionRect::MoveThreshold	= 5.0;
const double	SelectionRect::SizeThreshold	= 3.0;

SelectionRect::SelectionRect( void ) : mFillColor( 255, 0, 0 ),
mOutlineColor( 255, 0, 0 ),
mBounds (),
mUseBounds (false),
mCanDrawHandles(true),
mSizeUp(false),
mSizeDown(false),
mSizeLeft(false),
mSizeRight(false),
mSizing(false),
mMoving(false),
mButtonDown(false)
{	
}

void SelectionRect::Render( UICanvas *theCanvas )
{
	theCanvas->SetOpacity( mFillColor.a / 255.0f );

	theCanvas->ClearTo( mFillColor, UIRect( mRect.left, mRect.top,
		mRect.right, mRect.bottom ) );

	theCanvas->SetOpacity( mOutlineColor.a / 255.0f );

	theCanvas->ClearTo( mOutlineColor, UIRect( mRect.left, mRect.top,
		mRect.left + LineWidth, mRect.bottom ) );

	theCanvas->ClearTo( mOutlineColor, UIRect( mRect.right - LineWidth, mRect.top,
		mRect.right, mRect.bottom ) );

	theCanvas->ClearTo( mOutlineColor, UIRect( mRect.left + LineWidth, mRect.top,
		mRect.right - LineWidth, mRect.top + LineWidth ) );

	theCanvas->ClearTo( mOutlineColor, UIRect( mRect.left + LineWidth, mRect.bottom - LineWidth,
		mRect.right - LineWidth, mRect.bottom ) );

	if( mCanDrawHandles && (mRect.Height() >= 8) && (mRect.Width() >= 8) )
	{
		theCanvas->ClearTo( mOutlineColor, UIRect( mRect.left + LineWidth, mRect.top + LineWidth, 
			mRect.left + HandleSize, mRect.top + HandleSize ) );

		theCanvas->ClearTo( mOutlineColor, UIRect( mRect.right - HandleSize, mRect.top + LineWidth,
			mRect.right - LineWidth, mRect.top + HandleSize ) );

		theCanvas->ClearTo( mOutlineColor, UIRect( mRect.left + LineWidth, mRect.bottom - HandleSize,
			mRect.left + HandleSize, mRect.bottom - LineWidth ) );

		theCanvas->ClearTo( mOutlineColor, UIRect( mRect.right - HandleSize, mRect.bottom - HandleSize,
			mRect.right - LineWidth, mRect.bottom - LineWidth ) );

		if( mRect.Height() >= 16 && mRect.Width() >= 16 )
		{
			long HHandleLoc, VHandleLoc;

			HHandleLoc = (mRect.bottom + mRect.top - HandleSize) / 2;
			VHandleLoc = (mRect.right + mRect.left - HandleSize) / 2;

			theCanvas->ClearTo( mOutlineColor, UIRect( mRect.left + LineWidth, HHandleLoc,
				mRect.left + HandleSize, HHandleLoc + HandleSize ) );

			theCanvas->ClearTo( mOutlineColor, UIRect( mRect.right - HandleSize, HHandleLoc,
				mRect.right - LineWidth, HHandleLoc + HandleSize ) );

			theCanvas->ClearTo( mOutlineColor, UIRect( VHandleLoc, mRect.top + LineWidth,
				VHandleLoc + HandleSize, mRect.top + HandleSize ) );

			theCanvas->ClearTo( mOutlineColor, UIRect( VHandleLoc, mRect.bottom - HandleSize,
				VHandleLoc + HandleSize, mRect.bottom - LineWidth ) );
		}
	}
}

void SelectionRect::ProcessMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UI_UNREF (hwnd);

	switch( uMsg )
	{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:		
			mMouseDownPoint.x	= GET_X_LPARAM( lParam );
			mMouseDownPoint.y	= GET_Y_LPARAM( lParam );
			mMouseDownRect		= mRect;
			mButtonDown				= true;
			mMoving						= false;
			mSizing						= false;
			break;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			mButtonDown	= false;
			break;

		case WM_KEYDOWN:
		{
			UIPoint Nudge(0,0);

			switch( wParam )
			{
				case VK_UP:
					Nudge.y = -1;
					break;
				case VK_DOWN:
					Nudge.y = 1;
					break;
				case VK_LEFT:
					Nudge.x = -1;
					break;
				case VK_RIGHT:
					Nudge.x = 1;
					break;
			}

			mRect += Nudge;
			break;
		}

		case WM_MOUSEMOVE:
		{
			POINTS  MousePoint = MAKEPOINTS( lParam );
			HCURSOR hCursor    = 0;

			if( !mButtonDown )
			{
				long	 HHandleLoc;
				long   VHandleLoc;

				HHandleLoc = (mRect.bottom + mRect.top - HandleSize) / 2;
				VHandleLoc = (mRect.right + mRect.left - HandleSize) / 2;

				mSizeUp    = false;
				mSizeDown  = false;
				mSizeLeft  = false;
				mSizeRight = false;

				// Ordered so when very small the control will prefer to size down and to the right
				if( (MousePoint.x >= mRect.right - HandleSize) && (MousePoint.x <= mRect.right) )
				{
					if( (MousePoint.y <= mRect.top + HandleSize ) ||
							(MousePoint.y >= mRect.bottom - HandleSize ) ||
							(MousePoint.y >= HHandleLoc && MousePoint.y <= HHandleLoc + HandleSize) )
					{
						mSizeRight = true;
					}
				}
				else if( (MousePoint.x <= mRect.left + HandleSize) && (MousePoint.x >= mRect.left) )
				{
					if( (MousePoint.y <= mRect.top + HandleSize ) ||
							(MousePoint.y >= mRect.bottom - HandleSize ) ||
							(MousePoint.y >= HHandleLoc && MousePoint.y <= HHandleLoc + HandleSize) )
					{
						mSizeLeft = true;
					}
				}

				if( (MousePoint.y >= mRect.bottom - HandleSize) && (MousePoint.y <= mRect.bottom) )
				{
					if( (MousePoint.x <= mRect.left + HandleSize ) ||
							(MousePoint.x >= mRect.right - HandleSize ) ||
							(MousePoint.x >= VHandleLoc && MousePoint.x <= VHandleLoc + HandleSize) )
					{
						mSizeDown = true;
					}
				}
				else if( (MousePoint.y <= mRect.top + HandleSize) && (MousePoint.y >= mRect.top) )
				{
					if( (MousePoint.x <= mRect.left + HandleSize ) ||
							(MousePoint.x >= mRect.right - HandleSize ) ||
							(MousePoint.x >= VHandleLoc && MousePoint.x <= VHandleLoc + HandleSize) )
					{
						mSizeUp = true;
					}
				}						
			}

			if( mSizeUp )
			{
				if( mSizeLeft )
					hCursor = LoadCursor( 0, IDC_SIZENWSE );
				else if( mSizeRight )
					hCursor = LoadCursor( 0, IDC_SIZENESW );
				else
					hCursor = LoadCursor( 0, IDC_SIZENS );
			}
			else if( mSizeDown )
			{
				if( mSizeLeft )
					hCursor = LoadCursor( 0, IDC_SIZENESW );
				else if( mSizeRight )
					hCursor = LoadCursor( 0, IDC_SIZENWSE );
				else
					hCursor = LoadCursor( 0, IDC_SIZENS );
			}
			else if( mSizeLeft || mSizeRight )
				hCursor = LoadCursor( 0, IDC_SIZEWE );
		
			if( !hCursor )
				hCursor = LoadCursor( 0, IDC_ARROW );

			SetCursor( hCursor );

			if( mButtonDown )
			{
				if( mMoving )
				{
					mRect = mMouseDownRect;
					mRect += UIPoint(MousePoint.x, MousePoint.y) - mMouseDownPoint;
				}
				else if( mSizing )
				{
					UISize NewSize;
					UIPoint NewLocation;

					if( mSizeLeft )
					{
						NewSize.x     = mMouseDownRect.Width() + (mMouseDownPoint.x - MousePoint.x);
						NewLocation.x = mMouseDownRect.left - (mMouseDownPoint.x - MousePoint.x);
					}
					else if( mSizeRight )
					{
						NewSize.x     = mMouseDownRect.Width() - (mMouseDownPoint.x - MousePoint.x);
						NewLocation.x = mMouseDownRect.left;
					}
					else
					{
						NewSize.x     = mMouseDownRect.Width();
						NewLocation.x = mMouseDownRect.left;
					}
							
					if( mSizeUp )
					{
						NewSize.y     = mMouseDownRect.Height() + (mMouseDownPoint.y - MousePoint.y);
						NewLocation.y = mMouseDownRect.top - (mMouseDownPoint.y - MousePoint.y);
					}
					else if( mSizeDown )
					{
						NewSize.y     = mMouseDownRect.Height() - (mMouseDownPoint.y - MousePoint.y);
						NewLocation.y = mMouseDownRect.top;
					}
					else
					{
						NewSize.y     = mMouseDownRect.Height();
						NewLocation.y = mMouseDownRect.top;
					}

					if( NewSize.x < 1 )
						NewSize.x = 1;
					if( NewSize.y < 1 )
						NewSize.y = 1;
					
					mRect.left		= NewLocation.x;
					mRect.top			= NewLocation.y;
					mRect.right		= NewLocation.x + NewSize.x;
					mRect.bottom	= NewLocation.y + NewSize.y;
				}
				else
				{
					int    dx = MousePoint.x - mMouseDownPoint.x;
					int    dy = MousePoint.y - mMouseDownPoint.y;
					double d  = sqrt( (double)(dx * dx + dy * dy) );

					if( mSizeUp || mSizeDown || mSizeLeft || mSizeRight )
					{
						if( d > SizeThreshold )
							mSizing = true;
					}
					else if( d > MoveThreshold )
						mMoving = true;
				}
			}
			break;
		}
	}

	if (mUseBounds)
		ClipRectToBounds ();
}
//-----------------------------------------------------------------
void SelectionRect::SetBounds ( const UIRect & bounds )
{
	mBounds = bounds;

	if (mUseBounds)
		ClipRectToBounds ();

}
//-----------------------------------------------------------------
void SelectionRect::SetUseBounds (bool b)
{
	mUseBounds = b;

	if (mUseBounds)
		ClipRectToBounds ();
}
//-----------------------------------------------------------------
void SelectionRect::ClipRectToBounds ()
{
	mRect.bottom = min (mBounds.bottom, mRect.bottom);
	mRect.top    = max (mBounds.top,    mRect.top);
	mRect.left   = max (mBounds.left,   mRect.left);
	mRect.right  = min (mBounds.right,  mRect.right);
}
//-----------------------------------------------------------------

void SelectionRect::SetRect( const UIRect &NewRect )
{ 
	mRect = NewRect;
	
	if (mUseBounds)
		ClipRectToBounds ();
}
