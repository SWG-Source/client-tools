//======================================================================
//
// CuiCachedAvatarManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiCachedAvatarManager_H
#define INCLUDED_CuiCachedAvatarManager_H

//======================================================================

class CreatureObject;
class Iff;
class NetworkId;

//----------------------------------------------------------------------

class CuiCachedAvatarManager
{
public:

	struct CachedAvatarInfo
	{
		CachedAvatarInfo() : m_planetName(""), m_skillTemplate(""), m_level(0) {}

		std::string m_planetName;
		std::string m_skillTemplate;
		int m_level;
	};

	static bool              saveAvatar      (const CreatureObject & avatar, const NetworkId & id, const std::string & userName, const std::string & clusterName);
	static bool              loadAvatar      (CreatureObject & avatar,       CachedAvatarInfo & info,       const NetworkId & id, const std::string & userName, const std::string & clusterName);
	static bool              saveAvatar      (const CreatureObject & avatar, const CachedAvatarInfo & info, const NetworkId & id, const std::string & userName, const std::string & clusterName);

	static bool              loadCreature    (CreatureObject * creature,       CachedAvatarInfo & info,       const std::string & filename);
	static bool              saveCreature    (const CreatureObject & creature, const CachedAvatarInfo & info, const std::string & filename);

	static bool              getCachedAvatarInfo (const NetworkId &id, const std::string & userName, const std::string & clusterName, CachedAvatarInfo & info);

	static const std::string createPath      (const CreatureObject * avatar, const NetworkId & id, const std::string & userName, const std::string & clusterName);

	static bool              deleteCache     (const CreatureObject & avatar, const NetworkId & id, const std::string & userName, const std::string & clusterName);

	static bool              saveScenePlayer ();

	static void              update          (float deltaTimeSecs);

	static void              saveScenePlayerNextUpdate  ();

	static void              install                    ();
	static void              remove                     ();

	//-- launchpad character list support

	static void              addToCharacterList          (const uint32 clusterId, const NetworkId & id, const Unicode::String & name, const std::string & planetName);
	static void              setCharacterListPlanet      (const uint32 clusterId, const NetworkId & id, const std::string & planet);
	static void              setCharacterListLastPlayed  (const uint32 clusterId, const NetworkId & id);
	static void              saveCharacterList           ();

	static void              removeAvatar                (const NetworkId & id, uint32 clusterId);

private:
	static bool load_0000 (Iff & iff, CreatureObject * creature, std::string & planetName);
	static bool load_0001 (Iff & iff, CreatureObject * creature, CachedAvatarInfo & info);
	static bool save_0000 (Iff & iff, const CreatureObject & creature, const std::string & planetName);
	static bool save_0001 (Iff & iff, const CreatureObject & creature, const CachedAvatarInfo & info);


};

//======================================================================

#endif
