#ifndef __UISIZEEFFECTOR_H__
#define __UISIZEEFFECTOR_H__

#include "UIEffector.h"
#include "UINotification.h"

//======================================================================================

class UISizeEffector : public UIEffector, public UINotification
{
public:

	static const char			*TypeName;

	class PropertyName
	{
	public:
		static const UILowerString Speed;
		static const UILowerString TargetSize;
	};

												 UISizeEffector( void );
												 UISizeEffector( const UISize &TargetSize, const UISize &SizeSpeed );
	virtual								~UISizeEffector( void );

	virtual bool					 IsA( const UITypeID Type ) const;
	virtual const char    *GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void               GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void				   GetPropertyNames( UIPropertyNameVector &, bool forCopy  ) const;
	virtual bool				   SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool				   GetProperty( const UILowerString & Name, UIString &Value ) const;					

	virtual EffectResult	 Effect( UIBaseObject *theObject );

					void					 SetTargetSize( const UISize &TargetSize ) { mTargetSize = TargetSize; };

	virtual void					 Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	void                           SetSpeed (const UISize & speed);

private:

					void					 CalculateHeartbeatSkips( void );

	UISize mTargetSize;
	UISize mSizeSpeed;

	UISize mSizeChange;

	float	 mVerticalTime;
	float	 mVerticalTimePerPixel;
	float	 mHorizontalTime;
	float	 mHorizontalTimePerPixel;
};

//======================================================================================

inline bool UISizeEffector::IsA( const UITypeID Type ) const
{
	return (Type == TUISizeEffector) || UIEffector::IsA( Type );
};

#endif // __UISIZEEFFECTOR_H__