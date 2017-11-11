// ======================================================================
//
// UIRotationEffector.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIRotationEffector_H
#define INCLUDED_UIRotationEffector_H

#include "UIEffector.h"

//======================================================================================

class UIRotationEffector :
public UIEffector
{
public:

	static const char * const TypeName;

	class PropertyName
	{
	public:
		static const UILowerString  Period;
		static const UILowerString  Cycling;
		static const UILowerString  Target;

	};

	                       UIRotationEffector   ();
	                       UIRotationEffector   (const float period);

	virtual bool           IsA               (const UITypeID Type) const;
	virtual const char    *GetTypeName       () const;
	virtual UIBaseObject  *Clone             () const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames  (UIPropertyNameVector &, bool forCopy) const;
	virtual bool           SetProperty       (const UILowerString & Name, const UIString &Value);
	virtual bool           GetProperty       (const UILowerString & Name, UIString &Value) const;

	virtual EffectResult   Effect            (UIBaseObject *theObject);

	void                   SetPeriod         (const float period);

private:

	float         mPeriod;
	float         mStep;
	bool          mCycling;
	float         mTarget;
};

//======================================================================================

inline bool UIRotationEffector::IsA( const UITypeID Type ) const
{
	return (Type == TUIRotationEffector) || UIEffector::IsA( Type );
};

// ======================================================================

#endif
