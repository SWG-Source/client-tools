//======================================================================
//
// SwgCuiCommandBrowser.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCommandBrowser_H
#define INCLUDED_SwgCuiCommandBrowser_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
};

class CreatureObject;
class CuiDragInfo;
class PlayerObject;
class UIButton;
class UIPage;
class UITabbedPane;
class UIText;
class UIVolumePage;
class UIImageStyle;

//----------------------------------------------------------------------


class SwgCuiCommandBrowser :
public CuiMediator,
public UIEventCallback
{
public:

	enum TabType
	{
		TT_none,
		TT_combat,
		TT_social,
		TT_mood,
		TT_space,
		TT_astromech,
		TT_chatter,
		TT_macro,
		TT_costume,
		TT_other
	};

	explicit                      SwgCuiCommandBrowser (UIPage & page);

	void                          performActivate   ();
	void                          performDeactivate ();

	void                          OnTabbedPaneChanged (UIWidget * context);
	bool                          OnMessage           (UIWidget * context, const UIMessage & msg);

	void                          OnVolumePageSelectionChanged (UIWidget * context);
	void                          OnButtonPressed           (UIWidget *context);

	void                          onCommandsChanged (const CreatureObject & creature);
	void                          onCommandAdded    (const std::pair<CreatureObject *, std::string> & msg);
	void                          onCommandRemoved  (const std::pair<CreatureObject *, std::string> & msg);
	void                          onMacroAdded      (const std::string & name);
	void                          onMacroRemoved    (const std::string & name);
	void                          onMacroModified   (const std::string & name);
	void                          onMacrosReset     (const bool & );

	void                          reset  ();
	void                          update ();

	void                          selectCommand     (const std::string & cmd);
	void                          selectString      (const std::string & str);
	void                          selectMacro       (const std::string & macro);

	static UIPage *                      selectEntry       (const std::string & cmd, const std::string & str, UIVolumePage & vol);

private:
	                             ~SwgCuiCommandBrowser ();
	                              SwgCuiCommandBrowser (const SwgCuiCommandBrowser & rhs);
	SwgCuiCommandBrowser &         operator= (const SwgCuiCommandBrowser & rhs);

	UIPage *                      createCommandPage  (const std::string & cmd, const CuiDragInfo & dragInfo, UIPage * displayPage, UIVolumePage & vol, UIPage * & actualInsertBefore, UIImageStyle* iconStyle) const;
	UIPage *                      findInsertionPoint (UIVolumePage const & vol, const Unicode::String & displayText, bool & duplicate) const;
	void                          addCommand         (const std::string & cmd, UIPage * const insertBefore);
	static void                   clearCommands      (UIVolumePage & vol);
	void                          addSocial          (const std::string & cmd, uint32 id);
	void                          addChatterSpeech   (const std::string & cmd);
	void                          addMood            (const std::string & cmd, const Unicode::String & displayName, uint32 id);
	void                          addMacro           (const std::string & name, const std::string & userDefinedName, const std::string & text, const std::string& imageStyle, const UIColor & color, UIPage * insertBefore);
	UIPage *                      addEntry           (const std::string & cmd, const CuiDragInfo & dragInfo, UIPage * const insertBefore, UIVolumePage & vol, UIImageStyle* iconStyle) const;
	void                          removeEntry        (const std::string & cmd, UIVolumePage & vol);
//	bool                          loadMacros         ();
//	bool                          saveMacros         ();

	MessageDispatch::Callback *  m_callback;

	UIButton*                    m_newMacroButton;
	UIButton*                    m_editMacroButton;
	UIButton*                    m_deleteMacroButton;

	UIVolumePage *               m_volume;
	UIVolumePage *               m_macroVolume;
	UITabbedPane *               m_tabs;
	UIText *                     m_textName;
	UIText *                     m_textDesc;

	UIPage *                     m_sample;

	UIPage *                     m_pageInfo;

	TabType                      m_tabType;

	typedef stdmap<UIPage*, CuiDragInfo>::fwd             PageMap;
	uint32                       m_displayGroupSpace;
};

//======================================================================

#endif

