#ifndef __UIDeformer_H__
#define __UIDeformer_H__

//=============================================================================

#include "UIEffector.h"
#include "UINotification.h"
#include "UIWatcher.h"

//=============================================================================

class UICanvas;
class UIWidget;

class UIDeformer : public UIEffector, public UINotification, public UIWatchable
{
public:
	//----------------------------------------------------------------------
	static const char			*TypeName;

	//----------------------------------------------------------------------
	UIDeformer();
	virtual ~UIDeformer();

	virtual bool IsA( const UITypeID Type ) const;
	virtual const char * GetTypeName() const;
	virtual UIBaseObject * Clone() const = 0;

	virtual void GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual bool SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool GetProperty( const UILowerString & Name, UIString &Value ) const;					

	virtual EffectResult Effect( UIBaseObject *theObject );

	virtual bool Deform(UICanvas & canvas, UIFloatPoint const * points, UIFloatPoint * out, size_t count) = 0;

	virtual EffectResult OnCreate( UIBaseObject *theObject );

	virtual void SetTarget(UIWidget * target);
	virtual UIWidget * GetTarget() const;

	virtual bool Detach(const UIBaseObject *);

	UISize const & GetOriginalSize() const;

	virtual UIFloatPoint GetDeformedScale() const;

	virtual void Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

protected:

	//----------------------------------------------------------------------
	virtual void OnTargetChanged(UIWidget * newTarget, UIWidget * oldTarget);

private:
	UIDeformer(UIDeformer const & rhs);
	UIDeformer const & operator=(UIDeformer const & rhs);
	UIWidget * mTarget;
	UISize mOriginalSize;
};

//=============================================================================

inline bool UIDeformer::IsA( const UITypeID Type ) const
{
	return (Type == TUIDeformer) || UIEffector::IsA( Type );
};

//=============================================================================

inline UISize const & UIDeformer::GetOriginalSize() const
{
	return mOriginalSize;
}

#endif // __UIDeformer_H__
