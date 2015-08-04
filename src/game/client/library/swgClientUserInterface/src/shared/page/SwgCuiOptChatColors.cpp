//======================================================================
//
// SwgCuiOptChatColors.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptChatColors.h"

#include "clientGame/ClientTextManager.h"
#include "clientUserInterface/CuiColorPicker.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/TextIterator.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "StringId.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITextbox.h"

//======================================================================

SwgCuiOptChatColors::SwgCuiOptChatColors (UIPage & page) :
SwgCuiOptBase("SwgCuiOptChatColors", page),
m_addWordButton(NULL),
m_addSentenceButton(NULL),
m_removeButton(NULL),
m_resetDefaultsButton(NULL),
m_gameTextList(NULL),
m_wordList(NULL),
m_sentenceList(NULL),
m_newEntryTextBox(NULL),
m_palette(NULL),
m_colorPicker(NULL),
m_callBack(new MessageDispatch::Callback)
{
	getCodeDataObject(TUIButton, m_resetDefaultsButton, "buttonDefaults");
	registerMediatorObject(*m_resetDefaultsButton, true);

	getCodeDataObject(TUIList, m_gameTextList, "listFight");
	registerMediatorObject(*m_gameTextList, true);

	getCodeDataObject(TUIButton, m_addWordButton, "buttonHighlightWord");
	registerMediatorObject(*m_addWordButton, true);

	getCodeDataObject(TUIButton, m_addSentenceButton, "buttonHighlightSentence");
	registerMediatorObject(*m_addSentenceButton, true);

	getCodeDataObject(TUIButton, m_removeButton, "buttonHighlightRemove");
	registerMediatorObject(*m_removeButton, true);
	m_removeButton->SetEnabled(false);

	getCodeDataObject(TUIList, m_wordList, "listWords");
	registerMediatorObject(*m_wordList, true);
	m_wordList->Clear();

	getCodeDataObject(TUIList, m_sentenceList, "listSentences");
	registerMediatorObject(*m_sentenceList, true);
	m_sentenceList->Clear();

	getCodeDataObject(TUITextbox, m_newEntryTextBox, "texthighlight");
	registerMediatorObject(*m_newEntryTextBox, true);

	getCodeDataObject(TUIPage, m_colorPickerPage, "colorPicker");
	registerMediatorObject(*m_colorPickerPage, true);

	m_colorPickerPage->SetProperty(CuiColorPicker::Properties::AutoSizePaletteCells, Unicode::narrowToWide ("true"));

	m_palette = PaletteArgbList::fetch(TemporaryCrcString("palette/ui.pal", true));
	m_colorPicker = new CuiColorPicker(*m_colorPickerPage);
	m_colorPicker->fetch();
	m_colorPicker->setText(Unicode::emptyString);
	m_colorPicker->setPalette(m_palette);
	m_colorPicker->setForceColumns(16);
	//registerMediatorObject(*m_colorPicker, true);

	// Game text colors

	m_gameTextList->Clear();
	ClientTextManager::UnicodeStringList coloredGameText;
	ClientTextManager::getColoredGameText(coloredGameText);
	ClientTextManager::UnicodeStringList::const_iterator iterColoredGameText = coloredGameText.begin();

	for (; iterColoredGameText != coloredGameText.end(); ++iterColoredGameText)
	{
		Unicode::String const &text = (*iterColoredGameText);

		m_gameTextList->AddRow(text, "");
	}

	// Word colors

	m_wordList->Clear();
	ClientTextManager::UnicodeStringList coloredWords;
	ClientTextManager::getColoredWords(coloredWords);
	ClientTextManager::UnicodeStringList::const_iterator iterColoredWords = coloredWords.begin();

	for (; iterColoredWords != coloredWords.end(); ++iterColoredWords)
	{
		Unicode::String const &text = (*iterColoredWords);

		m_wordList->AddRow(text, "");
	}

	// Sentence colors

	m_sentenceList->Clear();
	ClientTextManager::UnicodeStringList coloredSentences;
	ClientTextManager::getColoredSentences(coloredSentences);
	ClientTextManager::UnicodeStringList::const_iterator iterColoredSentences = coloredSentences.begin();

	for (; iterColoredSentences != coloredSentences.end(); ++iterColoredSentences)
	{
		Unicode::String const &text = (*iterColoredSentences);

		m_sentenceList->AddRow(text, "");
	}
}

