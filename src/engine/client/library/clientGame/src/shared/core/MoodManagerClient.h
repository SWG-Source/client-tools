//======================================================================
//
// MoodManagerClient.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiMoodManager_H
#define INCLUDED_CuiMoodManager_H

//======================================================================

class LocalizedStringTable;

//----------------------------------------------------------------------

class MoodManagerClient
{
public:
	static void install ();
	static void remove ();

	static const Unicode::String &       getMoodCommand     ();
	static const Unicode::String &       getMoodListCommand ();

	static const LocalizedStringTable *  getMoodsTable ();

	static uint32                        getMoodByName          (const Unicode::String & name);
	static uint32                        getMoodByAbbrevNoCase  (const Unicode::String & abbrev);
	static bool                          getMoodName            (uint32 mood, Unicode::String & name);
	static bool                          getMoodEmoteAnimation  (uint32 mood, std::string & emoteName);
	static Unicode::String               getMoodAdverbPhrase    (uint32 mood, bool random);
};

//======================================================================

#endif
