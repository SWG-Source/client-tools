//======================================================================
//
// CuiAvatarCreationEmotes.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiAvatarCreationEmotes_H
#define INCLUDED_CuiAvatarCreationEmotes_H

//======================================================================

class UIDataSource;
class UINamespace;

#include "UILowerString.h"
#include <map>
#include <string>

//----------------------------------------------------------------------

class CuiAvatarCreationEmotes
{
public:

	struct Properties
	{
		static const UILowerString percent;
		static const UILowerString weight;
		static const char * const def;
	};
	
	typedef stdmap<int, std::string>::fwd       EmoteMap;
	
	struct EmoteGroup
	{
		typedef stdmap<std::string, int>::fwd            EmoteNumberMap;
		
		struct EmoteData
		{
			EmoteMap            map;
			int                 percent;
			int                 maximum;
			mutable std::string lastEmote;
			
			void createProfessionEmotes (const EmoteNumberMap & numberMap);			
			bool findRandomEmote (std::string & result, bool skipLastResult = true) const;
		};
		
		typedef stdmap<std::string, EmoteData>::fwd      ProfessionEmotes;
		
		int                basePercent;
		EmoteNumberMap     defaultEmoteWeights;
		ProfessionEmotes   emotes;
		std::string        lastEmote;
		
		bool           findRandomEmote       (const std::string & profession, std::string & result) const;
		static void    mergeNumberMap (const UIDataSource & ds, EmoteNumberMap & numberMap);
		void           setup (const UINamespace & ns);
		
		void unload ();
		
		//-----------------------------------------------------------------
		
	};
	
	//----------------------------------------------------------------------
	
	static void install ();
	static void remove  ();
	static void registerEmoteGroup (const std::string & groupname, bool reload = false);
	static bool findRandomEmote    (const std::string & groupname, const std::string & dataName, std::string & emote);
};

//======================================================================

#endif
