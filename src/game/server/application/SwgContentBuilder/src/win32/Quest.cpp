// ======================================================================
//
// Quest.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "Quest.h"

#include <algorithm>

// ======================================================================

Quest::Quest () :
	m_conversationStfName (),
	m_questType ("deliver"),
	m_overallObjvar (),
	m_deliverObjectTemplate (),
	m_retrieveObjectTemplate (),
	m_tempObjvar (),
	m_playerScript ("npc.static_quest.quest_player"),
	m_questLeg (0),
	m_questNpcEntry ("none"),
	m_questNpcScript ("npc.static_quest.quest_npc"),
	m_questNpcName (),
	m_missionConversationType ("normal"),
	m_questNpcConversationType ("normal"),
	m_locationPlanet ("none"),
	m_locationX (0.f),
	m_locationY (0.f),
	m_locationZ (0.f),
	m_credits (0),
	m_reward1ObjectTemplate (),
	m_reward1Objvar (),
	m_reward1ObjvarValue (),
	m_reward1Faction (),
	m_reward1FactionValue (0),
	m_reward2ObjectTemplate (),
	m_reward2Objvar (),
	m_reward2ObjvarValue (),
	m_reward2Faction (),
	m_reward2FactionValue (0),
	m_reward3ObjectTemplate (),
	m_reward3Objvar (),
	m_reward3ObjvarValue (),
	m_reward3Faction (),
	m_reward3FactionValue (0),
	m_reward4ObjectTemplate (),
	m_reward4Objvar (),
	m_reward4ObjvarValue (),
	m_reward4Faction (),
	m_reward4FactionValue (0),
	m_extraNpc1Entry ("none"),
	m_extraNpc1Disposition ("neutral"),
	m_extraNpc2Entry ("none"),
	m_extraNpc2Disposition ("neutral"),
	m_extraNpc3Entry ("none"),
	m_extraNpc3Disposition ("neutral"),
	m_extraNpc4Entry ("none"),
	m_extraNpc4Disposition ("neutral"),
	m_extraSpawnObjectTemplate (),
	m_extraSpawnDelay (0),
	m_extraSpawnFrequency (0),
	m_extraSpawnLimit (0),
	m_extraSpawnRadius (0),
	m_extraSpawnAction1 ("none"),
	m_extraSpawnAction2 ("none"),
	m_extraSpawnAction3 ("none"),
	m_extraSpawnAction4 ("none"),
	m_extraSpawnScript (),
	m_extraSpawnEncounterDataTable (),
	m_gatingObjvar (),
	m_gatingObjectTemplate (),
	m_gatingFaction (),
	m_gatingFactionValue (0)
{
}

// ----------------------------------------------------------------------

Quest::~Quest ()
{
}

// ----------------------------------------------------------------------

std::string const & Quest::getConversationStfName () const
{
	return m_conversationStfName;
}

// ----------------------------------------------------------------------

void Quest::setConversationStfName (std::string const & conversationStfName)
{
	m_conversationStfName = conversationStfName;
	std::transform (m_conversationStfName.begin (), m_conversationStfName.end (), m_conversationStfName.begin (), tolower);
}

// ----------------------------------------------------------------------

std::string const & Quest::getQuestType () const
{
	return m_questType;
}

// ----------------------------------------------------------------------

void Quest::setQuestType (std::string const & questType)
{
	m_questType = questType;
}

// ----------------------------------------------------------------------

std::string const & Quest::getOverallObjvar () const
{
	return m_overallObjvar;
}

// ----------------------------------------------------------------------

void Quest::setOverallObjvar (std::string const & overallObjvar)
{
	m_overallObjvar = overallObjvar;
}

// ----------------------------------------------------------------------

std::string const & Quest::getTempObjvar () const
{
	return m_tempObjvar;
}

// ----------------------------------------------------------------------

void Quest::setTempObjvar (std::string const & tempObjvar)
{
	m_tempObjvar = tempObjvar;
}

// ----------------------------------------------------------------------

std::string const & Quest::getDeliverObjectTemplate () const
{
	return m_deliverObjectTemplate;
}

