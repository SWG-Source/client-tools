#ifndef __UILOCATIONEFFECTOR_H__
#define __UILOCATIONEFFECTOR_H__

#include "UIEffector.h"
#include "UINotification.h"

//======================================================================================

class UILocationEffector : public UIEffector, public UINotification
{
public:

	static const char			*TypeName;

	class PropertyName
	{
	public:
		static const UILowerString Speed;
		static const UILowerString TargetLocation;
	};

												 UILocationEffector( void );
												 UILocationEffector( const UIPoint &TargetLocation, const UIPoint &LocationSpeed );
	virtual								~UILocationEffector( void );

	virtual bool					 IsA( const UITypeID Type ) const;
	virtual const char    *GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void               GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void				   GetPropertyNames( UIPropertyNameVector & , bool forCopy ) const;
	virtual bool				   SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool				   GetProperty( const UILowerString & Name, UIString &Value ) const;					

	virtual EffectResult	 Effect( UIBaseObject *theObject );

					void					 SetTargetLocation( const UIPoint &TargetLocation ) { mTargetLocation = TargetLocation; };

	virtual void					 Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

private:

					void					 CalculateHeartbeatSkips( void );

	UIPoint mTargetLocation;
	UIPoint mLocationSpeed;

	UIPoint mLocationChange;

	float	 mVerticalTime;
	float	 mVerticalTimePerPixel;
	float	 mHorizontalTime;
	float	 mHorizontalTimePerPixel;
};

//======================================================================================

inline bool UILocationEffector::IsA( const UITypeID Type ) const
{
	return (Type == TUILocationEffector) || UIEffector::IsA( Type );
};

#endif // __UILOCATIONEFFECTOR_H__