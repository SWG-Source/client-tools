//======================================================================
//
// SwgCuiAvatarSummary.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAvatarSummary_H
#define INCLUDED_SwgCuiAvatarSummary_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CuiCreatureSkillsList;
class CuiWidget3dObjectListViewer;
class UIButton;
class UIPage;
class UITabbedPane;
class UIText;
class UITextbox;
class UICheckbox;

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiAvatarSummary :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                 SwgCuiAvatarSummary  (UIPage & page);

	void                     OnButtonPressed      (UIWidget *context);

	void                     performActivate      ();
	void                     performDeactivate    ();

	void                     update               (float deltaTimeSecs);

	void                     onCreationFinished   (bool b);
	void                     onRandomNameChanged  (const Unicode::String & name);
	void                     onCreationAborted    (bool b);

private:
	virtual                 ~SwgCuiAvatarSummary ();
	                         SwgCuiAvatarSummary ();
	                         SwgCuiAvatarSummary (const SwgCuiAvatarSummary & rhs);
	SwgCuiAvatarSummary &    operator=           (const SwgCuiAvatarSummary & rhs);

private:

	UIButton *                    m_buttonBack;
	UIButton *                    m_buttonNext;

	UIButton *                    m_buttonRandomName;

	UITextbox *                   m_textboxName;
	UITextbox *                   m_textboxSurname;

	UIText *                      m_textBio;

	CuiWidget3dObjectListViewer * m_viewer;

	CuiCreatureSkillsList       * m_skills;

	MessageDispatch::Callback *   m_callback;
	
	bool                          m_pendingCreationFinished;

	bool                          m_nameModified;

	typedef stdvector<std::string>::fwd StringVector;
	StringVector *                m_oneNameTemplateVector;
	bool                          m_aborted;

	UITabbedPane *                m_tabs;

	UICheckbox *                  m_checkNewbieTutorial;
};

//======================================================================

#endif
