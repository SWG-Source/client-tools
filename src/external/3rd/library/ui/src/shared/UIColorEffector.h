// ======================================================================
//
// UIColorEffector.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIColorEffector_H
#define INCLUDED_UIColorEffector_H

#include "UIEffector.h"
class UIWidget;

//======================================================================================

class UIColorEffector : public UIEffector
{
public:

	static const char * const TypeName;

	class PropertyName
	{
	public:
		static const UILowerString  Speed;
		static const UILowerString  TargetColor;
		static const UILowerString  TargetColor2;
		static const UILowerString  Cycling;
		static const UILowerString  Background;
		static const UILowerString  BackgroundTint;
		static const UILowerString  Icon;
		static const UILowerString  RestoreColor;
		static const UILowerString  ForceColor;
	};

	                       UIColorEffector   ();
	                       UIColorEffector   (const UIColor & targetColor, const float speed, bool background, bool backgroundTint, bool icon);

	virtual bool           IsA               (const UITypeID Type) const;
	virtual const char    *GetTypeName       () const;
	virtual UIBaseObject  *Clone             () const;

	virtual void           GetPropertyGroups (UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames  (UIPropertyNameVector &, bool forCopy) const;
	virtual bool           SetProperty       (const UILowerString & Name, const UIString &Value);
	virtual bool           GetProperty       (const UILowerString & Name, UIString &Value) const;

	virtual EffectResult   Effect            (UIBaseObject *theObject);

	void                   SetTargetColor    (const UIColor & targetColor);
	void                   SetTargetColor2   (const UIColor & targetColor);
	void                   SetSpeed          (const float speed);

	virtual EffectResult OnCreate( UIBaseObject *theObject );
	virtual void OnDestroy( UIBaseObject *theObject );

private:

	EffectResult           handleColorChange (UIWidget * const widget, const UIColor & targetColor);
	
	UIColor GetCurrentColor(UIWidget const * const widget) const;
	void SetCurrentColor(UIWidget * const widget, UIColor const & color);

	bool          mBackground;
	bool          mBackgroundTint;
	bool          mIcon;
	UIColor       mTargetColor;
	UIColor       mTargetColor2;
	bool          mCycling;
	float         mSpeed;
	unsigned char mStep;
	UIColor mOriginalColor;
	bool mRestorelColor;
	bool mForceColor;
};

//======================================================================================

inline bool UIColorEffector::IsA( const UITypeID Type ) const
{
	return (Type == TUIColorEffector) || UIEffector::IsA( Type );
};

// ======================================================================

#endif
