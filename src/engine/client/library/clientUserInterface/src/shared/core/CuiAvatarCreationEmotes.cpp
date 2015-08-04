//======================================================================
//
// CuiAvatarCreationEmotes.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiAvatarCreationEmotes.h"

#include "sharedRandom/Random.h"
#include "UIDataSource.h"
#include "UIData.h"
#include "UINamespace.h"
#include "UIManager.h"
#include "UIPage.h"

//======================================================================

namespace
{
	std::string        s_selectionEmotesPath = "/AvatarData.ProfessionSelectionEmotes";

	typedef CuiAvatarCreationEmotes::EmoteGroup EmoteGroup;

	typedef stdmap<std::string, EmoteGroup>::fwd EmoteGroupMap;
	EmoteGroupMap      s_emoteGroups;
}

//----------------------------------------------------------------------

const UILowerString CuiAvatarCreationEmotes::Properties::percent = UILowerString ("percent");
const UILowerString CuiAvatarCreationEmotes::Properties::weight  = UILowerString ("weight");
const char * const CuiAvatarCreationEmotes::Properties::def     = "default";

//-----------------------------------------------------------------

void CuiAvatarCreationEmotes::EmoteGroup::EmoteData::createProfessionEmotes (const EmoteNumberMap & numberMap)
{
	int current = 0;
	for (EmoteNumberMap::const_iterator it = numberMap.begin (); it != numberMap.end (); ++it)
	{
		const std::string & name = (*it).first;
		const int weight         = (*it).second;
		
		map [current] = name;
		current += weight;
	}
	
	maximum = current - 1;
	lastEmote.clear ();
}

//-----------------------------------------------------------------

bool CuiAvatarCreationEmotes::EmoteGroup::EmoteData::findRandomEmote (std::string & result, bool skipLastResult) const
{
	EmoteMap::const_iterator it = map.upper_bound (Random::random (0, maximum));
	
	if (it == map.end ())
		return false;
	
	const std::string & emote = (*it).second;
	
	if (skipLastResult && emote == lastEmote)
		return findRandomEmote (result, false);
	
	lastEmote = result = emote;
	return true;
}

//-----------------------------------------------------------------

bool CuiAvatarCreationEmotes::EmoteGroup::findRandomEmote (const std::string & profession, std::string & result) const
{
	ProfessionEmotes::const_iterator pit = emotes.find (profession);
	if (pit == emotes.end ())
	{
		pit = emotes.find (Properties::def);
		if (pit == emotes.end ())
			return false;
	}
	
	const EmoteData & emoteData = (*pit).second;
	
	return emoteData.findRandomEmote (result);
}

//-----------------------------------------------------------------

void CuiAvatarCreationEmotes::EmoteGroup::mergeNumberMap (const UIDataSource & ds, EmoteNumberMap & numberMap)
{
	const UIDataList & datalist = ds.GetData ();
	for (UIDataList::const_iterator dit = datalist.begin (); dit != datalist.end (); ++dit)
	{
		const UIData * const data = NON_NULL (*dit);
		
		int weight = 1;
		data->GetPropertyInteger (Properties::weight, weight);
		numberMap [data->GetName ()] = weight;
	}
}

//-----------------------------------------------------------------

void CuiAvatarCreationEmotes::EmoteGroup::setup (const UINamespace & ns)
{
	UIDataSource * const def = dynamic_cast<UIDataSource *>(ns.GetChild (Properties::def));
	defaultEmoteWeights.clear ();

	basePercent = 100;
	if (def)
	{
		def->GetPropertyInteger (Properties::percent, basePercent);	
		mergeNumberMap (*def, defaultEmoteWeights);
	}
	
	{
		EmoteData & defaultEmoteData = emotes [Properties::def];
		defaultEmoteData.createProfessionEmotes (defaultEmoteWeights);	
		defaultEmoteData.percent = basePercent;
	}
	
	const UIBaseObject::UIObjectList & olist = ns.GetChildrenRef ();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		const UIDataSource * const ds = dynamic_cast<UIDataSource *>(*it);
		
		if (ds && _stricmp (ds->GetName ().c_str (), Properties::def))
		{
			EmoteData & emoteData = emotes [ds->GetName ()];
			
			EmoteNumberMap numberMap (defaultEmoteWeights);
			mergeNumberMap (*ds, numberMap);
			
			emoteData.createProfessionEmotes (numberMap);
			
			if (!ds->GetPropertyInteger (Properties::percent, emoteData.percent))
				emoteData.percent = 100;
		}
	}
}

//-----------------------------------------------------------------

void CuiAvatarCreationEmotes::EmoteGroup::unload ()
{
	defaultEmoteWeights.clear ();
	emotes.clear ();
}

//----------------------------------------------------------------------

void CuiAvatarCreationEmotes::install ()	
{
	registerEmoteGroup ("reject",    true);
	registerEmoteGroup ("accept",    true);
	registerEmoteGroup ("idle_face", true);
	registerEmoteGroup ("idle_body", true);
}

//-----------------------------------------------------------------

void CuiAvatarCreationEmotes::remove ()
{
	s_emoteGroups.clear ();
}

//----------------------------------------------------------------------

void CuiAvatarCreationEmotes::registerEmoteGroup (const std::string & groupname, bool reload)
{
	const EmoteGroupMap::iterator it = s_emoteGroups.find (groupname);

	if (reload || it == s_emoteGroups.end ())
	{
		UINamespace * const ns = static_cast<UINamespace *>(UIManager::gUIManager ().GetRootPage ()->GetObjectFromPath (s_selectionEmotesPath.c_str (), TUINamespace));
		const UINamespace * const group_ns = ns ? dynamic_cast<UINamespace *>(ns->GetChild (groupname.c_str ())) : 0;

		if (group_ns)
			s_emoteGroups [groupname].setup (*group_ns);
		else
		{
			WARNING (true, ("Unable to find emote group for '%s'", groupname.c_str ()));
			s_emoteGroups [groupname].unload ();
		}
	}
}

//----------------------------------------------------------------------

bool CuiAvatarCreationEmotes::findRandomEmote    (const std::string & groupname, const std::string & dataname, std::string & emote)
{
	registerEmoteGroup (groupname, false);
	const EmoteGroupMap::iterator it = s_emoteGroups.find (groupname);

	if (it != s_emoteGroups.end ())
	{
		const EmoteGroup & group = (*it).second;
		return group.findRandomEmote (dataname, emote);
	}

	return false;
}

//======================================================================
