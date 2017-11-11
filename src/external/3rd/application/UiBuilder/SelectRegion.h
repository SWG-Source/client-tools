#ifndef __SELECTREGION_H__
#define __SELECTREGION_H__

#include "UITypes.h"
#include "UIString.h"
#include "UISmartPointer.h"
#include "selectionrect.h"

class UIImage;
class UIImageStyle;
class UIDirect3DPrimaryCanvas;

class SelectRegionDialogBox
{
public:

			 SelectRegionDialogBox( void );

	bool EditProperty( HWND hwndParent, UIImageStyle *theStyle, UIString &Value );
	bool EditProperty( HWND hwndParent, UIImage *theImage, UIString &Value );

private:

	void UpdateTextbox( void );
	void UpdateSelectionRect( void );
	void ReadRectFromTextbox( void );
	void EnableControls( void );
	void UpdateScrollbars( HWND hwnd, const UIPoint * center = 0 );
	void UpdateCanvasScrollPosition( HWND hwnd, UINT uMsg, WPARAM wParam );
	void SizeToContent( void );
	void ShowZoomLevel( void );
	LPARAM InverseTranslateLParam( LPARAM );

				 BOOL CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );	
				 BOOL CALLBACK CanvasProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	static BOOL CALLBACK StaticDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );	
	static BOOL CALLBACK StaticCanvasProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	HWND													 mHwnd;

	UIRect												 mRegion;
	UISmartPointer<UIImageStyle> mStyle;
	UISmartPointer<UIImage> mImage;
	UIDirect3DPrimaryCanvas				*mDisplay;

	float													 mZoomLevel;

	UISize												 mImageSize;
	UISize												 mOldSize;
	UISize												 mInitialSize;
	UIPoint												 mScrollOffset;

	SelectionRect									 mSelectionRect;
	bool													 mDrawHighlight;

	static SelectRegionDialogBox	*sThis;
};

#endif // __SELECTREGION_H__