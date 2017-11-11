//======================================================================
//
// SwgCuiOptChatColors.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptChatColors_H
#define INCLUDED_SwgCuiOptChatColors_H

#include "swgClientUserInterface/SwgCuiOptBase.h"
#include "UIEventCallback.h"

class CuiColorPicker;
class PackedRgb;
class PaletteArgb;
class UIButton;
class UIList;
class UIPage;
class UITextbox;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//======================================================================

class SwgCuiOptChatColors : 
public SwgCuiOptBase
{
public:

	explicit SwgCuiOptChatColors (UIPage & page);

	virtual void OnButtonPressed(UIWidget *context);
	virtual void OnGenericSelectionChanged(UIWidget *context);
	virtual bool OnMessage(UIWidget *context, const UIMessage &msg);
	virtual void OnTextboxChanged(UIWidget *context);

	virtual void update(float deltaTime);

	void               onConfirmResetDefaults(const CuiMessageBox &messageBox);

protected:

	virtual void performActivate();
	virtual void performDeactivate();

private:

	void               updateColor();
	void               updateColor(UIList &list, Unicode::String const &text, PackedRgb const &packedRgb);


	UIButton *         m_addWordButton;
	UIButton *         m_addSentenceButton;
	UIButton *         m_removeButton;
	UIButton *         m_resetDefaultsButton;
	UIList *           m_gameTextList;
	UIList *           m_wordList;
	UIList *           m_sentenceList;
	UITextbox *        m_newEntryTextBox;
	PaletteArgb const *m_palette;
	UIPage *           m_colorPickerPage;
	CuiColorPicker *   m_colorPicker;
	MessageDispatch::Callback * m_callBack;

private:

	~SwgCuiOptChatColors ();
	SwgCuiOptChatColors & operator=(const SwgCuiOptChatColors & rhs);
	SwgCuiOptChatColors            (const SwgCuiOptChatColors & rhs);
};

//======================================================================

#endif
