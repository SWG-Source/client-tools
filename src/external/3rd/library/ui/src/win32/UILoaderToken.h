#ifndef __UILOADERTOKEN_H__
#define __UILOADERTOKEN_H__

#include "UILowerString.h"
#include "UIString.h"

#include <map>

struct UILoaderToken
{
public:
	typedef std::map<UILowerString, UIString>          UIStringMap;
	
	bool CreateFromXML( const UINarrowString &, UINarrowString::const_iterator &, UINarrowString &ParseError );
	void WriteToString( UINarrowString & ) const;

	Unicode::String     Header;
	UIStringMap         Attributes;
	bool                IsData;
	bool                IsContainer;
	bool                IsEndContainer;

	std::string         AttributeName;
	UIString            AttributeValue;

};

#endif
