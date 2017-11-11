#ifndef __UIDIRECTDRAWOFFSCREENCANVAS_H__
#define __UIDIRECTDRAWOFFSCREENCANVAS_H__

#include "UIDirectDrawCanvas.h"

class UIDirectDrawOffscreenCanvas : public UIDirectDrawCanvas
{
public:	
										     UIDirectDrawOffscreenCanvas( const UISize & );
	virtual						    ~UIDirectDrawOffscreenCanvas();
										
	virtual bool			     IsA( const UITypeID ) const;
	virtual UIBaseObject	*Clone( void ) const { return new UIDirectDrawOffscreenCanvas (UISize::zero); }
										
	virtual bool			     Generate( void ) const;

private:
	UIDirectDrawOffscreenCanvas & operator=                     (const UIDirectDrawOffscreenCanvas &);
	                              UIDirectDrawOffscreenCanvas   (const UIDirectDrawOffscreenCanvas &);

};

#endif // __UIDIRECTDRAWOFFSCREENCANVAS_H__