//----------------------------------------------------------------------

SwgCuiOptChatColors::~SwgCuiOptChatColors ()
{
	delete m_callBack;
	m_callBack = NULL;

	if (m_palette != NULL)
	{
		m_palette->release();
		m_palette = NULL;
	}

	m_colorPicker->release();
	m_colorPicker = NULL;
	m_addWordButton = NULL;
	m_addSentenceButton = NULL;
	m_removeButton = NULL;
	m_gameTextList = NULL;
	m_wordList = NULL;
	m_sentenceList = NULL;
	m_newEntryTextBox = NULL;
	m_resetDefaultsButton = NULL;
}

//----------------------------------------------------------------------

void SwgCuiOptChatColors::performActivate()
{
	SwgCuiOptBase::performActivate();
	m_colorPicker->activate();
	setIsUpdating(true);

	OnTextboxChanged(m_newEntryTextBox);
}

//-----------------------------------------------------------------------------

void SwgCuiOptChatColors::performDeactivate()
{
	SwgCuiOptBase::performDeactivate ();
	m_colorPicker->deactivate();
	setIsUpdating(false);

	ClientTextManager::saveUserSettings();
}

//-----------------------------------------------------------------------------

bool SwgCuiOptChatColors::OnMessage(UIWidget *context, const UIMessage &message)
{
	bool result = true;

	if ((message.Keystroke == UIMessage::Enter) &&
	         (message.Type == UIMessage::KeyDown))
	{
		// Don't let the 'Enter' key close this window

		result = false;
	}
	else if ((context == m_newEntryTextBox) &&
	         message.IsMouseDownCommand())
	{
		m_gameTextList->SelectRow(-1);
		m_wordList->SelectRow(-1);
		m_sentenceList->SelectRow(-1);
		m_removeButton->SetEnabled(false);
	}
	else
	{
		result = SwgCuiOptBase::OnMessage(context, message);
	}

	return result;
}

//-----------------------------------------------------------------------------

void SwgCuiOptChatColors::OnGenericSelectionChanged(UIWidget *context)
{
	if (context == m_wordList)
	{
		if (m_wordList->GetLastSelectedRow() != -1)
		{
			m_removeButton->SetEnabled(m_wordList->GetLastSelectedRow() != -1);
			m_gameTextList->SelectRow(-1);
			m_sentenceList->SelectRow(-1);
		}
	}
	else if (context == m_sentenceList)
	{
		if (m_sentenceList->GetLastSelectedRow() != -1)
		{
			m_removeButton->SetEnabled(m_sentenceList->GetLastSelectedRow() != -1);
			m_gameTextList->SelectRow(-1);
			m_wordList->SelectRow(-1);
		}
	}
	else if (context == m_gameTextList)
	{
		if (m_gameTextList->GetLastSelectedRow() != -1)
		{
			m_removeButton->SetEnabled(false);
			m_wordList->SelectRow(-1);
			m_sentenceList->SelectRow(-1);
		}
	}
}

//-----------------------------------------------------------------------------

void SwgCuiOptChatColors::OnTextboxChanged(UIWidget *context)
{
	if (context == m_newEntryTextBox)
	{
		bool const enableButtons = !m_newEntryTextBox->GetLocalText().empty();

		m_addWordButton->SetEnabled(enableButtons);
		m_addSentenceButton->SetEnabled(enableButtons);
	}
}

