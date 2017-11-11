// ============================================================================
//
// ClientTextManager.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientTextManager.h"

#include "clientGame/Game.h"
#include "LocalizationManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"
#include "sharedGame/TextIterator.h"
#include "sharedGame/TextManager.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"

#include <map>

// ============================================================================
//
// ClientTextManagerNamespace
//
// ============================================================================

// ----------------------------------------------------------------------------
namespace ClientTextManagerNamespace
{
	typedef std::vector<Unicode::String> UnicodeStringVector;

	class ColorCode
	{
	public:

		ColorCode();

		char m_code[9];
	};

	class ColorCodedText
	{
	public:

		ColorCodedText();

		void setColorCode(ColorCode const &colorCode);
		void setText(Unicode::String const &text);

		Unicode::String const &getText() const;
		Unicode::String        getColorCodeUnicodeString() const;
		std::string            getColorCodeString() const;

	private:

		Unicode::String m_text;
		ColorCode       m_colorCode;
	};

	typedef std::map<Unicode::String, ColorCodedText>              ColoredTextList;
	typedef std::map<ClientTextManager::TextType, Unicode::String> DefaultGameTextColors;
	typedef std::map<Unicode::String, std::string>                 DefaultGameTextColorStringIds;

	DefaultGameTextColors         s_defaultGameTextColors;
	DefaultGameTextColorStringIds s_defaultGameTextColorStringIds;
	ColoredTextList               s_coloredWords;
	ColoredTextList               s_coloredSentences;
	ColoredTextList               s_coloredGameText;
	bool                          s_installed = false;
	ColorCode                     s_whiteColorCode;
	bool                          s_dataChanged = false;
	Tag const                     s_tag = TAG(C,C,L,R);
	Unicode::String               s_playerName;
	Unicode::String               s_resetColorCode;


	void        getDefaultTextColorFromDataTable(char const *text, ClientTextManager::TextType const textType, DataTable const &dataTable);
	std::string getColoredGameTextStringIdFromString(Unicode::String colorName);
	bool        setColoredText(ColoredTextList &coloredTextList, Unicode::String const &text, PackedRgb const &packedRgb, bool addColor);
	bool        setColoredText(ColoredTextList &coloredTextList, Unicode::String const &text, std::string const &colorCodeText, bool addColor);
	void        removeColoredText(ColoredTextList &coloredText, Unicode::String const &text);
	void        getColorCode(PackedRgb const &packedRgb, ColorCode &colorCode);
	void        getColoredText(ColoredTextList const &coloredTextList, ClientTextManager::UnicodeStringList &unicodeStringList);
	std::string getFileName();
	void        save();
	void        load();
}

using namespace ClientTextManagerNamespace;

// ----------------------------------------------------------------------------
void ClientTextManagerNamespace::getDefaultTextColorFromDataTable(char const *text, ClientTextManager::TextType const textType, DataTable const &dataTable)
{
	// Find the row we are interested in

	for (int row = 0; row < dataTable.getNumRows(); ++row)
	{
		std::string const &rowString = dataTable.getStringValue("stringId", row);
		StringId stringId("ui_text_color", rowString);

		if (rowString == text)
		{
			// Store the color for this text type

			uint8 const red = static_cast<uint8>(dataTable.getIntValue(1, row));
			uint8 const green = static_cast<uint8>(dataTable.getIntValue(2, row));
			uint8 const blue = static_cast<uint8>(dataTable.getIntValue(3, row));
			PackedRgb packedRgb(red, green, blue);

			ClientTextManager::setColoredGameText(stringId.localize(), packedRgb);

			Unicode::String coloredGameTextName = Unicode::toLower(stringId.localize());
			s_defaultGameTextColors.insert(std::make_pair(textType, coloredGameTextName));
			s_defaultGameTextColorStringIds.insert(std::make_pair(coloredGameTextName, rowString));

			break;
		}
	}
}

// ----------------------------------------------------------------------------
std::string ClientTextManagerNamespace::getColoredGameTextStringIdFromString(Unicode::String colorName)
{
	DefaultGameTextColorStringIds::const_iterator iterColoredGameTextColorStringIds = s_defaultGameTextColorStringIds.begin();
	Unicode::String lower = Unicode::toLower(colorName);

	for (; iterColoredGameTextColorStringIds != s_defaultGameTextColorStringIds.end(); ++iterColoredGameTextColorStringIds)
	{
		if (lower == iterColoredGameTextColorStringIds->first)
			return iterColoredGameTextColorStringIds->second;
	}


	return std::string("");
}


