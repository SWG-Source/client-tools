#ifndef __UIOPACITYEFFECTOR_H__
#define __UIOPACITYEFFECTOR_H__

#include "UIEffector.h"

class UIWidget;

//======================================================================================

class UIOpacityEffector : public UIEffector
{
public:

	static const char * const TypeName;

	class PropertyName
	{
	public:
		static const UILowerString Speed;
		static const UILowerString TargetOpacity;
		static const UILowerString Background;
		static const UILowerString Cycling;
		static const UILowerString TargetOpacity2;
	};

	                       UIOpacityEffector ();
	                       UIOpacityEffector (const float TargetOpacity, const float TargetOpacity2, const float OpacitySpeed, bool isCycling);

	virtual bool           IsA               (const UITypeID Type) const;
	virtual const char    *GetTypeName       () const;
	virtual UIBaseObject  *Clone             () const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames  (UIPropertyNameVector &, bool forCopy ) const;
	virtual bool           SetProperty       (const UILowerString & Name, const UIString &Value);
	virtual bool           GetProperty       (const UILowerString & Name, UIString &Value) const;

	virtual EffectResult   Effect            (UIBaseObject *theObject);

	void                   SetTargetOpacity  (const float TargetOpacity);
	void                   SetTargetOpacity2 (const float TargetOpacity);
	void                   SetOpacitySpeed   (const float OpacitySpeed);

private:

	EffectResult           handleOpacityChange (UIWidget & widget, const float targetOpacity) const;

private:

	bool  mBackground;
	float mTargetOpacity;
	float mTargetOpacity2;
	float mOpacitySpeed;
	float mOpacityStep;
	bool  mCycling;
};

//======================================================================================

inline bool UIOpacityEffector::IsA( const UITypeID Type ) const
{
	return (Type == TUIOpacityEffector) || UIEffector::IsA( Type );
};

//-----------------------------------------------------------------

#endif // __UIOPACITYEFFECTOR_H__