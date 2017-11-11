//======================================================================
//
// UIPie.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIPie_H
#define INCLUDED_UIPie_H

#include "UIWidget.h"
#include "UINotification.h"

//======================================================================

class UIPieStyle;

//----------------------------------------------------------------------

class UIPie :
public UIWidget,
public UINotification
{
public:

	static const char * const TypeName;

	struct PropertyName
	{ //lint !e578 // symbol hides symbol
		static const UILowerString PieInnerRadiusType;
		static const UILowerString PieSegmentStretchType;
		static const UILowerString PieWinding;
		static const UILowerString PieInnerRadius;
		static const UILowerString PieInnerRadiusPackInfo;
		static const UILowerString PieAngleStart;
		static const UILowerString PieAngleClipFinish;
		static const UILowerString PieValue;
		static const UILowerString PieUseTextureOnce;
		static const UILowerString Style;
		static const UILowerString PieColor;
		static const UILowerString PieOpacity;
		static const UILowerString SquaredOff;
		
	};

	enum PieInnerRadiusType
	{
		PIRT_center,
		PIRT_fixed,
		PIRT_proportional,
		PIRT_absolute
	};

	enum PieSegmentStretchType
	{
		PSST_none,
		PSST_stretch,
		PSST_shrink,
		PSST_leftover
	};

	enum PieWinding
	{
		PW_ccw,
		PW_cw
	};

	struct PieInnerRadiusTypeNames
	{
		static const Unicode::String Center;
		static const Unicode::String Fixed;
		static const Unicode::String Proportional;
		static const Unicode::String Absolute;
	};

	struct PieSegmentStretchTypeNames
	{
		static const Unicode::String None;
		static const Unicode::String Stretch;
		static const Unicode::String Shrink;
		static const Unicode::String Leftover;
	};

	struct PieWindingNames
	{
		static const Unicode::String ccw;
		static const Unicode::String cw;
	};

	UIPie ();
	~UIPie ();

	bool                IsA                          (const UITypeID Type) const;
	const char *        GetTypeName                  () const;
	UIBaseObject *      Clone                        () const;

	virtual void        GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	void                GetPropertyNames             (UIPropertyNameVector &, bool forCopy ) const;
	virtual void        GetLinkPropertyNames (UIPropertyNameVector &) const;

	bool                SetProperty                  (const UILowerString & Name, const UIString &Value );
	bool                GetProperty                  (const UILowerString & Name, UIString &Value ) const;

	UIStyle *           GetStyle                     () const;
	void                Render                       (UICanvas &) const;
	void                RenderCenter                 (UICanvas & canvas) const;
	void                RenderRing                   (UICanvas & canvas) const;

	void                Link                         ();
	bool                ProcessMessage               (const UIMessage & msg);

	void                SetStyle                     (UIPieStyle * style);

	void                Notify                       (UINotificationServer *notifyingObject, UIBaseObject *contextObject, UINotification::Code notificationCode);

	void                SetAngleStartRadians         (float rad);
	void                SetAngleClipFinishRadians    (float rad);
	void                SetValue                     (float value);
	void                SetInnerRadius               (long r, bool resetPacking = true);

	static bool                FindInnerRadiusType          (const Unicode::String & str, PieInnerRadiusType & type);
	static bool                FindSegmentStretchType       (const Unicode::String & str, PieSegmentStretchType & type);
	static bool                FindWinding                  (const Unicode::String & str, PieWinding & type);

	static const Unicode::String & FindInnerRadiusTypeName      (PieInnerRadiusType type);
	static const Unicode::String & FindSegmentStretchTypeName   (PieSegmentStretchType type);
	static const Unicode::String & FindWindingName              (PieWinding type);

	void                SetInnerRadiusType           (PieInnerRadiusType type);
	void                SetSegmentStretchType        (PieSegmentStretchType type);
	void                SetWinding                   (PieWinding type);


private:

	UIPie &            operator= (const UIPie &);
	                   UIPie     (const UIPie &);

	void                     Recompute () const;
	void                     ResetPackSizes ();
	void                     UpdateFromPackInfo  ();

	PieInnerRadiusType      mInnerRadiusType;
	PieSegmentStretchType   mSegmentStretchType;
	PieWinding              mWinding;

	UIPieStyle *            mStyle;

	long                    mInnerRadius;
	UIPoint                 mInnerRadiusPackInfo;

	float                   mAngleStartRadians;
	float                   mAngleClipFinishRadians;
	mutable float           mAngleEndRadians;
	float                   mValue;

	mutable int             mNumSegments;
	mutable float           mAnglePerSegment;

	mutable UISize          mLastScrollExtent;

	mutable bool            mPackSizeInfoDirty;

	UIColor                 mPieColor;
	float                   mPieOpacity;

	mutable bool            mRecomputeDirty;

	bool                    mSquaredOff;
	bool                    mUseTextureOnce;
};

//----------------------------------------------------------------------

inline const char * UIPie::GetTypeName () const
{
	return TypeName;
}

//----------------------------------------------------------------------

inline UIBaseObject  * UIPie::Clone () const
{
	return new UIPie;
}

//======================================================================

#endif
