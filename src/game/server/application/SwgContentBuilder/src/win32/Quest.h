// ======================================================================
//
// Quest.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_Quest_H
#define INCLUDED_Quest_H

// ======================================================================

#include <string>

// ======================================================================

class Quest
{
public:

	Quest ();
	~Quest ();

	void read (std::string const & line);
	void write (std::string & output);

	void verify (int index, std::string & result);

	std::string const & getConversationStfName () const;
	void                setConversationStfName (std::string const & conversationStfName);
	std::string const & getQuestType () const;
	void                setQuestType (std::string const & questType);
	std::string const & getOverallObjvar () const;
	void                setOverallObjvar (std::string const & overallObjvar);
	std::string const & getTempObjvar () const;
	void                setTempObjvar (std::string const & tempObjvar);
	std::string const & getDeliverObjectTemplate () const;
	void                setDeliverObjectTemplate (std::string const & deliverObjectTemplate);
	std::string const & getRetrieveObjectTemplate () const;
	void                setRetrieveObjectTemplate (std::string const & retrieveObjectTemplate);
	std::string const & getPlayerScript () const;
	void                setPlayerScript (std::string const & playerScript);
	int  getQuestLeg () const;
	void setQuestLeg (int questLeg);
	std::string const & getQuestNpcEntry () const;
	void                setQuestNpcEntry (std::string const & questNpcEntry);
	std::string const & getQuestNpcScript () const;
	void                setQuestNpcScript (std::string const & questNpcScript);
	std::string const & getQuestNpcName () const;
	void                setQuestNpcName (std::string const & questNpcName);
	std::string const & getMissionConversationType () const;
	void                setMissionConversationType (std::string const & missionConversationType);
	std::string const & getQuestNpcConversationType () const;
	void                setQuestNpcConversationType (std::string const & questNpcConversationType);

	std::string const & getLocationPlanet () const;
	void                setLocationPlanet (std::string const & locationPlanet);
	float getLocationX () const;
	void  setLocationX (float locationX);
	float getLocationY () const;
	void  setLocationY (float locationY);
	float getLocationZ () const;
	void  setLocationZ (float locationZ);

	int  getCredits () const;
	void setCredits (int credits);

	std::string const & getReward1ObjectTemplate () const;
	void                setReward1ObjectTemplate (std::string const & reward1ObjectTemplate);
	std::string const & getReward1Objvar () const;
	void                setReward1Objvar (std::string const & reward1Objvar);
	std::string const & getReward1ObjvarValue () const;
	void                setReward1ObjvarValue (std::string const & reward1ObjvarValue);
	std::string const & getReward1Faction () const;
	void                setReward1Faction (std::string const & reward1Faction);
	int                 getReward1FactionValue () const;
	void                setReward1FactionValue (int reward1FactionValue);
	std::string const & getReward2ObjectTemplate () const;
	void                setReward2ObjectTemplate (std::string const & reward2ObjectTemplate);
	std::string const & getReward2Objvar () const;
	void                setReward2Objvar (std::string const & reward2Objvar);
	std::string const & getReward2ObjvarValue () const;
	void                setReward2ObjvarValue (std::string const & reward2ObjvarValue);
	std::string const & getReward2Faction () const;
	void                setReward2Faction (std::string const & reward2Faction);
	int                 getReward2FactionValue () const;
	void                setReward2FactionValue (int reward2FactionValue);
	std::string const & getReward3ObjectTemplate () const;
	void                setReward3ObjectTemplate (std::string const & reward3ObjectTemplate);
	std::string const & getReward3Objvar () const;
	void                setReward3Objvar (std::string const & reward3Objvar);
	std::string const & getReward3ObjvarValue () const;
	void                setReward3ObjvarValue (std::string const & reward3ObjvarValue);
	std::string const & getReward3Faction () const;
	void                setReward3Faction (std::string const & reward3Faction);
	int                 getReward3FactionValue () const;
	void                setReward3FactionValue (int reward3FactionValue);
	std::string const & getReward4ObjectTemplate () const;
	void                setReward4ObjectTemplate (std::string const & reward4ObjectTemplate);
	std::string const & getReward4Objvar () const;
	void                setReward4Objvar (std::string const & reward4Objvar);
	std::string const & getReward4ObjvarValue () const;
	void                setReward4ObjvarValue (std::string const & reward4ObjvarValue);
	std::string const & getReward4Faction () const;
	void                setReward4Faction (std::string const & reward4Faction);
	int                 getReward4FactionValue () const;
	void                setReward4FactionValue (int reward4FactionValue);

