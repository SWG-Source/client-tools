#ifndef __SELECTIONRECT_H__
#define __SELECTIONRECT_H__

class UICanvas;

#include "UITypes.h"

class SelectionRect
{
public:

			 SelectionRect( void );

	void Render( UICanvas * );

	void ProcessMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void SetFillColor( UIColor NewFillColor )				{ mFillColor = NewFillColor; }
	void SetOutlineColor( UIColor NewOutlineColor ) { mOutlineColor = NewOutlineColor; }
	void SetRect( const UIRect &NewRect );
	void SetCanDrawHandles( bool NewCanDrawHandles ){ mCanDrawHandles = NewCanDrawHandles; }

	const UIRect &GetRect( void ) const							{ return mRect; }

	void SetBounds ( const UIRect & bounds );
	void SetUseBounds (bool b);

	static const long		LineWidth;
	static const long		HandleSize;
	static const double SizeThreshold;
	static const double MoveThreshold;

private:

	void ClipRectToBounds ();

private:

	UIColor		mFillColor;
	UIColor		mOutlineColor;
	UIRect		mRect;

	UIRect      mBounds;
	bool        mUseBounds;

	bool			mCanDrawHandles;

	bool			mSizeUp;
	bool			mSizeDown;
	bool			mSizeLeft;
	bool			mSizeRight;

	bool			mSizing;
	bool			mMoving;

	bool			mButtonDown;

	UIRect		mMouseDownRect;
	UIPoint		mMouseDownPoint;
};

#endif // __SELECTIONRECT_H__