// ----------------------------------------------------------------------------
void ClientTextManagerNamespace::getColorCode(PackedRgb const &packedRgb, ColorCode &colorCode)
{
	unsigned code = packedRgb.asUint32();
	unsigned rgb = code & 0xFFFFFF;

	sprintf(colorCode.m_code, "\\#%06x", rgb);
}

// ----------------------------------------------------------------------------
bool ClientTextManagerNamespace::setColoredText(ColoredTextList &coloredTextList, Unicode::String const &text, std::string const &colorCodeText, bool addColor = true)
{
	bool result = false;

	// See if this text is already used

	Unicode::String lowerText(Unicode::toLower(text));

	DEBUG_FATAL(colorCodeText.size() > 9, ("Color code is too big: %s", colorCodeText.c_str()));
	ColorCode colorCode;
	sprintf(colorCode.m_code, "%s", colorCodeText.c_str());

	ColoredTextList::iterator iterColoredTextList = coloredTextList.find(lowerText);

	// if already in list, change the value
	if (iterColoredTextList != coloredTextList.end())
	{
		// Set the color of the existing entry

		iterColoredTextList->second.setColorCode(colorCode);
	}
	// otherwise, if we're adding colors add this color
	else if (addColor)
	{
		result = true;

		// Save the new color coded text

		ColorCodedText colorCodedText;
		colorCodedText.setColorCode(colorCode);
		colorCodedText.setText(text);

		coloredTextList.insert(std::make_pair(lowerText, colorCodedText));

		//DEBUG_REPORT_LOG(true, ("%s\n", Unicode::wideToNarrow(lowerText)));
	}

	s_dataChanged = true;

	return result;
}

// ----------------------------------------------------------------------------
bool ClientTextManagerNamespace::setColoredText(ColoredTextList &coloredTextList, Unicode::String const &text, PackedRgb const &packedRgb, bool addColor = true)
{
	ColorCode colorCode;
	getColorCode(packedRgb, colorCode);

	return setColoredText(coloredTextList, text, colorCode.m_code, addColor);
}

// Removes the specified text from the colored text list.
// ----------------------------------------------------------------------------
void ClientTextManagerNamespace::removeColoredText(ColoredTextList &coloredTextList, Unicode::String const &text)
{
	ColoredTextList::iterator iterColoredTextList = coloredTextList.find(Unicode::toLower(text));

	if (iterColoredTextList != coloredTextList.end())
	{
		//DEBUG_REPORT_LOG(true, ("ClientTextManagerNamespace::removeColoredText() - %s", Unicode::wideToNarrow(text).c_str()));

		IGNORE_RETURN(coloredTextList.erase(iterColoredTextList));

		s_dataChanged = true;
	}
}

// ----------------------------------------------------------------------------
void ClientTextManagerNamespace::getColoredText(ColoredTextList const &coloredTextList, ClientTextManager::UnicodeStringList &unicodeStringList)
{
	unicodeStringList.clear();
	unicodeStringList.reserve(coloredTextList.size());

	ColoredTextList::const_iterator iterColoredText = coloredTextList.begin();

	for (; iterColoredText != coloredTextList.end(); ++iterColoredText)
	{
		Unicode::String text;

		text += iterColoredText->second.getColorCodeUnicodeString();
		text += iterColoredText->second.getText();

		unicodeStringList.push_back(text);
	}
}

// ----------------------------------------------------------------------------
std::string ClientTextManagerNamespace::getFileName()
{
	std::string result;
	std::string loginId;
	std::string clusterName;
	Unicode::String playerName;
	NetworkId id;

	if (Game::getPlayerPath(loginId, clusterName, playerName, id))
	{
		s_playerName = playerName;

		result = "profiles/" + loginId + "/chat_colors.iff";
	}

	return result;
}

