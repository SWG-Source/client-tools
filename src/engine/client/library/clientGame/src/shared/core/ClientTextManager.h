// ============================================================================
//
// ClientTextManager.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ClientTextManager_H
#define INCLUDED_ClientTextManager_H

class PackedRgb;

//-----------------------------------------------------------------
namespace MessageDispatch
{
	class Callback;
}

// ----------------------------------------------------------------------------
class ClientTextManager
{
public:

	static void install();

	typedef stdvector<Unicode::String>::fwd UnicodeStringList;

	enum TextType
	{
		TT_combatGeneric,
		TT_combatOutOfRange,
		TT_combatPostureChange,
		TT_combatYouHitOther,
		TT_combatYouHitOtherCritical,
		TT_combatYouHitOtherGlance,
		TT_combatYouHitOtherProc,
		TT_combatYouMissOther,
		TT_combatOtherHitYou,
		TT_combatOtherHitYouCritical,
		TT_combatOtherHitYouGlance,
		TT_combatOtherHitYouProc,
		TT_combatOtherMissYou,
		TT_combatOtherHitOther,
		TT_combatOtherHitOtherCritical,
		TT_combatOtherHitOtherGlance,
		TT_combatOtherHitOtherProc,
		TT_combatOtherMissOther,
		TT_combatOtherMedical,
		TT_combatOtherBuff,
		TT_combatOtherDebuff,
		TT_spatial,
		TT_tell,
		TT_systemMessage,
		TT_matchMaking,
		TT_guild,
		TT_emote,
		TT_chatChannel,
		TT_chatChannelGroup,
		TT_social,
		TT_onlineStatus,
		TT_who_player_name,
		TT_who_guild,
		TT_who_species,
		TT_who_title,
		TT_who_region,
		TT_who_planet,
		TT_who_tags,
		TT_city,
		TT_count
	};

public:

	static void            loadUserSettings();
	static void            saveUserSettings();

	static Unicode::String colorAndFilterText(Unicode::String const &text, TextType const textType, bool const profanityFiltered);
	static Unicode::String colorText(Unicode::String const &text, TextType const textType);

	static Unicode::String getColorCode(TextType const textType);
	static Unicode::String getColorCode(PackedRgb const &packedRgb);
	static bool            setColoredWord(Unicode::String const &text, PackedRgb const &packedRgb);
	static bool            setColoredSentence(Unicode::String const &text, PackedRgb const &packedRgb);
	static bool            setColoredGameText(Unicode::String const &text, PackedRgb const &packedRgb);
	static void            resetColoredGameText();
	static void            removeColoredWord(Unicode::String const &text);
	static void            removeColoredSentence(Unicode::String const &text);

	static void            getColoredWords(UnicodeStringList &coloredWords);
	static void            getColoredSentences(UnicodeStringList &coloredSentences);
	static void            getColoredGameText(UnicodeStringList &coloredGameText);

	static Unicode::String const &getResetTagCode();

private:

	static void remove();

	ClientTextManager();
	~ClientTextManager();
};

// ============================================================================

#endif // INCLUDED_ClientTextManager_H
