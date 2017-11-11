//======================================================================
//
// UIImageFragment.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIImageFragment_H
#define INCLUDED_UIImageFragment_H

//======================================================================

#include "UIBaseObject.h"

class UICanvas;

//----------------------------------------------------------------------

class UIImageFragment : public UIBaseObject
{
public:

	static const char * const TypeName; //lint !e1516 // data member hides inherited data member

	class PropertyName
	{  //lint !e578 symbol hides symbol
	public:

		static const UILowerString Offset;
		static const UILowerString OffsetProportional;
		static const UILowerString SourceRect;
		static const UILowerString SourceResource;
	};

	UIImageFragment();
	virtual               ~UIImageFragment();

	virtual bool           IsA( const UITypeID ) const;
	virtual const char    *GetTypeName () const;
	virtual UIBaseObject  *Clone () const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void           GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool           SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual void           Render( UICanvas &, const UISize & size ) const;
	virtual void           Render( UICanvas & ) const;


	const UISize &         GetSize () const { return mSize; };
	void                   SetSize( const UIPoint &NewSize );
	void                   SetSize( UIScalar x, UIScalar y );

	long                   GetWidth () const;
	long                   GetHeight () const;

	void                   SetOffset( const UIPoint &NewOffset );
	void                   SetOffset( long x, long y );
	UIPoint                GetOffset () const;
	UICanvas *             GetCanvas ();
	UIRect                 GetSourceRect () const;

	void                   SetCanvas( UICanvas * );
	void                   SetSourceRect( const UIRect & );
	bool                   SetSourceResource( const UIString & );
	bool                   IsReadyToRender () const;

	static void GetStaticPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES);

private:

	UIImageFragment & operator=      (const UIImageFragment &);
	UIImageFragment   (const UIImageFragment &);

	UICanvas                            *mSourceCanvas;
	UIPoint                              mSourcePoint;
	UISize                               mSize;
	UIPoint                              mOffset;
	UIPoint                              mOffsetProportional;
	bool                                 mSourceRectSet;
};

//----------------------------------------------------------------------

inline bool UIImageFragment::IsReadyToRender () const
{
	return mSourceCanvas && mSourceRectSet;
}

//----------------------------------------------------------------------

inline void UIImageFragment::SetSize( const UIPoint &NewSize )
{
	mSize = NewSize;
}

//----------------------------------------------------------------------

inline void UIImageFragment::SetSize( UIScalar x, UIScalar y )
{
	mSize.x = x; mSize.y = y;
}

//----------------------------------------------------------------------

inline long UIImageFragment::GetWidth () const
{
	return mSize.x;
}

//----------------------------------------------------------------------

inline long UIImageFragment::GetHeight () const
{
	return mSize.y;
}

//----------------------------------------------------------------------

inline void UIImageFragment::SetOffset( const UIPoint &NewOffset )
{
	mOffset = NewOffset;
}

//----------------------------------------------------------------------

inline void UIImageFragment::SetOffset( long x, long y )
{
	mOffset.x = x; mOffset.y = y;
}

//----------------------------------------------------------------------

inline UIPoint UIImageFragment::GetOffset () const
{
	return mOffset;
}

//----------------------------------------------------------------------

inline UICanvas * UIImageFragment::GetCanvas ()
{
	return mSourceCanvas;
}

//----------------------------------------------------------------------

inline UIRect UIImageFragment::GetSourceRect () const
{
	return UIRect( mOffset + mSourcePoint, mSize);
}

//======================================================================

#endif
