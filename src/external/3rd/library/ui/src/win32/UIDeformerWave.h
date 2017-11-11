#ifndef __UIDeformerWave_H__
#define __UIDeformerWave_H__

//=============================================================================

#include "UIDeformer.h"

//=============================================================================

class UIDeformerWave : public UIDeformer
{
public:
	//----------------------------------------------------------------------

	static const char			*TypeName;

	class PropertyName
	{
	public:
		static const UILowerString Strength;
		static const UILowerString StrengthEnd;
		static const UILowerString Rate;
		static const UILowerString RateEnd;
		static const UILowerString Duration;
		static const UILowerString Interpolate;
	};

	//----------------------------------------------------------------------
	
	UIDeformerWave();
	virtual ~UIDeformerWave();

	virtual bool IsA( const UITypeID Type ) const;
	virtual const char * GetTypeName( void ) const;
	virtual UIBaseObject * Clone() const;

	virtual EffectResult Effect( UIBaseObject *theObject );

	virtual void GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual bool SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool GetProperty( const UILowerString & Name, UIString &Value ) const;		

	virtual void Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	//----------------------------------------------------------------------

	virtual bool Deform(UICanvas & canvas, UIFloatPoint const * points, UIFloatPoint * out, size_t count);
	virtual void OnTargetChanged(UIWidget * newTarget, UIWidget * oldTarget);


	void SetDuration(float duration);
	void SetInterpolate(bool interp);

private:
	UIDeformerWave(UIDeformerWave const & rhs);
	UIDeformerWave const & operator=(UIDeformerWave const & rhs);

private:

	UIFloatPoint mStrength;
	UIFloatPoint mStrengthEnd;
	UIFloatPoint mRate;
	UIFloatPoint mRateEnd;

	//struct MultiTargetData need to make this happen.
	//{
		float mDuration;
		float mTime;
		bool mInterpolate;
	//};
};

//=============================================================================

inline bool UIDeformerWave::IsA( const UITypeID Type ) const
{
	return (Type == TUIDeformerWave) || UIDeformer::IsA( Type );
};

#endif // __UIDeformerWave_H__