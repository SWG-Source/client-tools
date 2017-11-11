#ifndef __UISAVER_H__
#define __UISAVER_H__

#include "UIString.h"

#include <map>

class UIWidget;
class UIBaseObject;

//-----------------------------------------------------------------

class UISaver
{
public:

	typedef std::map<UINarrowString, UINarrowString> UINarrowStringMap;
	virtual void SaveToStringSet( UINarrowStringMap &Files, UINarrowStringMap::iterator CurrentFile, UIBaseObject &Source, int indent = 0) const;

	UISaver ();
	~UISaver ();

private:
	
	UIString MakeSafeDataValue( const UIString &Source ) const;

	typedef std::map<std::string, UIBaseObject *> DefaultObjectMap;
	mutable DefaultObjectMap m_defaultObjectMap;

};
//-----------------------------------------------------------------

#endif // __UISAVER_H__