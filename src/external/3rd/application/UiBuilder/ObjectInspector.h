#ifndef __OBJECTINSPECTOR_H__
#define __OBJECTINSPECTOR_H__

#include "UITypes.h"
#include "UIString.h"
#include "UIBaseObject.h"

class ObjectInspector
{
public:
			
			 ObjectInspector( HWND hWnd );
			~ObjectInspector( void );

	void SetObject( UIBaseObject * );
	UIBaseObject *GetObject( void ) const;

	bool GetSelectedPropertyName( UINarrowString &Out );
	void SendKeyDown( unsigned KeyCode );

	HWND GetTextControl( void ) const { return mTextOverlay; };

private:

	static LRESULT CALLBACK StaticWindowProc( HWND, UINT, WPARAM, LPARAM );
				 LRESULT					WindowProc( HWND, UINT, WPARAM, LPARAM );

	static LRESULT CALLBACK StaticTextboxWindowProc( HWND, UINT, WPARAM, LPARAM );
				 LRESULT					TextboxWindowProc( HWND, UINT, WPARAM, LPARAM );
	       
	void LoadListboxWithObjectProperties( void );
	void UpdateButtonEnabledState( void );

	void ShowTextControl( void );
	void UpdateTextControlFromProperty( void );
	void UpdatePropertyFromTextControl( void );

	HWND					mPropertyListbox;
	HWND					mTextOverlay;
	WNDPROC				mOldWindowProc;
	WNDPROC				mOldTextboxWindowProc;

	UIBaseObject::UIBaseObjectPointer mObject;
	UINarrowString		mCurrentPropertyName;

	unsigned int  mTimerID;

	HWND mPathWindow;
};

#endif /* __OBJECTINSPECTOR_H__ */