// ======================================================================
//
// Configuration.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_Configuration_H
#define INCLUDED_Configuration_H

class Vector;

// ======================================================================

class Configuration
{
public:

	class MissionTemplate
	{
		friend class Configuration;

	public:

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		enum JournalType
		{
			JT_invalid,
			JT_default,
			JT_generic,
			JT_genericLocationTitle,
			JT_navigationPoints,
			JT_spawners
		};

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		class PropertyTemplate
		{
		public:

			enum PropertyType
			{
				PT_unknown,
				PT_notImplemented,
				PT_bool,
				PT_integer,
				PT_string,
				PT_enumList,
				PT_questName,
				PT_objectTemplate,
				PT_spaceZone,
				PT_spaceFaction,
				PT_spaceMobile,
				PT_spaceMobileList,
				PT_navPoint,
				PT_navPointList,
				PT_spawner,
				PT_spawnerList,
				PT_cargo
			};

		public:

			PropertyTemplate();
			~PropertyTemplate();

			void parseIniString(CString const & iniString);

			CString const getPropertyTemplateFormatString() const;
			CString const getPropertyData(int index) const;

		public:

			bool m_required;
			CString m_columnName;
			CString m_dataTableHeader;
			PropertyType m_propertyType;
			CString m_propertyData;
			StringList m_propertyDataList;
			CString m_toolTipText;

		private:

			CString const getPropertyTypeString() const;
		};

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		class StringTemplate
		{
		public:

			StringTemplate();
			~StringTemplate();

			void parseIniString(CString const & iniString);

			CString const getStringTemplateFormatString() const;

		public:

			bool m_required;
			CString m_stringId;
			CString m_toolTipText;
		};

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		class QuestStringSuffixTemplate
		{
		public:

			QuestStringSuffixTemplate();
			~QuestStringSuffixTemplate();

			void parseIniString(CString const & iniString);

			CString const getQuestStringSuffixTemplateFormatString() const;

		public:

			CString m_suffix;
			CString m_toolTipText;
		};

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	public:

		explicit MissionTemplate(char const * name);
		~MissionTemplate();

		CString const & getName() const;

		bool getAllowNavPointsInMultipleZones() const;

		CString const & getInformation() const;

		int getNumberOfStringTemplates() const;
		StringTemplate const & getStringTemplate(int index) const;
		StringTemplate const * getStringTemplate(CString const & key) const;

		int getNumberOfPropertyTemplates() const;
		PropertyTemplate const & getPropertyTemplate(int index) const;
		PropertyTemplate const * getPropertyTemplate(CString const & key) const;

		int getNumberOfQuestStringSuffixTemplates() const;
		QuestStringSuffixTemplate const & getQuestStringSuffixTemplate(int index) const;
		bool isQuestStringSuffixTemplate(CString const & key) const;

		CString const getMissionTemplateFormatString() const;

		void setJournalType(CString const & journalType);
		JournalType getJournalType() const;
	
	private:

		MissionTemplate();

	private:

		CString m_name;

		bool m_allowNavPointsInMultipleZones;

		CString m_information;

		typedef std::vector<PropertyTemplate> PropertyTemplateList;
		PropertyTemplateList m_propertyTemplateList;

		typedef std::vector<StringTemplate> StringTemplateList;
		StringTemplateList m_stringTemplateList;

		typedef std::vector<QuestStringSuffixTemplate> QuestStringSuffixTemplateList;
		QuestStringSuffixTemplateList m_questStringSuffixTemplateList;

		JournalType m_journalType;
	};

public:

	static bool install();
	static void reload();

	static CString const getConfiguration();
	static CString const & getSpaceQuestDirectory();

	static CString const extractMissionTemplateType(CString pathName);
	static CString const extractRootName(CString pathName);

	static CString const createServerMissionDataTableName(CString const & rootName, CString const & missionTemplateType);
	static CString const createSharedStringTableName(CString const & rootName, CString const & missionTemplateType);
	static CString const createSharedQuestListDataTableName(CString const & rootName, CString const & missionTemplateType);
	static CString const createSharedQuestTaskDataTableName(CString const & rootName, CString const & missionTemplateType);

	//-- From cfg
	static CString const & getServerMissionDataTablePath();
	static CString const & getSharedStringFilePath();
	static CString const getSharedQuestListDataTablePath();
	static CString const getSharedQuestTaskDataTablePath();

	//-- From ini
	static int getNumberOfStringButtons();
	static CString const & getStringButtonName(int index);
	static CString const & getStringButtonData(int index);

	static int getNumberOfSpaceZones();
	static CString const & getSpaceZone(int index);
	static CString const getSpaceZoneList();
	static int getSpaceZoneIndex(CString const & key);
	static bool isValidSpaceZone(CString const & key);

	static bool isValidNavPoint(CString const & spaceZone, CString const & navPoint, bool allowNavPointsInMultipleZones);
	static bool isValidSpawner(CString const & spaceZone, CString const & spawner);

	static int getNumberOfSpaceMobiles();
	static CString const & getSpaceMobile(int index);
	static bool isValidSpaceMobile(CString const & spaceMobile);
	static CString const getSpaceMobileCargoName(CString const & spaceMobile);

	static int getNumberOfSpaceFactions();
	static CString const & getSpaceFaction(int index);
	static CString const getSpaceFactionList();
	static int getSpaceFactionIndex(CString const & key);
	static bool isValidSpaceFaction(CString const & key);

	static int getNumberOfQuestCategories();
	static CString const & getQuestCategory(int index);
	static CString const getQuestCategoryList();
	static int getQuestCategoryIndex(CString const & key);
	static bool isValidQuestCategory(CString const & key);

	static StringList const * getCargo(CString const & cargoName);

	static int getNumberOfMissionTemplates();
	static MissionTemplate const * getMissionTemplate(int index);
	static MissionTemplate const * getMissionTemplate(char const * missionTemplateType);
	static bool isValidMissionTemplateType(char const * missionTemplateType);

	static void populateNavPoints(CTreeCtrl & treeCtrl);
	static void populateSpawners(CTreeCtrl & treeCtrl);

	static void packString(StringList const & stringList, CString & string, char separator);
	static void unpackString(CString const & string, StringList & stringList, char separator);

	static Vector getNavPointLocation(CString const & spaceZone, CString const & navPoint);

private:

	static void remove();

	static bool loadCfg();
	static bool loadIni();

	static void loadCargo(CString const & cargoName);
};

// ======================================================================

#endif