//-----------------------------------------------------------------------------

void SwgCuiOptChatColors::updateColor()
{
	bool error = false;
	PackedArgb packedArgb(m_palette->getEntry(m_colorPicker->getValue(), error));
	WARNING(error, ("SwgCuiOptChatColors error getting color"));

	PackedRgb packedRgb(packedArgb.getR(), packedArgb.getG(), packedArgb.getB());

	if (m_wordList->GetLastSelectedRow() != -1)
	{
		int const lastSelectedRow = m_wordList->GetLastSelectedRow();
		Unicode::String text;
		m_wordList->GetLocalText(lastSelectedRow, text);

		TextIterator textIterator(text);
		Unicode::String printableText(textIterator.getPrintableText());

		m_wordList->SetLocalText(lastSelectedRow, ClientTextManager::getColorCode(packedRgb) + printableText);
		m_wordList->SelectRow(lastSelectedRow);

		ClientTextManager::setColoredWord(printableText, packedRgb);
	}
	else if (m_sentenceList->GetLastSelectedRow() != -1)
	{
		int const lastSelectedRow = m_sentenceList->GetLastSelectedRow();
		Unicode::String text;
		m_sentenceList->GetLocalText(lastSelectedRow, text);

		TextIterator textIterator(text);
		Unicode::String printableText(textIterator.getPrintableText());

		m_sentenceList->SetLocalText(lastSelectedRow, ClientTextManager::getColorCode(packedRgb) + printableText);
		m_sentenceList->SelectRow(lastSelectedRow);

		ClientTextManager::setColoredSentence(printableText, packedRgb);
	}
	else if (m_gameTextList->GetLastSelectedRow() != -1)
	{
		int const lastSelectedRow = m_gameTextList->GetLastSelectedRow();
		Unicode::String text;
		m_gameTextList->GetLocalText(lastSelectedRow, text);

		TextIterator textIterator(text);
		Unicode::String printableText(textIterator.getPrintableText());

		m_gameTextList->SetLocalText(lastSelectedRow, ClientTextManager::getColorCode(packedRgb) + printableText);
		m_gameTextList->SelectRow(lastSelectedRow);

		ClientTextManager::setColoredGameText(printableText, packedRgb);
	}
}

//-----------------------------------------------------------------------------

void SwgCuiOptChatColors::updateColor(UIList &list, Unicode::String const &text, PackedRgb const &packedRgb)
{
	for (int row = 0; row < list.GetRowCount(); ++row)
	{
		Unicode::String localText;
		m_wordList->GetLocalText(row, localText);

		if (TextIterator(localText).getPrintableText() == text)
		{
			m_wordList->SetLocalText(row, ClientTextManager::getColorCode(packedRgb) + text);
			m_wordList->SelectRow(row);
			break;
		}
	}
}

//-----------------------------------------------------------------------------

