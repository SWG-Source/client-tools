//======================================================================
//
// CuiCharacterHairManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiCharacterHairManager_H
#define INCLUDED_CuiCharacterHairManager_H

class CreatureObject;
class TangibleObject;
class ClientObject;

//======================================================================

class CuiCharacterHairManager
{
public:
	typedef stdvector<std::string>::fwd StringVector;

	static bool                   getAvailableStylesSkillMod (std::string const & templateName, int skillModValue, StringVector & /*OUT*/result);
	static bool                   getAvailableStylesCreation (std::string const & templateName, StringVector & /*OUT*/result, std::string & /*OUT*/defaultHair);
	static bool                   loadHairStyles     (std::string const & templateName, StringVector & /*OUT*/ result, std::string & /*OUT*/ defaultHair, int hairSkillModValue);
	static bool                   setupDefaultHair   (ClientObject & obj);
	static int                    getIndexForHair    (std::string const & templateName);
	static std::string            getHairForIndex    (int index);
	static void                   replaceHair        (CreatureObject & obj, TangibleObject * srcHair, bool destroyOldHair);
};

//======================================================================

#endif
