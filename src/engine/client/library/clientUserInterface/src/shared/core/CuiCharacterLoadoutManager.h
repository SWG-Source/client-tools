//======================================================================
//
// CuiCharacterLoadoutManager.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiCharacterLoadoutManager_H
#define INCLUDED_CuiCharacterLoadoutManager_H

//======================================================================

class ClientObject;

//----------------------------------------------------------------------

class CuiCharacterLoadoutManager
{
public:
	static void                   install ();
	static void                   remove ();

	typedef std::pair<int, std::string>             ArrangementTemplatePair;
	typedef stdvector<ArrangementTemplatePair>::fwd LoadoutVector;

	static const LoadoutVector *  getLoadout         (const std::string & playerTemplate);
	static bool                   setupLoadout       (ClientObject & obj);
	static void                   setLoadoutFilename (const std::string & filename);

	static void                   clear ();
	static void                   init  ();

	static bool                   test (std::string & result, const stdvector<std::string>::fwd & templateNames);

private:
	typedef stdmap<std::string, LoadoutVector>::fwd LoadoutMap;
	static LoadoutMap *         ms_loadoutMap;
	static std::string          ms_filename;

	static bool                 initializeLoadoutMap (const std::string & filename);
};

//======================================================================

#endif
