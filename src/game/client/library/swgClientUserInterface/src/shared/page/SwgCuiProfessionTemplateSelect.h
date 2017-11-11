//======================================================================
//
// SwgCuiProfessionTemplateSelect.h
// copyrizzite (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiProfessionTemplateSelect_H
#define INCLUDED_SwgCuiProfessionTemplateSelect_H

//======================================================================

#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"

//----------------------------------------------------------------------

class PlayerObject;
class Timer;

class UIButton;
class UIEffector;
class UIPage;
class UIText;
class UIVolumePage;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiProfessionTemplateSelect : 
public CuiMediator,
public UIEventCallback
{ 
public:
	explicit SwgCuiProfessionTemplateSelect(UIPage & page);
	
	static SwgCuiProfessionTemplateSelect * createInto(UIPage & parent);

	void OnButtonPressed(UIWidget * context);
	void update(float deltaTimeSecs);

	void allowCancel(bool cancel);
	
protected:
	void performActivate();
	void performDeactivate();

	
private:
	~SwgCuiProfessionTemplateSelect();
	 SwgCuiProfessionTemplateSelect(const SwgCuiProfessionTemplateSelect & rhs);
	 SwgCuiProfessionTemplateSelect & operator=(const SwgCuiProfessionTemplateSelect & rhs);

	 void buildProfessionButtons();
	 void destroyProfessionButtons();
	 void setProfessionTemplate(UIString const & templateName);
	 void setProfessionSubTemplate(UIString const & templateName);
	 void setButtonSelected();
	 void volumePageFixup();

private:

	class Implementation;
	Implementation * m_pimpl;
};

//======================================================================

#endif
