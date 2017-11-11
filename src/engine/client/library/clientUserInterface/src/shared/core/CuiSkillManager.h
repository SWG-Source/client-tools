// ======================================================================
//
// CuiSkillManager.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiSkillManager_H
#define INCLUDED_CuiSkillManager_H

// ======================================================================

class CreatureObject;
class SkillObject;

//-----------------------------------------------------------------

class CuiSkillManager
{
public:

	typedef stdvector<Unicode::String>::fwd      StringVector;

	//----------------------------------------------------------------------

	struct Messages
	{
		struct SelectedCurrentWorkingSkillChanged
		{
			typedef std::string    Payload;
		};

		struct ProfessionTemplateChanged
		{
			typedef std::string    Payload;
		};
	};

	//----------------------------------------------------------------------

	struct StringTables
	{
		struct Skill
		{
			static const std::string names;
			static const std::string descs;
			static const std::string titles;
			static const std::string categoryNames;
		};

		struct SkillMod
		{
			static const std::string names;
			static const std::string descs;
			static const std::string extendedDescs;
		};
		
		struct Exp
		{
			static const std::string names;
			static const std::string descs;
		};

		struct Cmd
		{
			static const std::string names;
			static const std::string descs;
		};

		struct Stats
		{
			static const std::string names;
			static const std::string descs;
		};
	};

	//----------------------------------------------------------------------

	static void                 updateExperience     (bool showMessage);

	static bool                 localizeSkillName        (const SkillObject & skill, Unicode::String & str, bool forceEnglish = false);
	static bool                 localizeSkillDescription (const SkillObject & skill, Unicode::String & str);

	static bool                 localizeSkillName        (const std::string & skill, Unicode::String & str, bool forceEnglish = false);
	static bool                 localizeSkillDescription (const std::string & skill, Unicode::String & str);
	static bool                 localizeSkillTitle       (const std::string & title, Unicode::String & str);

	static bool                 localizeExpName          (const std::string & expName, Unicode::String & str);
	static bool                 localizeExpDescription   (const std::string & expName, Unicode::String & str);

	static bool                 localizeCmdName          (const std::string & cmdName, Unicode::String & str);
	static bool                 localizeCmdDescription   (const std::string & cmdName, Unicode::String & str);

	static bool                 localizeSkillModName     (const std::string & skillModName, Unicode::String & str);
	static bool                 localizeSkillModDesc     (const std::string & skillModName, Unicode::String & str);
	static bool                 localizeSkillModExtendedDesc     (const std::string & skillModName, Unicode::String & str);

	static void                 formatSkillExpDescription   (const SkillObject & skill, Unicode::String & str, bool showPlayerInfo, bool useLineBreak);
	static void                 formatSkillExpDescriptions  (const SkillObject & skill, StringVector & sv, bool showPlayerInfo, bool useLineBreak);
	static void install                  ();
	static void remove                   ();

	static void                 testSkillSystem (bool verbose);

	static std::string const & getSkillTemplate();
	static void setSkillTemplate(std::string const & templateName, bool sendToServer = true);
	static void clearCachedSkillTemplate();
	
	static std::string const & getWorkingSkill();
	static void setWorkingSkill(std::string const & skillName, bool sendToServer = true);

	static SkillObject const * getWorkingSkillObject();

	static void enableEffects(bool enabled);
	static void triggerSkillEffects();
private:

	CuiSkillManager (const CuiSkillManager & rhs);
	CuiSkillManager & operator= (const CuiSkillManager & rhs);

	static std::string m_skillTemplate;
	static std::string m_workingSkill;
};

// ======================================================================

#endif
