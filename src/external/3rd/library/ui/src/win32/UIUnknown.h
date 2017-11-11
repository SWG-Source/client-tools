#ifndef __UIUNKNOWN_H__
#define __UIUNKNOWN_H__

#include "UIWidget.h"

class UIUnknown : public UIWidget
{
public:

	class PropertyName
	{
	public:
		static const UILowerString TypeName;
	};

	UIUnknown( void );
	virtual~UIUnknown( void );
	virtual bool            IsA( const UITypeID Type ) const;
	virtual const char *    GetTypeName( void ) const;
	virtual void            SetTypeName( const UINarrowString & str );
	virtual UIBaseObject *  Clone( void ) const;
	void                    Render (UICanvas & c) const;
	virtual void      GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void      GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual bool      SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool      GetProperty( const UILowerString & Name, UIString &Value ) const;
	
	virtual UIStyle * GetStyle( void ) const { return 0; };

private:

	UIUnknown( UIUnknown & );
	UIUnknown &operator = ( UIUnknown & );

	UINarrowString mTypeName;
};

#endif // __UIUNKNOWN_H__
