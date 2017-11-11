#ifndef __UIDeformerRotate_H__
#define __UIDeformerRotate_H__

//=============================================================================

#include "UIDeformer.h"

//=============================================================================

class UIDeformerRotate : public UIDeformer
{
public:
	//----------------------------------------------------------------------

	static const char * TypeName;

	class PropertyName
	{
	public:
		static const UILowerString HorizontalActive;
		static const UILowerString HorizontalRight; 
		static const UILowerString VerticalActive;
		static const UILowerString VerticalDown;
		static const UILowerString FromTheCenter;
		static const UILowerString Duration;
		static const UILowerString StartDelay;
		static const UILowerString SlideIn;
		static const UILowerString RotateStart;
		static const UILowerString RotateStop;
		static const UILowerString RotateStopAndHold;
	};

	//----------------------------------------------------------------------

	UIDeformerRotate();
	virtual ~UIDeformerRotate();

	virtual bool IsA( const UITypeID Type ) const;
	virtual const char * GetTypeName( void ) const;
	virtual UIBaseObject * Clone() const;

	virtual void GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual bool SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool GetProperty( const UILowerString & Name, UIString &Value ) const;		

	virtual bool Deform(UICanvas & canvas, UIFloatPoint const * points, UIFloatPoint * out, size_t count);
	virtual EffectResult Effect(UIBaseObject *theObject);
	virtual void Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );
	virtual void OnTargetChanged(UIWidget * newTarget, UIWidget * oldTarget);

	virtual UIFloatPoint GetDeformedScale() const;

private:
	UIDeformerRotate(UIDeformerRotate const & rhs);
	UIDeformerRotate const & operator=(UIDeformerRotate const & rhs);

	float TotalTime() const;
	void updatePoint(float const timeLerp, float const c, UIFloatPoint const & focus, UIFloatPoint const * const pointIn, UIFloatPoint * const pointOut) const;


private:
	bool mHorizontalActive;
	bool mHorizontalRight; 
	bool mVerticalActive;
	bool mVerticalDown;
	bool mFromTheCenter;
	float mDuration;
	float mTimeSeconds;
	float mStartDelay;
	bool mSlideIn;
	bool mTriggerEffect;
	float mRotateStart;
	float mRotateStop;
	bool mRotateStopAndHold;
	bool mTimerDirty;
	UIFloatPoint mCanvasScale;
};

//=============================================================================

inline bool UIDeformerRotate::IsA( const UITypeID Type ) const
{
	return (Type == TUIDeformerRotate) || UIDeformer::IsA( Type );
};

//----------------------------------------------------------------------

inline float UIDeformerRotate::TotalTime() const
{
	return mDuration + mStartDelay;
}

#endif // __UIDeformerRotate_H__