// ----------------------------------------------------------------------------
void ClientTextManagerNamespace::load()
{
	// Save all the words

	std::string fileName(getFileName());

	// Check for file corruption

	if (Iff::isValid(fileName.c_str()))
	{
		// Load the custom colors

		Iff iff(2);

		if (iff.open(fileName.c_str(), true))
		{
			s_coloredWords.clear();
			s_coloredSentences.clear();

			iff.enterForm(s_tag);
			{
				iff.enterChunk(TAG_0000);
				{
					// Words

					{
						int count = iff.read_int32();

						for (int i = 0; i < count; ++i)
						{
							Unicode::String text(iff.read_unicodeString());
							std::string colorCode(iff.read_stdstring());

							setColoredText(s_coloredWords, text, colorCode);
						}
					}

					// Sentences

					{
						int count = iff.read_int32();

						for (int i = 0; i < count; ++i)
						{
							Unicode::String text(iff.read_unicodeString());
							std::string colorCode(iff.read_stdstring());

							setColoredText(s_coloredSentences, text, colorCode);
						}
					}

					// Game text

					{
						int count = iff.read_int32();

						// Keep this to make sure most people don't lose any values.  Basically,
						// if a person only plays English or Japanese they will maintain their
						// settings through this change.  The cost is a single int32 read/write.
						// This also makes it easy to know whether the .iff file being used is
						// of the old format or not.

						for (int i = 0; i < count; ++i)
						{
							// Only accept values if the String already exists.
							// This will lose values for people who play the different languages
							// and are not playing in the first locale they played in.
							Unicode::String text(iff.read_unicodeString());
							std::string colorCode(iff.read_stdstring());

							// Only change colors for game text, do not add.
							// This also makes it possible to now add new game text types.
							setColoredText(s_coloredGameText, text, colorCode, false);
						}
						if (count == 0)
						{
							// Make sure there is data left in the chunk
							if (iff.getChunkLengthLeft() > 0)
							{
								count = iff.read_int32();
								for (int i = 0; i < count; ++i)
								{
									std::string id(iff.read_stdstring());
									StringId stringId("ui_text_color", id);
									std::string colorCode(iff.read_stdstring());

									// Only change colors for game text, do not add.
									// This also makes it possible to now add new game text types.
									setColoredText(s_coloredGameText, stringId.localize(), colorCode, false);
								}
							}
						}
					}
				}
				iff.exitChunk(TAG_0000);
			}
			iff.exitForm(s_tag);
		}
	}
}

// ----------------------------------------------------------------------------
void ClientTextManagerNamespace::save()
{
	if (s_dataChanged)
	{
		s_dataChanged = false;

		// Save all the words

		Iff iff(2);

		iff.insertForm(s_tag);
		{
			iff.insertChunk(TAG_0000);
			{
				// Words

				ColoredTextList::const_iterator iterColoredWords = s_coloredWords.begin();

				iff.insertChunkData(static_cast<int>(s_coloredWords.size()));

				for (; iterColoredWords != s_coloredWords.end(); ++iterColoredWords)
				{
					iff.insertChunkString(iterColoredWords->second.getText());
					iff.insertChunkString(iterColoredWords->second.getColorCodeString().c_str());
				}

				// Sentences

				ColoredTextList::const_iterator iterColoredSentences = s_coloredSentences.begin();

				iff.insertChunkData(static_cast<int>(s_coloredSentences.size()));

				for (; iterColoredSentences != s_coloredSentences.end(); ++iterColoredSentences)
				{
					iff.insertChunkString(iterColoredSentences->second.getText());
					iff.insertChunkString(iterColoredSentences->second.getColorCodeString().c_str());
				}

				// Game text

				ColoredTextList::const_iterator iterColoredGameText = s_coloredGameText.begin();

				// Maintain a 0 value int32 for loading purposes, to determine if this is an old
				// iff or new.
				iff.insertChunkData(static_cast<int>(0));

				iff.insertChunkData(static_cast<int>(s_coloredGameText.size()));

				for (; iterColoredGameText != s_coloredGameText.end(); ++iterColoredGameText)
				{
					// get ID from the default values using getText() value
					Unicode::String text = iterColoredGameText->second.getText();
					std::string stringId = getColoredGameTextStringIdFromString(text);
					if (stringId == "")
					{
						DEBUG_REPORT_LOG(true, ("ClientTextManagerNamespace::save() - Color not found: %s", Unicode::wideToUTF8(text).c_str()));
					}
					iff.insertChunkString(stringId.c_str());
					iff.insertChunkString(iterColoredGameText->second.getColorCodeString().c_str());
				}
			}
			iff.exitChunk(TAG_0000);
		}
		iff.exitForm(s_tag);

		std::string fileName(getFileName());

		if (!fileName.empty())
		{
			iff.write(fileName.c_str());
		}
	}
}

