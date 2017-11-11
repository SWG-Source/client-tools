// ======================================================================
//
// CuiDataDrivenPageListener.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiDataDrivenPageListener_H
#define INCLUDED_CuiDataDrivenPageListener_H

#include "UIEventCallback.h"

class CuiDataDrivenPage;
class UIButton;
class UIWidget;
class CuiMediator;

class CuiDataDrivenPageListener : 
public UIEventCallback
{
public:
	
	CuiDataDrivenPageListener(CuiDataDrivenPage* owner, UIButton *okButton, UIButton *cancelButton, UIButton *closeButton);
	virtual ~CuiDataDrivenPageListener();

	virtual void OnShow                     ( UIWidget *Context );
	virtual void OnHide                     ( UIWidget *Context );

	virtual void OnEnable                   ( UIWidget *Context );
	virtual void OnDisable                  ( UIWidget *Context );

	virtual void OnButtonPressed            ( UIWidget *Context );

	virtual void OnCheckboxSet              ( UIWidget *Context );
	virtual void OnCheckboxUnset            ( UIWidget *Context );

	virtual void OnSliderbarChanged         ( UIWidget *Context );

	virtual void OnTabbedPaneChanged        (UIWidget * Context);

	virtual void OnVolumePageSelectionChanged (UIWidget * context);

	virtual void OnGenericSelectionChanged  (UIWidget * context);


private:
	CuiDataDrivenPage*                   m_owner;
	
	UIButton*                            m_okButton;
	UIButton*                            m_cancelButton;
	UIButton*                            m_closeButton;
};


#endif
