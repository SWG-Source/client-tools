#ifndef __EFFECTOR_H__
#define __EFFECTOR_H__

#include "UIBaseObject.h"

//======================================================================================

class UIEffector : public UIBaseObject
{
public:

	enum EffectResult
	{
		Continue,
		Stop
	};

	class PropertyName
	{
	public:
		static const UILowerString SoundOnActivate;
		static const UILowerString SoundOnDeactivate;
		static const UILowerString SoundOnDelayActivate;
	};

	UIEffector();
	virtual ~UIEffector();

	virtual bool IsA( const UITypeID Type ) const;
	virtual EffectResult Effect( UIBaseObject *theObject ) = 0;
	virtual EffectResult OnCreate( UIBaseObject *theObject );
	virtual void OnDestroy( UIBaseObject *theObject );

	virtual void GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual bool SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool GetProperty( const UILowerString & Name, UIString &Value ) const;

	void PlaySoundActivate() const;
	void PlaySoundDeactivate() const;
	void PlaySoundDelay() const;

private:
	UIString mSoundOnActivate;
	UIString mSoundOnDeactivate;
	UIString mSoundOnDelayActivate;
};

//======================================================================================

#endif // __EFFECTOR_H__