// ----------------------------------------------------------------------

void Quest::setDeliverObjectTemplate (std::string const & deliverObjectTemplate)
{
	m_deliverObjectTemplate = deliverObjectTemplate;
}

// ----------------------------------------------------------------------

std::string const & Quest::getRetrieveObjectTemplate () const
{
	return m_retrieveObjectTemplate;
}

// ----------------------------------------------------------------------

void Quest::setRetrieveObjectTemplate (std::string const & retrieveObjectTemplate)
{
	m_retrieveObjectTemplate = retrieveObjectTemplate;
}

// ----------------------------------------------------------------------

std::string const & Quest::getPlayerScript () const
{
	return m_playerScript;
}

// ----------------------------------------------------------------------

void Quest::setPlayerScript (std::string const & playerScript)
{
	m_playerScript = playerScript;
}

// ----------------------------------------------------------------------

int Quest::getQuestLeg () const
{
	return m_questLeg;
}

// ----------------------------------------------------------------------

void Quest::setQuestLeg (int const questLeg)
{
	m_questLeg = questLeg;
}

// ----------------------------------------------------------------------

std::string const & Quest::getQuestNpcEntry () const
{
	return m_questNpcEntry;
}

// ----------------------------------------------------------------------

void Quest::setQuestNpcEntry (std::string const & questNpcEntry)
{
	m_questNpcEntry = questNpcEntry;
}

// ----------------------------------------------------------------------

std::string const & Quest::getQuestNpcScript () const
{
	return m_questNpcScript;
}

// ----------------------------------------------------------------------

void Quest::setQuestNpcScript (std::string const & questNpcScript)
{
	m_questNpcScript = questNpcScript;
}

// ----------------------------------------------------------------------

std::string const & Quest::getQuestNpcName () const
{
	return m_questNpcName;
}

// ----------------------------------------------------------------------

void Quest::setQuestNpcName (std::string const & questNpcName)
{
	m_questNpcName = questNpcName;
}

// ----------------------------------------------------------------------

std::string const & Quest::getMissionConversationType () const
{
	return m_missionConversationType;
}

// ----------------------------------------------------------------------

void Quest::setMissionConversationType (std::string const & missionConversationType)
{
	m_missionConversationType = missionConversationType;
}

// ----------------------------------------------------------------------

std::string const & Quest::getQuestNpcConversationType () const
{
	return m_questNpcConversationType;
}

// ----------------------------------------------------------------------

void Quest::setQuestNpcConversationType (std::string const & questNpcConversationType)
{
	m_questNpcConversationType = questNpcConversationType;
}

// ----------------------------------------------------------------------

std::string const & Quest::getLocationPlanet () const
{
	return m_locationPlanet;
}

// ----------------------------------------------------------------------

void Quest::setLocationPlanet (std::string const & locationPlanet)
{
	m_locationPlanet = locationPlanet;
}

// ----------------------------------------------------------------------

float Quest::getLocationX () const
{
	return m_locationX;
}

// ----------------------------------------------------------------------

void Quest::setLocationX (float const locationX)
{
	m_locationX = locationX;
}

// ----------------------------------------------------------------------

float Quest::getLocationY () const
{
	return m_locationY;
}

// ----------------------------------------------------------------------

void Quest::setLocationY (float const locationY)
{
	m_locationY = locationY;
}

// ----------------------------------------------------------------------

float Quest::getLocationZ () const
{
	return m_locationZ;
}

// ----------------------------------------------------------------------

void Quest::setLocationZ (float const locationZ)
{
	m_locationZ = locationZ;
}

// ----------------------------------------------------------------------

int Quest::getCredits () const
{
	return m_credits;
}

// ----------------------------------------------------------------------