// ============================================================================
//
// ClientTextManagerNamespace::ColorCodedText
//
// ============================================================================

// ----------------------------------------------------------------------------
ClientTextManagerNamespace::ColorCodedText::ColorCodedText()
 : m_text()
 , m_colorCode()
{
}

// ----------------------------------------------------------------------------
void ClientTextManagerNamespace::ColorCodedText::setColorCode(ColorCode const &colorCode)
{
	m_colorCode = colorCode;
}

// ----------------------------------------------------------------------------
Unicode::String const &ClientTextManagerNamespace::ColorCodedText::getText() const
{
	return m_text;
}

// ----------------------------------------------------------------------------
Unicode::String ClientTextManagerNamespace::ColorCodedText::getColorCodeUnicodeString() const
{
	return Unicode::narrowToWide(m_colorCode.m_code);
}

// ----------------------------------------------------------------------------
std::string ClientTextManagerNamespace::ColorCodedText::getColorCodeString() const
{
	return m_colorCode.m_code;
}


// ----------------------------------------------------------------------------
void ClientTextManagerNamespace::ColorCodedText::setText(Unicode::String const &text)
{
	m_text = text;
}

// ============================================================================
//
// ClientTextManagerNamespace::ColorCode
//
// ============================================================================

// ----------------------------------------------------------------------------
ClientTextManagerNamespace::ColorCode::ColorCode()
{
	m_code[0] = '\0';
}

// ============================================================================
//
// ClientTextManager
//
// ============================================================================

// ----------------------------------------------------------------------------
void ClientTextManager::install()
{
	InstallTimer const installTimer("ClientTextManager::install");

	DEBUG_FATAL(s_installed,("ClientTextManager already installed.\n"));
	s_installed = true;
	ExitChain::add(remove,"ClientTextManager::remove");

	s_resetColorCode = Unicode::narrowToWide("\\#.");

	// Create a white color code

	ClientTextManagerNamespace::getColorCode(PackedRgb::solidWhite, s_whiteColorCode);

	resetColoredGameText();
}

// ----------------------------------------------------------------------------
void ClientTextManager::remove()
{
	s_coloredWords.clear();
	s_coloredSentences.clear();
	s_coloredGameText.clear();
	s_defaultGameTextColors.clear();

	s_installed = false;
}

// ----------------------------------------------------------------------------
bool ClientTextManager::setColoredWord(Unicode::String const &text, PackedRgb const &packedRgb)
{
	return setColoredText(s_coloredWords, text, packedRgb);
}

// ----------------------------------------------------------------------------
bool ClientTextManager::setColoredSentence(Unicode::String const &text, PackedRgb const &packedRgb)
{
	return setColoredText(s_coloredSentences, text, packedRgb);
}

// ----------------------------------------------------------------------------
bool ClientTextManager::setColoredGameText(Unicode::String const &text, PackedRgb const &packedRgb)
{
	return setColoredText(s_coloredGameText, text, packedRgb);
}

// ----------------------------------------------------------------------------
void ClientTextManager::removeColoredWord(Unicode::String const &text)
{
	removeColoredText(s_coloredWords, text);
}

// ----------------------------------------------------------------------------
void ClientTextManager::removeColoredSentence(Unicode::String const &text)
{
	removeColoredText(s_coloredSentences, text);
}

// ----------------------------------------------------------------------------
Unicode::String ClientTextManager::colorText(Unicode::String const &text, TextType const textType)
{
	return colorAndFilterText(text, textType, false);
}

