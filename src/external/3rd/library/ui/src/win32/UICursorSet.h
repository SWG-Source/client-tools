//======================================================================
//
// UICursorSet.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UICursorSet_H
#define INCLUDED_UICursorSet_H

//======================================================================

#include "UIBaseObject.h"

class UICursor;

class UICursorSet :
public UIBaseObject
{
public:

	static const char * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString Count;
	};

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool                SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool                GetProperty( const UILowerString & hName, UIString &Value ) const;
	virtual void                Link();
	virtual bool                IsA( const UITypeID Type ) const;
	virtual const char         *GetTypeName()              const { return TypeName; }
	virtual UIBaseObject       *Clone()                    const { return new UICursorSet; }

	UICursorSet ();
	~UICursorSet ();

	UICursor * GetCursor (int index) const;
	void       SetCursor (int index, UICursor *);

	int         GetCount () const;

private:

	void        SetCount (int count);

	UICursor ** mCursors;
	int         mCount;
};

//----------------------------------------------------------------------

inline int         UICursorSet::GetCount () const
{
	return mCount;
}

//======================================================================

#endif
