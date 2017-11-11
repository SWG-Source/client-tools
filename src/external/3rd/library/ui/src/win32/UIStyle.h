#ifndef __UISTYLE_H__
#define __UISTYLE_H__

#include "UIBaseObject.h"

class UICanvas;

//lint -esym(1516,UIStyle::TypeName) // hides

//======================================================================================

class UIStyle :
public UIBaseObject
{
public:

	enum Layout
	{
		L_horizontal,
		L_vertical
	};

	static const char * const   TypeName; //lint !e1516 // data member hides inherited member

	                            UIStyle( void );
	virtual                    ~UIStyle( void );

	virtual bool                IsA( const UITypeID ) const;
	virtual const char         *GetTypeName( void ) const;

protected:

	UIStyle & operator= (const UIStyle &);
	          UIStyle   (const UIStyle &);

};

//======================================================================================

inline bool UIStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIStyle) || UIBaseObject::IsA( Type );
}

//----------------------------------------------------------------------

#endif // __UISTYLE_H__
