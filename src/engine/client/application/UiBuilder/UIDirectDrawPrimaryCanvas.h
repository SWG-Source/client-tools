#ifndef __UIDIRECTDRAWPRIMARYCANVAS_H__
#define __UIDIRECTDRAWPRIMARYCANVAS_H__

#include "UIDirectDrawCanvas.h"

class UIDirectDrawPrimaryCanvas : public UIDirectDrawCanvas
{
public:
										    UIDirectDrawPrimaryCanvas( const UISize &, HWND );
	virtual						   ~UIDirectDrawPrimaryCanvas();
										
	virtual bool			    IsA( const UITypeID ) const;
	virtual UIBaseObject *Clone( void ) const { return 0; }
										
	virtual bool			    Generate( void ) const;

private:


	UIDirectDrawPrimaryCanvas & operator=                   (const UIDirectDrawPrimaryCanvas &);
	                            UIDirectDrawPrimaryCanvas   (const UIDirectDrawPrimaryCanvas &);

	HWND				          mWindow;
};

#endif // __UIDIRECTDRAWPRIMARYCANVAS_H__