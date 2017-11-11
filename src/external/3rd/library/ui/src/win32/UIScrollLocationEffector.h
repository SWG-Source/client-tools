#ifndef __UISCROLLLOCATIONEFFECTOR_H__
#define __UISCROLLLOCATIONEFFECTOR_H__

#include "UIEffector.h"
#include "UINotification.h"

//======================================================================================

class UIScrollLocationEffector : public UIEffector, public UINotification
{
public:

	static const char			*TypeName;

	class PropertyName
	{
	public:
		static const UILowerString Speed;
		static const UILowerString TargetScrollLocation;
	};

												 UIScrollLocationEffector( void );
												 UIScrollLocationEffector( const UIPoint &TargetScrollLocation, const UIPoint &ScrollLocationSpeed );
	virtual								~UIScrollLocationEffector( void );

	virtual bool					 IsA( const UITypeID Type ) const;
	virtual const char    *GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void               GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void				   GetPropertyNames( UIPropertyNameVector &, bool forCopy  ) const;
	virtual bool				   SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool				   GetProperty( const UILowerString & Name, UIString &Value ) const;					

	virtual EffectResult	 Effect( UIBaseObject *theObject );

					void					 SetTargetScrollLocation( const UIPoint &TargetScrollLocation ) { mTargetScrollLocation = TargetScrollLocation; };

	virtual void					 Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

private:

					void					 CalculateHeartbeatSkips( void );

	UIPoint mTargetScrollLocation;
	UIPoint mScrollLocationSpeed;

	UIPoint mScrollLocationChange;

	float	 mVerticalTime;
	float	 mVerticalTimePerPixel;
	float	 mHorizontalTime;
	float	 mHorizontalTimePerPixel;
};

//======================================================================================

inline bool UIScrollLocationEffector::IsA( const UITypeID Type ) const
{
	return (Type == TUIScrollLocationEffector) || UIEffector::IsA( Type );
};

#endif // __UISCROLLLOCATIONEFFECTOR_H__