void SwgCuiOptChatColors::OnButtonPressed(UIWidget *context)
{
	bool error = false;
	PackedArgb packedArgb(m_palette->getEntry(m_colorPicker->getValue(), error));
	WARNING(error, ("SwgCuiOptChatColors error"));
	PackedRgb packedRgb(packedArgb.getR(), packedArgb.getG(), packedArgb.getB());

	if (context == m_addWordButton)
	{
		if (!m_newEntryTextBox->GetLocalText().empty())
		{
			if (ClientTextManager::setColoredWord(m_newEntryTextBox->GetLocalText(), packedRgb))
			{
				m_wordList->AddRow(ClientTextManager::getColorCode(packedRgb) + m_newEntryTextBox->GetLocalText(), "");
			}
			else
			{
				// The text was already in the list just the color is changing

				updateColor(*m_wordList, m_newEntryTextBox->GetLocalText(), packedRgb);
			}

			m_newEntryTextBox->SetLocalText(Unicode::emptyString);
			OnTextboxChanged(m_newEntryTextBox);
		}
	}
	else if (context == m_addSentenceButton)
	{
		if (!m_newEntryTextBox->GetLocalText().empty())
		{
			if (ClientTextManager::setColoredSentence(m_newEntryTextBox->GetLocalText(), packedRgb))
			{
				m_sentenceList->AddRow(ClientTextManager::getColorCode(packedRgb) + m_newEntryTextBox->GetLocalText(), "");
			}
			else
			{
				// The text was already in the list just the color is changing

				updateColor(*m_sentenceList, m_newEntryTextBox->GetLocalText(), packedRgb);
			}

			m_newEntryTextBox->SetLocalText(Unicode::emptyString);
			OnTextboxChanged(m_newEntryTextBox);
		}
	}
	else if (context == m_removeButton)
	{
		if (m_wordList->GetLastSelectedRow() != -1)
		{
			// Remove the word entry

			Unicode::String text;
			m_wordList->GetLocalText(m_wordList->GetLastSelectedRow(), text);

			TextIterator textIterator(text);

			ClientTextManager::removeColoredWord(textIterator.getPrintableText());
			m_wordList->RemoveRow(m_wordList->GetLastSelectedRow());
		}
		else if (m_sentenceList->GetLastSelectedRow() != -1)
		{
			// Remove the sentence entry

			Unicode::String text;
			m_sentenceList->GetLocalText(m_sentenceList->GetLastSelectedRow(), text);

			TextIterator textIterator(text);

			ClientTextManager::removeColoredSentence(textIterator.getPrintableText());
			m_sentenceList->RemoveRow(m_sentenceList->GetLastSelectedRow());
		}
	}
	else if (context == m_resetDefaultsButton)
	{
		StringId s("ui_opt", "confirm_reset_default_chat_color");
		CuiMessageBox *messageBox = CuiMessageBox::createYesNoBox(s.localize());
		m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiOptChatColors::onConfirmResetDefaults);
	}
}
//-----------------------------------------------------------------
void SwgCuiOptChatColors::onConfirmResetDefaults(const CuiMessageBox &messageBox)
{
	if (messageBox.completedAffirmative())
	{
		m_wordList->Clear();
		ClientTextManager::UnicodeStringList coloredWord;
		ClientTextManager::getColoredWords(coloredWord);
		ClientTextManager::UnicodeStringList::const_iterator iterColoredWord = coloredWord.begin();
		for(; iterColoredWord != coloredWord.end(); ++iterColoredWord)
		{
			ClientTextManager::removeColoredWord(*iterColoredWord);
		}

		m_sentenceList->Clear();
		ClientTextManager::UnicodeStringList coloredSentences;
		ClientTextManager::getColoredSentences(coloredSentences);
		ClientTextManager::UnicodeStringList::const_iterator iterColoredSentences = coloredSentences.begin();

		for (; iterColoredSentences != coloredSentences.end(); ++iterColoredSentences)
		{
			ClientTextManager::removeColoredSentence(*iterColoredSentences);
		}

		ClientTextManager::resetColoredGameText();

		m_gameTextList->Clear();
		ClientTextManager::UnicodeStringList coloredGameText;
		ClientTextManager::getColoredGameText(coloredGameText);
		ClientTextManager::UnicodeStringList::const_iterator iterColoredGameText = coloredGameText.begin();

		for (; iterColoredGameText != coloredGameText.end(); ++iterColoredGameText)
		{
			Unicode::String const &text = (*iterColoredGameText);

			m_gameTextList->AddRow(text, "");
		}

		ClientTextManager::saveUserSettings();
	}
}
//-----------------------------------------------------------------------------

void SwgCuiOptChatColors::update(float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_colorPicker->checkAndResetUSerChanged())
	{
		updateColor();
	}

	// This is a hack because the color picker should be getting its own update

	m_colorPicker->update(deltaTimeSecs);
}

//======================================================================