void Quest::setCredits (int credits)
{
	m_credits = credits;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward1ObjectTemplate () const
{
	return m_reward1ObjectTemplate;
}

// ----------------------------------------------------------------------

void Quest::setReward1ObjectTemplate (std::string const & reward1ObjectTemplate)
{
	m_reward1ObjectTemplate = reward1ObjectTemplate;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward1Objvar () const
{
	return m_reward1Objvar;
}

// ----------------------------------------------------------------------

void Quest::setReward1Objvar (std::string const & reward1Objvar)
{
	m_reward1Objvar = reward1Objvar;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward1ObjvarValue () const
{
	return m_reward1ObjvarValue;
}

// ----------------------------------------------------------------------

void Quest::setReward1ObjvarValue (std::string const & reward1ObjvarValue)
{
	m_reward1ObjvarValue = reward1ObjvarValue;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward1Faction () const
{
	return m_reward1Faction;
}

// ----------------------------------------------------------------------

void Quest::setReward1Faction (std::string const & reward1Faction)
{
	m_reward1Faction = reward1Faction;
}

// ----------------------------------------------------------------------

int Quest::getReward1FactionValue () const
{
	return m_reward1FactionValue;
}

// ----------------------------------------------------------------------

void Quest::setReward1FactionValue (int const reward1FactionValue)
{
	m_reward1FactionValue = reward1FactionValue;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward2ObjectTemplate () const
{
	return m_reward2ObjectTemplate;
}

// ----------------------------------------------------------------------

void Quest::setReward2ObjectTemplate (std::string const & reward2ObjectTemplate)
{
	m_reward2ObjectTemplate = reward2ObjectTemplate;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward2Objvar () const
{
	return m_reward2Objvar;
}

// ----------------------------------------------------------------------

void Quest::setReward2Objvar (std::string const & reward2Objvar)
{
	m_reward2Objvar = reward2Objvar;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward2ObjvarValue () const
{
	return m_reward2ObjvarValue;
}

// ----------------------------------------------------------------------

void Quest::setReward2ObjvarValue (std::string const & reward2ObjvarValue)
{
	m_reward2ObjvarValue = reward2ObjvarValue;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward2Faction () const
{
	return m_reward2Faction;
}

// ----------------------------------------------------------------------

void Quest::setReward2Faction (std::string const & reward2Faction)
{
	m_reward2Faction = reward2Faction;
}

// ----------------------------------------------------------------------

int Quest::getReward2FactionValue () const
{
	return m_reward2FactionValue;
}

// ----------------------------------------------------------------------

void Quest::setReward2FactionValue (int const reward2FactionValue)
{
	m_reward2FactionValue = reward2FactionValue;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward3ObjectTemplate () const
{
	return m_reward3ObjectTemplate;
}

// ----------------------------------------------------------------------

void Quest::setReward3ObjectTemplate (std::string const & reward3ObjectTemplate)
{
	m_reward3ObjectTemplate = reward3ObjectTemplate;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward3Objvar () const
{
	return m_reward3Objvar;
}

// ----------------------------------------------------------------------

void Quest::setReward3Objvar (std::string const & reward3Objvar)
{
	m_reward3Objvar = reward3Objvar;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward3ObjvarValue () const
{
	return m_reward3ObjvarValue;
}

// ----------------------------------------------------------------------

void Quest::setReward3ObjvarValue (std::string const & reward3ObjvarValue)
{
	m_reward3ObjvarValue = reward3ObjvarValue;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward3Faction () const
{
	return m_reward3Faction;
}

// ----------------------------------------------------------------------

void Quest::setReward3Faction (std::string const & reward3Faction)
{
	m_reward3Faction = reward3Faction;
}

// ----------------------------------------------------------------------

int Quest::getReward3FactionValue () const
{
	return m_reward3FactionValue;
}

// ----------------------------------------------------------------------

void Quest::setReward3FactionValue (int const reward3FactionValue)
{
	m_reward3FactionValue = reward3FactionValue;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward4ObjectTemplate () const
{
	return m_reward4ObjectTemplate;
}

// ----------------------------------------------------------------------

void Quest::setReward4ObjectTemplate (std::string const & reward4ObjectTemplate)
{
	m_reward4ObjectTemplate = reward4ObjectTemplate;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward4Objvar () const
{
	return m_reward4Objvar;
}

// ----------------------------------------------------------------------

void Quest::setReward4Objvar (std::string const & reward4Objvar)
{
	m_reward4Objvar = reward4Objvar;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward4ObjvarValue () const
{
	return m_reward4ObjvarValue;
}

// ----------------------------------------------------------------------

void Quest::setReward4ObjvarValue (std::string const & reward4ObjvarValue)
{
	m_reward4ObjvarValue = reward4ObjvarValue;
}

// ----------------------------------------------------------------------

std::string const & Quest::getReward4Faction () const
{
	return m_reward4Faction;
}

// ----------------------------------------------------------------------

void Quest::setReward4Faction (std::string const & reward4Faction)
{
	m_reward4Faction = reward4Faction;
}

// ----------------------------------------------------------------------

int Quest::getReward4FactionValue () const
{
	return m_reward4FactionValue;
}

// ----------------------------------------------------------------------

void Quest::setReward4FactionValue (int const reward4FactionValue)
{
	m_reward4FactionValue = reward4FactionValue;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraNpc1Entry () const
{
	return m_extraNpc1Entry;
}

// ----------------------------------------------------------------------

void Quest::setExtraNpc1Entry (std::string const & extraNpc1Entry)
{
	m_extraNpc1Entry = extraNpc1Entry;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraNpc1Disposition () const
{
	return m_extraNpc1Disposition;
}

// ----------------------------------------------------------------------

void Quest::setExtraNpc1Disposition (std::string const & extraNpc1Disposition)
{
	m_extraNpc1Disposition = extraNpc1Disposition;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraNpc2Entry () const
{
	return m_extraNpc2Entry;
}

// ----------------------------------------------------------------------

void Quest::setExtraNpc2Entry (std::string const & extraNpc2Entry)
{
	m_extraNpc2Entry = extraNpc2Entry;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraNpc2Disposition () const
{
	return m_extraNpc2Disposition;
}

// ----------------------------------------------------------------------

void Quest::setExtraNpc2Disposition (std::string const & extraNpc2Disposition)
{
	m_extraNpc2Disposition = extraNpc2Disposition;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraNpc3Entry () const
{
	return m_extraNpc3Entry;
}

// ----------------------------------------------------------------------

void Quest::setExtraNpc3Entry (std::string const & extraNpc3Entry)
{
	m_extraNpc3Entry = extraNpc3Entry;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraNpc3Disposition () const
{
	return m_extraNpc3Disposition;
}

// ----------------------------------------------------------------------

void Quest::setExtraNpc3Disposition (std::string const & extraNpc3Disposition)
{
	m_extraNpc3Disposition = extraNpc3Disposition;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraNpc4Entry () const
{
	return m_extraNpc4Entry;
}

// ----------------------------------------------------------------------

void Quest::setExtraNpc4Entry (std::string const & extraNpc4Entry)
{
	m_extraNpc4Entry = extraNpc4Entry;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraNpc4Disposition () const
{
	return m_extraNpc4Disposition;
}

// ----------------------------------------------------------------------

void Quest::setExtraNpc4Disposition (std::string const & extraNpc4Disposition)
{
	m_extraNpc4Disposition = extraNpc4Disposition;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraSpawnObjectTemplate () const
{
	return m_extraSpawnObjectTemplate;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnObjectTemplate (std::string const & extraSpawnObjectTemplate)
{
	m_extraSpawnObjectTemplate = extraSpawnObjectTemplate;
}

// ----------------------------------------------------------------------

int Quest::getExtraSpawnDelay () const
{
	return m_extraSpawnDelay;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnDelay (int extraSpawnDelay)
{
	m_extraSpawnDelay = extraSpawnDelay;
}

// ----------------------------------------------------------------------

int Quest::getExtraSpawnFrequency () const
{
	return m_extraSpawnFrequency;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnFrequency (int extraSpawnFrequency)
{
	m_extraSpawnFrequency = extraSpawnFrequency;
}

// ----------------------------------------------------------------------

int Quest::getExtraSpawnLimit () const
{
	return m_extraSpawnLimit;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnLimit (int extraSpawnLimit)
{
	m_extraSpawnLimit = extraSpawnLimit;
}

// ----------------------------------------------------------------------

int Quest::getExtraSpawnRadius () const
{
	return m_extraSpawnRadius;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnRadius (int extraSpawnRadius)
{
	m_extraSpawnRadius = extraSpawnRadius;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraSpawnAction1 () const
{
	return m_extraSpawnAction1;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnAction1 (std::string const & extraSpawnAction1)
{
	m_extraSpawnAction1 = extraSpawnAction1;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraSpawnAction2 () const
{
	return m_extraSpawnAction2;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnAction2 (std::string const & extraSpawnAction2)
{
	m_extraSpawnAction2 = extraSpawnAction2;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraSpawnAction3 () const
{
	return m_extraSpawnAction3;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnAction3 (std::string const & extraSpawnAction3)
{
	m_extraSpawnAction3 = extraSpawnAction3;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraSpawnAction4 () const
{
	return m_extraSpawnAction4;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnAction4 (std::string const & extraSpawnAction4)
{
	m_extraSpawnAction4 = extraSpawnAction4;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraSpawnScript () const
{
	return m_extraSpawnScript;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnScript (std::string const & extraSpawnScript)
{
	m_extraSpawnScript = extraSpawnScript;
}

// ----------------------------------------------------------------------

std::string const & Quest::getExtraSpawnEncounterDataTable () const
{
	return m_extraSpawnEncounterDataTable;
}

// ----------------------------------------------------------------------

void Quest::setExtraSpawnEncounterDataTable (std::string const & extraSpawnEncounterDataTable)
{
	m_extraSpawnEncounterDataTable = extraSpawnEncounterDataTable;
}

// ----------------------------------------------------------------------

std::string const & Quest::getGatingObjvar () const
{
	return m_gatingObjvar;
}

// ----------------------------------------------------------------------

void Quest::setGatingObjvar (std::string const & gatingObjvar)
{
	m_gatingObjvar = gatingObjvar;
}

// ----------------------------------------------------------------------

std::string const & Quest::getGatingObjectTemplate () const
{
	return m_gatingObjectTemplate;
}

// ----------------------------------------------------------------------

void Quest::setGatingObjectTemplate (std::string const & gatingObjectTemplate)
{
	m_gatingObjectTemplate = gatingObjectTemplate;
}

// ----------------------------------------------------------------------

std::string const & Quest::getGatingFaction () const
{
	return m_gatingFaction;
}

// ----------------------------------------------------------------------

void Quest::setGatingFaction (std::string const & gatingFaction)
{
	m_gatingFaction = gatingFaction;
}

// ----------------------------------------------------------------------

int Quest::getGatingFactionValue () const
{
	return m_gatingFactionValue;
}

// ----------------------------------------------------------------------

void Quest::setGatingFactionValue (int gatingFactionValue)
{
	m_gatingFactionValue = gatingFactionValue;
}

// ----------------------------------------------------------------------

void Quest::verify (int const index, std::string & result)
{
	if (getQuestNpcScript ().empty ())
	{
		CString buffer;
		buffer.Format ("Quest %i: quest npc script is empty\r\n", index);
		result += std::string (buffer);
	}

	if (getQuestNpcEntry ().empty () || getQuestNpcEntry () == "none")
	{
		CString buffer;
		buffer.Format ("Quest %i: quest target template is empty\r\n", index);
		result += std::string (buffer);
	}

	if (getQuestType () == "deliver" && getDeliverObjectTemplate ().empty ())
	{
		CString buffer;
		buffer.Format ("Quest %i: deliver mission specified, but deliver object template is empty\r\n", index);
		result += std::string (buffer);
	}

	if ((getQuestType () == "fetch" || getQuestType () == "retrieve") && getRetrieveObjectTemplate ().empty ())
	{
		CString buffer;
		buffer.Format ("Quest %i: retrieve mission specified, but retrieve object template is empty\r\n", index);
		result += std::string (buffer);
	}

	if (getPlayerScript ().empty ())
	{
		CString buffer;
		buffer.Format ("Quest %i: player script is empty\r\n", index);
		result += std::string (buffer);
	}
}

// ----------------------------------------------------------------------

static bool getNextToken (CString const & line, CString & output, int & currentPosition)
{
	int index = line.Find ('\t', currentPosition);
	if (index == -1)
	{
		if (currentPosition < line.GetLength ())
		{
			output = line.Right (line.GetLength () - currentPosition);
			currentPosition = line.GetLength ();

			return true;
		}

		return false;
	}

	output = line.Mid (currentPosition, index - currentPosition);
	currentPosition = index + 1;

	return true;
}

// ----------------------------------------------------------------------

void Quest::read (std::string const & line)
{
	CString s;
	int currentPosition = 0;

	getNextToken (line.c_str (), s, currentPosition);
	setQuestType (std::string (s));

	getNextToken (line.c_str (), s, currentPosition);
	//setConversationStfName (std::string (s));

	getNextToken (line.c_str (), s, currentPosition);
	setMissionConversationType (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setQuestNpcConversationType (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setOverallObjvar (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setTempObjvar (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setDeliverObjectTemplate (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setRetrieveObjectTemplate (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setPlayerScript (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setQuestNpcScript (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setQuestNpcEntry (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setQuestNpcName (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward1ObjectTemplate (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward1Objvar (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward1ObjvarValue (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward2ObjectTemplate (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward2Objvar (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward2ObjvarValue (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward3ObjectTemplate (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward3Objvar (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward3ObjvarValue (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward4ObjectTemplate (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward4Objvar (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward4ObjvarValue (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward1Faction (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward1FactionValue (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setReward2Faction (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward2FactionValue (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setReward3Faction (std::string (s));

	getNextToken (line.c_str (), s, currentPosition);
	setReward3FactionValue (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setReward4Faction (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setReward4FactionValue (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setCredits (atoi (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setLocationX (static_cast<float> (atof (s)));

	getNextToken (line.c_str (), s, currentPosition);
	setLocationY (static_cast<float> (atof (s)));

	getNextToken (line.c_str (), s, currentPosition);
	setLocationZ (static_cast<float> (atof (s)));

	getNextToken (line.c_str (), s, currentPosition);
	setLocationPlanet (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setQuestLeg (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setExtraNpc1Entry (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraNpc1Disposition (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraNpc2Entry (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraNpc2Disposition (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraNpc3Entry (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraNpc3Disposition (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraNpc4Entry (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraNpc4Disposition (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnObjectTemplate (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnDelay (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnFrequency (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnLimit (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnRadius (atoi (s));

	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnScript (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnAction1 (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnAction2 (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnAction3 (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnAction4 (std::string (s));
	
	getNextToken (line.c_str (), s, currentPosition);
	setExtraSpawnEncounterDataTable (std::string (s));

	getNextToken (line.c_str (), s, currentPosition);
	setGatingObjvar (std::string (s));

	getNextToken (line.c_str (), s, currentPosition);
	setGatingObjectTemplate (std::string (s));

	getNextToken (line.c_str (), s, currentPosition);
	setGatingFaction (std::string (s));

	getNextToken (line.c_str (), s, currentPosition);
	setGatingFactionValue (atoi (s));
}

// ----------------------------------------------------------------------

void Quest::write (std::string & output)
{
	CString buffer;

	output += getQuestType ();
	output += '\t';

	output += getConversationStfName ();
	output += '\t';

	output += getMissionConversationType ();
	output += '\t';

	output += getQuestNpcConversationType ();
	output += '\t';

	output += getOverallObjvar ();
	output += '\t';

	output += getTempObjvar ();
	output += '\t';

	output += getDeliverObjectTemplate ();
	output += '\t';

	output += getRetrieveObjectTemplate ();
	output += '\t';

	output += getPlayerScript ();
	output += '\t';

	output += getQuestNpcScript ();
	output += '\t';

	output += getQuestNpcEntry ();
	output += '\t';

	output += getQuestNpcName ();
	output += '\t';

	output += getReward1ObjectTemplate ();
	output += '\t';

	output += getReward1Objvar ();
	output += '\t';

	output += !getReward1Objvar ().empty () ? getReward1ObjvarValue () : "";
	output += '\t';

	output += getReward2ObjectTemplate ();
	output += '\t';

	output += getReward2Objvar ();
	output += '\t';

	output += !getReward2Objvar ().empty () ? getReward2ObjvarValue () : "";
	output += '\t';

	output += getReward3ObjectTemplate ();
	output += '\t';

	output += getReward3Objvar ();
	output += '\t';

	output += !getReward3Objvar ().empty () ? getReward3ObjvarValue () : "";
	output += '\t';

	output += getReward4ObjectTemplate ();
	output += '\t';

	output += getReward4Objvar ();
	output += '\t';

	output += !getReward4Objvar ().empty () ? getReward4ObjvarValue () : "";
	output += '\t';

	output += getReward1Faction ();
	output += '\t';

	if (getReward1Faction () != "none")
	{
		buffer.Format ("%i", getReward1FactionValue ());
		output += buffer;
	}
	output += '\t';

	output += getReward2Faction ();
	output += '\t';

	if (getReward2Faction () != "none")
	{
		buffer.Format ("%i", getReward2FactionValue ());
		output += buffer;
	}
	output += '\t';

	output += getReward3Faction ();
	output += '\t';

	if (getReward3Faction () != "none")
	{
		buffer.Format ("%i", getReward3FactionValue ());
		output += buffer;
	}
	output += '\t';

	output += getReward4Faction ();
	output += '\t';

	if (getReward4Faction () != "none")
	{
		buffer.Format ("%i", getReward4FactionValue ());
		output += buffer;
	}
	output += '\t';

	buffer.Format ("%i", getCredits ());
	output += buffer;
	output += '\t';

	if (getLocationPlanet () != "none")
	{
		buffer.Format ("%1.2f", getLocationX ());
		output += buffer;
	}
	output += '\t';

	if (getLocationPlanet () != "none")
	{
		buffer.Format ("%1.2f", getLocationY ());
		output += buffer;
	}
	output += '\t';

	if (getLocationPlanet () != "none")
	{
		buffer.Format ("%1.2f", getLocationZ ());
		output += buffer;
	}
	output += '\t';

	output += getLocationPlanet ();
	output += '\t';

	buffer.Format ("%i", getQuestLeg ());
	output += buffer;
	output += '\t';

	output += getExtraNpc1Entry ();
	output += '\t';

	output += getExtraNpc1Disposition ();
	output += '\t';

	output += getExtraNpc2Entry ();
	output += '\t';

	output += getExtraNpc2Disposition ();
	output += '\t';

	output += getExtraNpc3Entry ();
	output += '\t';

	output += getExtraNpc3Disposition ();
	output += '\t';

	output += getExtraNpc4Entry ();
	output += '\t';

	output += getExtraNpc4Disposition ();
	output += '\t';

	output += getExtraSpawnObjectTemplate ();
	output += '\t';

	buffer.Format ("%i", getExtraSpawnDelay ());
	output += buffer;
	output += '\t';

	buffer.Format ("%i", getExtraSpawnFrequency ());
	output += buffer;
	output += '\t';

	buffer.Format ("%i", getExtraSpawnLimit ());
	output += buffer;
	output += '\t';

	buffer.Format ("%i", getExtraSpawnRadius ());
	output += buffer;
	output += '\t';

	output += getExtraSpawnScript ();
	output += '\t';

	output += getExtraSpawnAction1 ();
	output += '\t';

	output += getExtraSpawnAction2 ();
	output += '\t';

	output += getExtraSpawnAction3 ();
	output += '\t';

	output += getExtraSpawnAction4 ();
	output += '\t';

	output += getExtraSpawnEncounterDataTable ();
	output += '\t';

	output += getGatingObjvar ();
	output += '\t';

	output += getGatingObjectTemplate ();
	output += '\t';

	output += getGatingFaction ();
	output += '\t';

	if (getGatingFaction () != "none")
	{
		buffer.Format ("%i", getGatingFactionValue ());
		output += buffer;
	}

	output += '\n';
}

// ======================================================================

