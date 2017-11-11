#ifndef __UICLIPBOARD_H__
#define __UICLIPBOARD_H__

#include "UITypes.h"
#include "UIString.h"

class UIClipboard
{
public:

										  UIClipboard( void );
									   ~UIClipboard( void );
									
	void						    SetText( const UIString & );
	void						    GetText( UIString & ) const;
	void                            EraseText ();

	// Clipboard is global
	static UIClipboard &gUIClipboard( void ){ if( !gSingleton ) gSingleton = new UIClipboard; return *gSingleton; };
	static void			    ExplicitDestroy( void ) { delete gSingleton; gSingleton = 0; };

private:

	static UIClipboard *gSingleton;

	UIString					  mText;
};

#endif // __UICLIPBOARD_H__
