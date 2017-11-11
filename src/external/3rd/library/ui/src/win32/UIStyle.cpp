#include "_precompile.h"

#include "UIStyle.h"
#include "UICanvas.h"

const char * const UIStyle::TypeName = "Style";

//----------------------------------------------------------------------

UIStyle::UIStyle( void ) :
UIBaseObject ()
{
}

//----------------------------------------------------------------------

UIStyle & UIStyle::operator= (const UIStyle & rhs)
{
	return static_cast<UIStyle &>(UIBaseObject::operator= (rhs));
}

//----------------------------------------------------------------------

UIStyle::UIStyle   (const UIStyle & rhs) : 
UIBaseObject (rhs)
{
}

//----------------------------------------------------------------------

UIStyle::~UIStyle( void )
{
}

//----------------------------------------------------------------------

const char *UIStyle::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------
