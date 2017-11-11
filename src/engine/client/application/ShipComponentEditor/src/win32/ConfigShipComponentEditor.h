//======================================================================
//
// ConfigShipComponentEditor.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ConfigShipComponentEditor_H
#define INCLUDED_ConfigShipComponentEditor_H

//======================================================================

class ConfigShipComponentEditor
{
public:

	static void install();

	static std::string const & getServerPathDsrc();
	static std::string const & getServerPathData();
	static std::string const & getSharedPathDsrc();
	static std::string const & getSharedPathData();
	static std::string const & getServerTemplateCrcStringTable();
	static std::string const & getClientDataFilePathDsrc();

	static std::string const & getTextEditorPath();
	static std::string const & getToolPath();
	static std::string const & getBranchName();

	static bool testFolders(std::string & result);
};

//======================================================================

#endif
