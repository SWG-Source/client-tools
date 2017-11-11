// ======================================================================
//
// UIComposite.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIComposite_H
#define INCLUDED_UIComposite_H

// ======================================================================

#include "UIPage.h"

//-----------------------------------------------------------------

class UIComposite : public UIPage
{
public:

	enum Orientation
	{
		O_horizontal,
		O_vertical
	};

	enum Alignment
	{
		A_front,
		A_center,
		A_back
	};

	enum SpacingType
	{
		ST_fill,
		ST_spread,
		ST_constant,
		ST_skinned
	};

	enum Padding
	{
		P_internal,
		P_external,
		P_both
	};

	static const char * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString  Orientation;
		static const UILowerString  Spacing;
		static const UILowerString  SpacingType;
		static const UILowerString  Alignment;
		static const UILowerString  Padding;
		static const UILowerString  FillRemainder;
	};

	class OrientationNames
	{
	public:
		static const std::string Horizontal;
		static const std::string Vertical;
	};

	class AlignmentNames
	{
	public:
		static const std::string Front;
		static const std::string Center;
		static const std::string Back;
	};

	class SpacingTypeNames
	{
	public:
		static const std::string Fill;
		static const std::string Spread;
		static const std::string Constant;
		static const std::string Skinned;
	};

	class PaddingNames
	{
	public:
		static const std::string Internal;
		static const std::string External;
		static const std::string Both;
	};

	                        UIComposite         ();
	virtual                ~UIComposite         ();
	
	virtual bool            IsA                 (const UITypeID) const;
	virtual const char     *GetTypeName         () const;
	virtual UIBaseObject   *Clone               () const;

	virtual void            GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const;
	virtual void            GetPropertyNames    (UIPropertyNameVector &, bool forCopy ) const;
	virtual bool            SetProperty         (const UILowerString & Name, const UIString &Value );
	virtual bool            GetProperty         (const UILowerString & Name, UIString &Value ) const;
	virtual void            CopyPropertiesFrom  (const UIBaseObject & rhs);

	virtual void            Pack                ();

	virtual bool            AddChild            (UIBaseObject * );
	virtual bool            InsertChildBefore   (UIBaseObject * childToAdd, const UIBaseObject * childToPrecede);
	virtual bool            InsertChildAfter    (UIBaseObject * childToAdd, const UIBaseObject * childTosucceed);
	virtual bool            RemoveChild         (UIBaseObject * );
	virtual bool            MoveChild           (UIBaseObject *, ChildMovementDirection );

	virtual void            Link                ();

	Orientation             GetOrientation      () const;
	void                    SetOrientation      (Orientation t);

	long                    GetSpacing          () const;
	void                    SetSpacing          (long s);

	SpacingType             GetSpacingType      () const;
	void                    SetSpacingType      (SpacingType type);

	Padding                 GetPadding          () const;
	void                    SetPadding          (Padding type);

	Alignment               GetAlignment        () const;
	void                    SetAlignment        (Alignment t);

private:
	                        UIComposite         (const UIComposite & rhs);
	UIComposite &           operator=           (const UIComposite & rhs);

	typedef ui_stdvector<UIWidget *>::fwd WidgetVector;

	int                     CountVisibleWidgets () const;
	void                    GetVisibleWidgets   (WidgetVector & wv) const;

	UIPoint                 PackFill            ();
	UIPoint                 PackConstant        (long spacing);
	void                    PackSpread          (long diff, int widgetCount);
	UIPoint PackSkinned();
	void                    PackAlignment       (long diff, int widgetCount);

	UIWidget * GetFillerWidget();

	bool         mInPacking;
	Orientation  mOrientation;
	long         mSpacing;
	Alignment    mAlignment;
	SpacingType  mSpacingType;
	Padding      mPadding;
	bool         mFillRemainder;
};

//----------------------------------------------------------------------

inline UIComposite::Orientation UIComposite::GetOrientation () const
{
	return mOrientation;
}

//----------------------------------------------------------------------

inline UIComposite::Alignment UIComposite::GetAlignment () const
{
	return mAlignment;
}

//-----------------------------------------------------------------

inline long UIComposite::GetSpacing () const
{
	return mSpacing;
}

//----------------------------------------------------------------------

inline UIComposite::SpacingType UIComposite::GetSpacingType () const
{
	return mSpacingType;
}

//----------------------------------------------------------------------

inline UIComposite::Padding UIComposite::GetPadding () const
{
	return mPadding;
}

// ======================================================================

#endif
