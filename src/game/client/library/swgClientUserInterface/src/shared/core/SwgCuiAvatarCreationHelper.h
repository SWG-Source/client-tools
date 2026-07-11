//======================================================================
//
// SwgCuiAvatarCreationHelper.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAvatarCreationHelper_H
#define INCLUDED_SwgCuiAvatarCreationHelper_H

//======================================================================

#include "swgSharedUtility/Attributes.def"

#include "StringId.h"

//----------------------------------------------------------------------

class CreatureObject;
class Iff;
class NetworkId;
class StringId;
class TangibleObject;

//----------------------------------------------------------------------

class SwgCuiAvatarCreationHelper
{
public:

	struct Messages
	{
		struct CreationFinished;
		struct RandomNameChanged
		{
			typedef Unicode::String Payload;
		};

		struct Aborted;

		struct VerifyAndLockNameResponse
		{
			struct Payload
			{
				bool success;
				Unicode::String name;
				StringId errorMessage;
			};
		};
	};

	typedef stdvector<CreatureObject *>::fwd CreatureVector;

	static void                      install                  ();
	static void                      remove                   ();
	static bool                      finishCreation           (bool automatic = false);
	static void                      VerifyAndLockName();

	static CreatureObject *          setCreature              (CreatureObject & obj);
	static CreatureObject *          getCreature              ();
	static CreatureObject *          duplicateCreature        (const CreatureObject & basea);
	static CreatureObject *          duplicateCreatureWithClothesAndCustomization(const CreatureObject & basea, bool includeAppearanceItems = true);

	static const std::string &       getProfession            ();
	static void                      setProfession            (const std::string &);

	static const std::string &       getStartingPlanet        ();
	static void                      setStartingPlanet        (const std::string &);

	static void                      getCreaturesFromPool     (CreatureVector & cv, int n);

	static void                      purgePool                (bool andThePedestalToo = false);

	static void                      purgeExtraPoolMembers    ();

	static void                      onCompleted              (bool success, const NetworkId & networkId);

	static void                      requestRandomName        (bool showDialog);
	static const Unicode::String &   getRandomName            ();

	static void                      onRandomNameReceived     (const std::string & serverTemplateName, const Unicode::String & name, const StringId & errorMessage);

	static void                      onClientVerifyAndLockNameResponseReceived(Unicode::String const & name, StringId const & errorMessage);

	static bool                      getCreatureCustomized    ();
	static void                      setCreatureCustomized    (bool b);

	static void                      setBiography             (const Unicode::String & str);
	static const Unicode::String &   getBiography             ();

	static bool                      wasLastCreationAutomatic (Unicode::String & name);

	static TangibleObject *          getPedestal              ();
	static float                     getPedestalOffsetFromTop ();

	static bool                      areAllDetailLevelsAvailable (const CreatureObject & creature);

	static bool                      isCreatingJedi           ();
	static void                      setCreatingJedi          (bool b);

	static void                      restartMusic             (bool onlyIfNotPlaying);
	static void                      stopMusic                (float fadeout, bool restartMainMusic);

	static std::string               convertSharedToServer    (const std::string & objectTemplateName);

private:

	static void                      unlinkPoolCustomizations ();

private:

	static void                      copyHair                 (CreatureObject &src, CreatureObject &dest, bool assignClientId = false);
	static bool                      ms_isCreatingJedi;
};

//----------------------------------------------------------------------

inline bool SwgCuiAvatarCreationHelper::isCreatingJedi           ()
{
	return ms_isCreatingJedi;
}

//======================================================================

#endif
