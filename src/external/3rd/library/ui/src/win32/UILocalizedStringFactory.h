// ======================================================================
//
// UILocalizedStringFactory.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UILocalizedStringFactory_H
#define INCLUDED_UILocalizedStringFactory_H

#include "UITypes.h"
#include "UnicodeUtils.h"

// ======================================================================

/**
* The UILocalizedStringFactory is an abstract interface to a factory whose
* purpose is to spew forth localized Unicode strings on demand.
*
* The imeplementations of this factory must support Unicode string lookup
* by 8-bit string name as well as 32 bit integer id.
*/

class UILocalizedStringFactory
{
public:

	virtual bool      GetLocalizedString (const std::string & name, Unicode::String &dest) const = 0;

	bool      GetLocalizedString (const Unicode::String       & name, Unicode::String &dest) const;
	virtual                      ~UILocalizedStringFactory () = 0;
	                              UILocalizedStringFactory ();

protected:

private:


	                              UILocalizedStringFactory (const UILocalizedStringFactory & rhs);
	UILocalizedStringFactory &    operator=    (const UILocalizedStringFactory & rhs);
};

//-----------------------------------------------------------------

inline bool UILocalizedStringFactory::GetLocalizedString (const Unicode::String & name, Unicode::String & dest) const
{
	return GetLocalizedString (Unicode::wideToNarrow (name), dest);
}
// ======================================================================

#endif
