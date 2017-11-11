//======================================================================
//
// UIPieStyle.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIPieStyle_H
#define INCLUDED_UIPieStyle_H

//======================================================================

#include "UIWidgetStyle.h"

class UIImageStyle;

//----------------------------------------------------------------------

class UIPieStyle :
public UIWidgetStyle
{
public:
	static const char * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString ImageStyle;
		static const UILowerString ImageBottomWidth;
		static const UILowerString ImageSegmentAngle;
		static const UILowerString ImageAutoAngle;
		static const UILowerString ImageUvs;
	};

	UIPieStyle ();
	~UIPieStyle ();

	bool                IsA         (const UITypeID type) const;
	const char         *GetTypeName () const;
	UIBaseObject       *Clone       () const;

	virtual void        GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	void                GetPropertyNames (UIPropertyNameVector &, bool forCopy ) const;
	virtual void        GetLinkPropertyNames (UIPropertyNameVector &) const;

	bool                SetProperty      (const UILowerString & Name, const UIString &Value );
	bool                GetProperty      (const UILowerString & Name, UIString &Value ) const;

	void                Link ();

	void                SetImageStyle             (UIImageStyle * style);

	float               GetImageStyleAngleRadians () const;

	void                SetImageBottomWidth       (long l);

	const UIImageStyle * const GetImageStyle () const;

	const UIFloatPoint * GetUvs () const;

private:

	UIPieStyle & operator=    (const UIPieStyle &);
	             UIPieStyle   (const UIPieStyle &);

	void           RecomputeImageStyleAngle () const;

	UIImageStyle *         mImageStyle;

	mutable float          mImageStyleAngleRadians;

	long                   mImageBottomWidth;

	mutable UIFloatPoint   mUvs [4];

	bool                   mImageAutoAngle;

	mutable bool           mImageStyleAngleDirty;
};

//----------------------------------------------------------------------

inline const char * UIPieStyle::GetTypeName () const
{
	return TypeName;
}

//----------------------------------------------------------------------

inline UIBaseObject * UIPieStyle::Clone       () const
{
	return new UIPieStyle;
}

//----------------------------------------------------------------------

inline float UIPieStyle::GetImageStyleAngleRadians () const
{
	if (mImageStyleAngleDirty)
		RecomputeImageStyleAngle ();
	return mImageStyleAngleRadians;
}

//----------------------------------------------------------------------

inline const UIImageStyle * const UIPieStyle::GetImageStyle () const
{
	return mImageStyle;
}

//----------------------------------------------------------------------

inline const UIFloatPoint * UIPieStyle::GetUvs () const
{
	if (mImageStyleAngleDirty)
		RecomputeImageStyleAngle ();
	return mUvs;
}

//======================================================================

#endif


