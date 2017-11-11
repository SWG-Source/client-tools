#ifndef __UITEMPLATECACHE_H__
#define __UITEMPLATECACHE_H__

#include "UIEventCallback.h"
#include "UINotification.h"
#include "UITypes.h"

class UITemplate;
class UIData;

//-----------------------------------------------------------------

class UITemplateCache :
public UINotification,
public UIEventCallback
{
public:

	typedef ui_stdvector<UIWidget *>::fwd                     WidgetVector;

	                  UITemplateCache ();
	                 ~UITemplateCache( void );
	
	virtual void      Notify( UINotificationServer *, UIBaseObject *, UINotification::Code );
	virtual void      OnWidgetDataChanged (const UIWidget * context, const UINarrowString & property, const UIString & value);
	WidgetVector     *Get( const UIData * ) const;
	void              Add( UIData *, WidgetVector * );
	void              Add( UIData *, UITemplate *, UIBaseObject * );
	void              Remove( UIData * );
	void              Clear( void );
	
private:
	
	struct DataToTemplateInstanceMap;
	DataToTemplateInstanceMap * mCache;

	UIData *                    mModifyingData;
};

#endif // __UITEMPLATECACHE_H__
