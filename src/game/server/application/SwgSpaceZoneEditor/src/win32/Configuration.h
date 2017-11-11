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

// ======================================================================

class Configuration
{
public:

	struct ObjVar
	{
		CString m_key;
		CString m_value;
	};

	typedef std::vector<ObjVar> ObjVarList;

	class PropertyTemplate
	{
	public:

		enum PropertyType
		{
			PT_unknown,
			PT_bool,
			PT_float,
			PT_integer,
			PT_string,
			PT_enumList,
			PT_count,
			PT_spaceMobile,
			PT_spaceMobileList,
			PT_patrolPointList,
			PT_scriptList
		};

	public:

		PropertyTemplate();
		~PropertyTemplate();

		void parseIniString(CString const & iniString);

		CString const getPropertyTemplateFormatString() const;
		CString const getPropertyData(int index) const;
		CString const getPropertyPackedObjVarTypeString() const;

	public:

		bool m_hidden;
		bool m_mangled;
		CString m_name;
		PropertyType m_propertyType;
		CString m_propertyData;
		StringList m_propertyDataList;
		CString m_toolTipText;

	private:

		CString const getPropertyTypeString() const;
	};

public:

	static bool install();

	static CString const getConfiguration();

	static CString const & getSpaceMobileDataTableFileName();

	static int getNumberOfSpaceMobiles();
	static CString const & getSpaceMobile(int index);
	static bool isValidSpaceMobile(CString const & spaceMobile);

	static int getNumberOfPropertyTemplates();
	static PropertyTemplate const & getPropertyTemplate(int index);
	static PropertyTemplate const * getPropertyTemplate(CString const & key);

	static CString const extractBranch(CString const & path);

	static void packString(StringList const & stringList, CString & string, char separator);
	static void unpackString(CString const & string, StringList & stringList, char separator);

	static void packObjVars(ObjVarList const & objVarList, CString & string);
	static void unpackObjVars(CString const & string, ObjVarList & objVarList);

private:

	static void remove();

	static bool loadCfg();
	static bool loadIni();
};

// ======================================================================

#endif

