// ======================================================================
//
// Configuration.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "Configuration.h"

// ======================================================================

namespace ConfigurationNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

#if 1
	CString const cms_root = "../../exe/win32/";
#else
	CString const cms_root = "";
#endif

	CString const cms_empty = "_none";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	typedef std::vector<CString> StringList;
	StringList ms_animationActions;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void populate (StringList const & stringList, CComboBox & comboBox)
	{
		comboBox.ResetContent ();

		StringList::const_iterator end = stringList.end ();
		for (StringList::const_iterator iter = stringList.begin (); iter != end; ++iter)
			comboBox.AddString (*iter);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
}

using namespace ConfigurationNamespace;

// ======================================================================

bool Configuration::install ()
{
	if (!loadIni ())
		return false;

	return true;
}

// ----------------------------------------------------------------------

void Configuration::populateAnimationActions (CComboBox & comboBox)
{
	populate (ms_animationActions, comboBox);
}

// ----------------------------------------------------------------------

CString const & Configuration::getEmptyAnimationAction ()
{
	return cms_empty;
}

// ======================================================================

bool Configuration::loadIni ()
{
	//-- open the config file
	CStdioFile infile;
	if (!infile.Open (cms_root + "SwgConversationEditor.ini", CFile::modeRead | CFile::typeText))
		return false;

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		//-- see if the line is empty
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		//-- see if the first character is a comment
		int index = line.Find ("#");
		if (index == 0)
			continue;

		//-- find the =
		index = line.Find ("=");
		if (index == -1)
			continue;

		//-- left half goes in key, right half goes in value
		int const length = line.GetLength ();
		CString const left = line.Left (index);
		CString right = line.Right (length - index - 1);

		if (left == "animationAction")
			ms_animationActions.push_back (right);
	}

	if (ms_animationActions.empty ())
		return false;

	return true;
}

// ======================================================================