// Filter the text for any profanity and colorizes the text if any words match
// the colored words.
// ----------------------------------------------------------------------------
Unicode::String ClientTextManager::colorAndFilterText(Unicode::String const &text, TextType const textType, bool const profanityFiltered)
{
	if (text.empty())
	{
		return text;
	}

	TextIterator textIterator(profanityFiltered ? TextManager::filterText(text) : text);

	// Get the raw printable text without any color codes in it

	Unicode::String printableText(textIterator.getPrintableText());
	Unicode::String lowerPrintableText(Unicode::toLower(printableText));

	// Put the original text color at the front of the line

	textIterator.insertTextAtFirstPrintablePosition(getColorCode(textType));

	// Find all the colored sentences

	ColoredTextList::const_iterator iterColoredSentences = s_coloredSentences.begin();

	for (; iterColoredSentences != s_coloredSentences.end(); ++iterColoredSentences)
	{
		Unicode::String const &coloredText = iterColoredSentences->first;

		int position = 0;

		while (position == 0)
		{
			position = lowerPrintableText.find(coloredText, position);

			if (position != Unicode::String::npos)
			{
				textIterator.insertTextAtFirstPrintablePosition(iterColoredSentences->second.getColorCodeUnicodeString());
				++position;
			}
		}
	}

	// Find all the positions of colored words

	ColoredTextList::const_iterator iterColoredWords = s_coloredWords.begin();

	for (; iterColoredWords != s_coloredWords.end(); ++iterColoredWords)
	{
		Unicode::String const &coloredText = iterColoredWords->first;

		int position = 0;

		while (position != Unicode::String::npos)
		{
			position = lowerPrintableText.find(coloredText, position);

			if (position != Unicode::String::npos)
			{
				textIterator.insertCurrentColorCodeAtPrintablePosition(position + coloredText.size());
				textIterator.insertTextAtPrintablePosition(position, iterColoredWords->second.getColorCodeUnicodeString());
				++position;
			}
		}
	}

	// Insert the tag to reset to the default text color

	textIterator.appendText(ClientTextManager::getResetTagCode());

	return textIterator.getRawText();
}

// ----------------------------------------------------------------------------
Unicode::String const &ClientTextManager::getResetTagCode()
{
	return s_resetColorCode;
}

// ----------------------------------------------------------------------------
Unicode::String ClientTextManager::getColorCode(PackedRgb const &packedRgb)
{
	ColorCode colorCode;

	ClientTextManagerNamespace::getColorCode(packedRgb, colorCode);

	return Unicode::narrowToWide(colorCode.m_code);
}

// ----------------------------------------------------------------------------
Unicode::String ClientTextManager::getColorCode(TextType const textType)
{
	Unicode::String result;
	DefaultGameTextColors::const_iterator iterDefaultGameTextColors = s_defaultGameTextColors.find(textType);

	if (iterDefaultGameTextColors != s_defaultGameTextColors.end())
	{
		Unicode::String const &name = iterDefaultGameTextColors->second;

		ColoredTextList::const_iterator iterColoredGameText = s_coloredGameText.find(name);

		if (iterColoredGameText != s_coloredGameText.end())
		{
			result = iterColoredGameText->second.getColorCodeUnicodeString();
		}
		else
		{
			result = Unicode::narrowToWide(s_whiteColorCode.m_code);
			DEBUG_WARNING(true, ("Unable to find game text color: %s", Unicode::wideToNarrow(name).c_str()));
		}
	}
	else
	{
		result = Unicode::narrowToWide(s_whiteColorCode.m_code);
		DEBUG_WARNING(true, ("Unable to find game text color."));
	}

	return result;
}

// ----------------------------------------------------------------------------
void ClientTextManager::getColoredWords(UnicodeStringList &coloredWords)
{
	getColoredText(s_coloredWords, coloredWords);
}

// ----------------------------------------------------------------------------
void ClientTextManager::getColoredSentences(UnicodeStringList &coloredSentences)
{
	getColoredText(s_coloredSentences, coloredSentences);
}

// ----------------------------------------------------------------------------
void ClientTextManager::getColoredGameText(UnicodeStringList &coloredGameText)
{
	getColoredText(s_coloredGameText, coloredGameText);
}

// ----------------------------------------------------------------------------
void ClientTextManager::saveUserSettings()
{
	save();
}

// ----------------------------------------------------------------------------
void ClientTextManager::loadUserSettings()
{
	load();
}

