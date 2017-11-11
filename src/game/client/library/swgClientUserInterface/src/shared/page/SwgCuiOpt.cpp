//======================================================================
//
// SwgCuiOpt.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOpt.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "swgClientUserInterface/SwgCuiOptAfk.h"
#include "swgClientUserInterface/SwgCuiOptAlarm.h"
#include "swgClientUserInterface/SwgCuiOptChat.h"
#include "swgClientUserInterface/SwgCuiOptChatColors.h"
#include "swgClientUserInterface/SwgCuiOptCombat.h"
#include "swgClientUserInterface/SwgCuiOptControls.h"
#include "swgClientUserInterface/SwgCuiOptGraphics.h"
#include "swgClientUserInterface/SwgCuiOptKeymap.h"
#include "swgClientUserInterface/SwgCuiOptMisc.h"
#include "swgClientUserInterface/SwgCuiOptSound.h"
#include "swgClientUserInterface/SwgCuiOptTerrain.h"
#include "swgClientUserInterface/SwgCuiOptUi.h"
#include "swgClientUserInterface/SwgCuiOptVoice.h"
#include <map>
#include <vector>

//======================================================================

namespace SwgCuiOptNamespace
{
	enum OptionTabs
	{
		OT_afk,
		OT_alarm,
		OT_chat,
		OT_chatColors,
		OT_combat,
		OT_controls,
		OT_graphics,
		OT_misc,
		OT_sound,
		OT_terrain,
		OT_ui,
		OT_keymap,
		OT_voice,
		OT_numTabs
	};

	SwgCuiOptBase *s_swgCuiOpt = NULL;
}

using namespace SwgCuiOptNamespace;

//----------------------------------------------------------------------

SwgCuiOpt::SwgCuiOpt (UIPage & page) :
CuiMediator ("SwgCuiOpt", page),
UIEventCallback  (),
m_optionPages    (new BaseMap),
m_tabs           (0),
m_buttonOk       (0),
m_buttonCancel   (0),
m_keepSettings   (false)
{
	Game::setGameOptionChangedCallback(&gameOptionChanged);

	getCodeDataObject (TUITabbedPane, m_tabs,      "tabs");

	UIPage * optionPage = 0;
	getCodeDataObject (TUIPage, optionPage,      "pageAfk");
	(*m_optionPages) [OT_afk] = new SwgCuiOptAfk        (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageAlarm");
	(*m_optionPages) [OT_alarm] = new SwgCuiOptAlarm      (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageChat");
	(*m_optionPages) [OT_chat] = new SwgCuiOptChat       (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageChatColors");
	(*m_optionPages) [OT_chatColors] = new SwgCuiOptChatColors (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageCombat");
	(*m_optionPages) [OT_combat] = new SwgCuiOptCombat     (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageControls");
	(*m_optionPages) [OT_controls] = new SwgCuiOptControls   (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageGraphics");
	(*m_optionPages) [OT_graphics] = new SwgCuiOptGraphics   (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageMisc");
	(*m_optionPages) [OT_misc] = new SwgCuiOptMisc       (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageSound");
	(*m_optionPages) [OT_sound] = new SwgCuiOptSound      (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageTerrain");
	(*m_optionPages) [OT_terrain] = new SwgCuiOptTerrain    (*optionPage);

	getCodeDataObject (TUIPage, optionPage,      "pageUi");
	(*m_optionPages) [OT_ui] = new SwgCuiOptUi         (*optionPage);

	getCodeDataObject (TUIPage, optionPage, "pageVoice");
	(*m_optionPages) [OT_voice] = new SwgCuiOptVoice (*optionPage);

	getCodeDataObject (TUIPage, optionPage, "pageKeymap");
	(*m_optionPages) [OT_keymap] = new SwgCuiOptKeymap (*optionPage, Game::getHudSceneType());

	UIButton * buttonDefaultPage = 0;
	getCodeDataObject (TUIButton, buttonDefaultPage,      "buttonDefaultPage");
	buttonDefaultPage->Press ();

	for (BaseMap::iterator it = m_optionPages->begin (); it != m_optionPages->end (); ++it)
	{
		SwgCuiOptBase * const base = (*it).second;
		if(base)
		{		
			base->fetch ();
			base->getPage ().SetEnabled (true);
			base->getPage ().SetVisible (false);
		}
	}

	getCodeDataObject (TUIButton, m_buttonOk,     "buttonOk");
	getCodeDataObject (TUIButton, m_buttonCancel, "buttonCancel");

	registerMediatorObject (*m_buttonOk,      true);
	registerMediatorObject (*m_buttonCancel,  true);
	registerMediatorObject (*m_tabs,          true);

	setState (MS_closeable);
	setState (MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiOpt::~SwgCuiOpt ()
{
	s_swgCuiOpt = NULL;

	for (BaseMap::iterator it = m_optionPages->begin (); it != m_optionPages->end (); ++it)
	{
		SwgCuiOptBase * const base = (*it).second;
		if(base)
			base->release ();
	}

	delete m_optionPages;
	m_optionPages = 0;
}

//----------------------------------------------------------------------

void SwgCuiOpt::gameOptionChanged()
{
	if (s_swgCuiOpt != NULL)
	{
		s_swgCuiOpt->queryWidgetValues();
	}
}

//----------------------------------------------------------------------

void SwgCuiOpt::performActivate   ()
{
	for (BaseMap::iterator it = m_optionPages->begin (); it != m_optionPages->end (); ++it)
	{
		SwgCuiOptBase * const base = (*it).second;
		if(base)
			base->storeRevertData ();
	}

	m_keepSettings = true;
	CuiManager::requestPointer (true);
	OnTabbedPaneChanged (m_tabs);
}

//----------------------------------------------------------------------

void SwgCuiOpt::performDeactivate ()
{
	CuiManager::requestPointer (false);

	for (BaseMap::iterator it = m_optionPages->begin (); it != m_optionPages->end (); ++it)
	{
		SwgCuiOptBase * const base = (*it).second;
		if(base)
			base->deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiOpt::OnButtonPressed   (UIWidget * context)
{
	if (context == m_buttonOk)
	{
		closeThroughWorkspace ();
	}
	else if (context == m_buttonCancel)
	{
		m_keepSettings = false;
		closeThroughWorkspace ();
	}
}

//----------------------------------------------------------------------

bool SwgCuiOpt::close()
{
	deactivate();

	if (m_keepSettings)
	{
		CuiSettings::save();
		CurrentUserOptionManager::save ();
		LocalMachineOptionManager::save ();
	}
	else
	{
		for (BaseMap::iterator it = m_optionPages->begin (); it != m_optionPages->end (); ++it)
		{
			SwgCuiOptBase * const base = (*it).second;
			if(base)
				base->revert ();
		}
	}

	for (BaseMap::iterator it = m_optionPages->begin (); it != m_optionPages->end (); ++it)
	{
		SwgCuiOptBase * const base = (*it).second;
		if(base)
			base->close ();
	}

	return CuiMediator::close ();
}

//----------------------------------------------------------------------

void SwgCuiOpt::OnTabbedPaneChanged (UIWidget * context)
{
	if (context == m_tabs)
	{	
		const UIWidget * const activeWidget = m_tabs->GetActiveWidget ();

		for (BaseMap::iterator it = m_optionPages->begin (); it != m_optionPages->end (); ++it)
		{
			SwgCuiOptBase * const base = (*it).second;

			if(base)
			{				
				if (&base->getPage () == activeWidget)
				{
					base->activate ();
					s_swgCuiOpt = (*it).second;
				}
				else
				{
					base->deactivate ();
				}
			}
		}
	}
}

//======================================================================