	std::string const & getExtraNpc1Entry () const;
	void                setExtraNpc1Entry (std::string const & extraNpc1Entry);
	std::string const & getExtraNpc1Disposition () const;
	void                setExtraNpc1Disposition (std::string const & extraNpc1Disposition);
	std::string const & getExtraNpc2Entry () const;
	void                setExtraNpc2Entry (std::string const & extraNpc2Entry);
	std::string const & getExtraNpc2Disposition () const;
	void                setExtraNpc2Disposition (std::string const & extraNpc2Disposition);
	std::string const & getExtraNpc3Entry () const;
	void                setExtraNpc3Entry (std::string const & extraNpc3Entry);
	std::string const & getExtraNpc3Disposition () const;
	void                setExtraNpc3Disposition (std::string const & extraNpc3Disposition);
	std::string const & getExtraNpc4Entry () const;
	void                setExtraNpc4Entry (std::string const & extraNpc4Entry);
	std::string const & getExtraNpc4Disposition () const;
	void                setExtraNpc4Disposition (std::string const & extraNpc4Disposition);
	std::string const & getExtraSpawnObjectTemplate () const;
	void                setExtraSpawnObjectTemplate (std::string const & extraSpawnObjectTemplate);
	int                 getExtraSpawnDelay () const;
	void                setExtraSpawnDelay (int extraSpawnDelay);
	int                 getExtraSpawnFrequency () const;
	void                setExtraSpawnFrequency (int extraSpawnFrequency);
	int                 getExtraSpawnLimit () const;
	void                setExtraSpawnLimit (int extraSpawnLimit);
	int                 getExtraSpawnRadius () const;
	void                setExtraSpawnRadius (int extraSpawnRadius);
	std::string const & getExtraSpawnAction1 () const;
	void                setExtraSpawnAction1 (std::string const & extraSpawnAction1);
	std::string const & getExtraSpawnAction2 () const;
	void                setExtraSpawnAction2 (std::string const & extraSpawnAction2);
	std::string const & getExtraSpawnAction3 () const;
	void                setExtraSpawnAction3 (std::string const & extraSpawnAction3);
	std::string const & getExtraSpawnAction4 () const;
	void                setExtraSpawnAction4 (std::string const & extraSpawnAction4);
	std::string const & getExtraSpawnScript () const;
	void                setExtraSpawnScript (std::string const & extraSpawnScript);
	std::string const & getExtraSpawnEncounterDataTable () const;
	void                setExtraSpawnEncounterDataTable (std::string const & extraSpawnEncounterDataTable);

	std::string const & getGatingObjvar () const;
	void                setGatingObjvar (std::string const & gatingObjvar);
	std::string const & getGatingObjectTemplate () const;
	void                setGatingObjectTemplate (std::string const & gatingObjectTemplate);
	std::string const & getGatingFaction () const;
	void                setGatingFaction (std::string const & gatingFaction);
	int                 getGatingFactionValue () const;
	void                setGatingFactionValue (int gatingFactionValue);

private:

	std::string m_conversationStfName;
	std::string m_questType;
	std::string m_overallObjvar;
	std::string m_tempObjvar;
	std::string m_deliverObjectTemplate;
	std::string m_retrieveObjectTemplate;
	std::string m_playerScript;
	int m_questLeg;
	std::string m_questNpcEntry;
	std::string m_questNpcScript;
	std::string m_questNpcName;
	std::string m_missionConversationType;
	std::string m_questNpcConversationType;

	std::string m_locationPlanet;
	float m_locationX;
	float m_locationY;
	float m_locationZ;

	int m_credits;
	std::string m_reward1ObjectTemplate;
	std::string m_reward1Objvar;
	std::string m_reward1ObjvarValue;
	std::string m_reward1Faction;
	int m_reward1FactionValue;
	std::string m_reward2ObjectTemplate;
	std::string m_reward2Objvar;
	std::string m_reward2ObjvarValue;
	std::string m_reward2Faction;
	int m_reward2FactionValue;
	std::string m_reward3ObjectTemplate;
	std::string m_reward3Objvar;
	std::string m_reward3ObjvarValue;
	std::string m_reward3Faction;
	int m_reward3FactionValue;
	std::string m_reward4ObjectTemplate;
	std::string m_reward4Objvar;
	std::string m_reward4ObjvarValue;
	std::string m_reward4Faction;
	int m_reward4FactionValue;

	std::string m_extraNpc1Entry;
	std::string m_extraNpc1Disposition;
	std::string m_extraNpc2Entry;
	std::string m_extraNpc2Disposition;
	std::string m_extraNpc3Entry;
	std::string m_extraNpc3Disposition;
	std::string m_extraNpc4Entry;
	std::string m_extraNpc4Disposition;
	std::string m_extraSpawnObjectTemplate;
	int m_extraSpawnDelay;
	int m_extraSpawnFrequency;
	int m_extraSpawnLimit;
	int m_extraSpawnRadius;
	std::string m_extraSpawnAction1;
	std::string m_extraSpawnAction2;
	std::string m_extraSpawnAction3;
	std::string m_extraSpawnAction4;
	std::string m_extraSpawnScript;
	std::string m_extraSpawnEncounterDataTable;

	std::string m_gatingObjvar;
	std::string m_gatingObjectTemplate;
	std::string m_gatingFaction;
	int m_gatingFactionValue;
};

// ======================================================================

#endif