void ClientTextManager::resetColoredGameText()
{
	// Load the default colors

	char const textColorsFile[] = "datatables/chat/default_text_colors.iff";
	Iff textColorsIff;

	if (textColorsIff.open(textColorsFile, true))
	{
		DataTable dataTable;

		dataTable.load(textColorsIff);

		s_defaultGameTextColors.clear();

		getDefaultTextColorFromDataTable("chat_channel", TT_chatChannel, dataTable);
		getDefaultTextColorFromDataTable("chat_channel_group", TT_chatChannelGroup, dataTable);
		getDefaultTextColorFromDataTable("chat_channel_guild", TT_guild, dataTable);
		getDefaultTextColorFromDataTable("chat_channel_city", TT_city, dataTable);
		getDefaultTextColorFromDataTable("combat_generic", TT_combatGeneric, dataTable);
		getDefaultTextColorFromDataTable("combat_out_of_range", TT_combatOutOfRange, dataTable);
		getDefaultTextColorFromDataTable("combat_posture_change", TT_combatPostureChange, dataTable);
		getDefaultTextColorFromDataTable("combat_other_hit_other", TT_combatOtherHitOther, dataTable);
		getDefaultTextColorFromDataTable("combat_other_hit_other_critical", TT_combatOtherHitOtherCritical, dataTable);
		getDefaultTextColorFromDataTable("combat_other_hit_other_glance", TT_combatOtherHitOtherGlance, dataTable);
		getDefaultTextColorFromDataTable("combat_other_hit_other_proc", TT_combatOtherHitOtherProc, dataTable);
		getDefaultTextColorFromDataTable("combat_other_hit_you", TT_combatOtherHitYou, dataTable);
		getDefaultTextColorFromDataTable("combat_other_hit_you_critical", TT_combatOtherHitYouCritical, dataTable);
		getDefaultTextColorFromDataTable("combat_other_hit_you_glance", TT_combatOtherHitYouGlance, dataTable);
		getDefaultTextColorFromDataTable("combat_other_hit_you_proc", TT_combatOtherHitYouProc, dataTable);
		getDefaultTextColorFromDataTable("combat_other_miss_other", TT_combatOtherMissOther, dataTable);
		getDefaultTextColorFromDataTable("combat_other_miss_you", TT_combatOtherMissYou, dataTable);
		getDefaultTextColorFromDataTable("combat_you_hit_other", TT_combatYouHitOther, dataTable);
		getDefaultTextColorFromDataTable("combat_you_hit_other_critical", TT_combatYouHitOtherCritical, dataTable);
		getDefaultTextColorFromDataTable("combat_you_hit_other_glance", TT_combatYouHitOtherGlance, dataTable);
		getDefaultTextColorFromDataTable("combat_you_hit_other_proc", TT_combatYouHitOtherProc, dataTable);
		getDefaultTextColorFromDataTable("combat_you_miss_other", TT_combatYouMissOther, dataTable);
		getDefaultTextColorFromDataTable("combat_medical", TT_combatOtherMedical, dataTable);
		getDefaultTextColorFromDataTable("combat_buff", TT_combatOtherBuff, dataTable);
		getDefaultTextColorFromDataTable("combat_debuff", TT_combatOtherDebuff, dataTable);
		getDefaultTextColorFromDataTable("emote", TT_emote, dataTable);
		getDefaultTextColorFromDataTable("guild", TT_guild, dataTable);
		getDefaultTextColorFromDataTable("city", TT_city, dataTable);
		getDefaultTextColorFromDataTable("matchmaking", TT_matchMaking, dataTable);
		getDefaultTextColorFromDataTable("online_status", TT_onlineStatus, dataTable);
		getDefaultTextColorFromDataTable("spatial", TT_spatial, dataTable);
		getDefaultTextColorFromDataTable("social", TT_social, dataTable);
		getDefaultTextColorFromDataTable("system_message", TT_systemMessage, dataTable);
		getDefaultTextColorFromDataTable("tell", TT_tell, dataTable);
		getDefaultTextColorFromDataTable("who_player_name", TT_who_player_name, dataTable);
		getDefaultTextColorFromDataTable("who_guild", TT_who_guild, dataTable);
		getDefaultTextColorFromDataTable("who_species", TT_who_species, dataTable);
		getDefaultTextColorFromDataTable("who_title", TT_who_title, dataTable);
		getDefaultTextColorFromDataTable("who_region", TT_who_region, dataTable);
		getDefaultTextColorFromDataTable("who_planet", TT_who_planet, dataTable);
		getDefaultTextColorFromDataTable("who_tags", TT_who_tags, dataTable);
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the default text color data table: %s", textColorsFile));
	}
}
// ============================================================================
