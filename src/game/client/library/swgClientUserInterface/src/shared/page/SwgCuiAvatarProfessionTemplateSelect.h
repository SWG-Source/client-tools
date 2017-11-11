//======================================================================
//
// SwgCuiAvatarProfessionTemplateSelect.h
// copyrizzite (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAvatarProfessionTemplateSelect_H
#define INCLUDED_SwgCuiAvatarProfessionTemplateSelect_H

//======================================================================

#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"

//----------------------------------------------------------------------

class PlayerObject;
class Timer;

class CuiMessageBox;
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

class SwgCuiAvatarProfessionTemplateSelect : 
public CuiMediator,
public UIEventCallback
{ 
public:
	explicit SwgCuiAvatarProfessionTemplateSelect(UIPage & page);
	
	static SwgCuiAvatarProfessionTemplateSelect * createInto(UIPage & parent);

	void OnButtonPressed(UIWidget * context);
	void update(float deltaTimeSecs);
	void onCreationFinished(bool b);
	void onCreationAborted(bool b);

protected:
	void performActivate();
	void performDeactivate();

private:
	~SwgCuiAvatarProfessionTemplateSelect();
	SwgCuiAvatarProfessionTemplateSelect(const SwgCuiAvatarProfessionTemplateSelect & rhs);
	SwgCuiAvatarProfessionTemplateSelect & operator=(const SwgCuiAvatarProfessionTemplateSelect & rhs);

	void buildProfessionButtons();
	void destroyProfessionButtons();
	void setProfessionTemplate(UIString const & templateName);
	void setProfessionSubTemplate(UIString const & templateName);
	void setButtonSelected();
	void volumePageFixup();
	void finishAndCreateCharacter();
	void returnToPreviousScreen(bool const dueToError);

private:

	class Implementation;
	Implementation * m_pimpl;

	UIButton * m_buttonBack;
	UIButton * m_buttonNext;
	UIButton * m_buttonPreview;

	MessageDispatch::Callback * m_callback;
	bool m_pendingCreationFinished;
	bool m_aborted;
	bool m_professionWasSelected;
};

//======================================================================

#endif
