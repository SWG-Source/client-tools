#ifndef __UIEVENTCALLBACK_H__
#define __UIEVENTCALLBACK_H__

class UIWidget;
struct UIMessage;
class UITabbedPane;
struct UIRect;

#include "UIString.h"

class UIEventCallback
{
public:

	// Generic message interception.  Return false to prevent
	// the control from processing the message
	virtual bool OnMessage( UIWidget *Context, const UIMessage & msg );	

	virtual void OnShow( UIWidget *Context );
	virtual void OnHide( UIWidget *Context );

	virtual void OnHoverIn( UIWidget *Context );
	virtual void OnHoverOut( UIWidget *Context );

	virtual void OnEnable( UIWidget *Context );
	virtual void OnDisable( UIWidget *Context );

	virtual void OnActivate( UIWidget *Context );
	virtual void OnDeactivate( UIWidget *Context );

	virtual void OnDrop( UIWidget *Context );

	virtual void OnWidgetDataChanged (const UIWidget * context, const UINarrowString & property, const UIString & value);

	// Control specific notifications.
	virtual void OnButtonPressed( UIWidget *Context );

	virtual void OnCheckboxSet( UIWidget *Context );
	virtual void OnCheckboxUnset( UIWidget *Context );

	virtual void OnDropdownboxSelectionChanged( UIWidget *Context );

	virtual void OnListboxSelectionChanged( UIWidget *Context );
	virtual void OnListboxDataSourceChanged( UIWidget *Context );
	virtual void OnListboxDoubleClicked( UIWidget *Context );

	virtual void OnSliderbarChanged( UIWidget *Context );

	virtual void OnSliderplaneChanged( UIWidget *Context );

	virtual void OnTextboxChanged( UIWidget *Context );
	virtual void OnTextboxOverflow( UIWidget *Context );

	virtual void OnTabbedPaneChanged (UIWidget * Context);

	virtual void OnPopupMenuSelection (UIWidget * context);

	virtual void OnVolumePageSelectionChanged (UIWidget * context);

	virtual void OnWidgetRectChanging (UIWidget * context, UIRect & targetRect);

	virtual void OnWidgetRectChanged  (UIWidget * context);

	virtual void OnGenericSelectionChanged (UIWidget * context);

	virtual void OnRunScript (UIWidget * context);

	virtual void OnSizeChanged(UIWidget * context);

	virtual void OnTreeRowExpansionToggled(UIWidget * context, int row);

	virtual ~UIEventCallback () = 0;

	//-- this must be implemented to shut up stupid lint warnings
	UIEventCallback () {}
};

#endif // __UIEVENTCALLBACK_H__
