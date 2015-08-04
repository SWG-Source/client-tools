//======================================================================
//
// CuiCachedAvatarManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCachedAvatarManager.h"

#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/WeaponObject.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "clientUserInterface/CuiPreferences.h"
#include "fileInterface/StdioFile.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Tag.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include <map>

//======================================================================

namespace
{
	namespace Paths
	{
		const std::string prefix = "profiles/";
		const std::string suffix = ".ca";
	}

	namespace Tags
	{
		const Tag CADF = TAG(C,A,D,F);
		const Tag WEAR = TAG(W,E,A,R);
		const Tag ITEM = TAG(I,T,E,M);
		const Tag PLAN = TAG(P,L,A,N);
		const Tag SkillTemplate = TAG(S,K,T,P);
		const Tag SkillLevel = TAG(S,L,V,L);
	}

	void fixFilename (std::string & filename)
	{
		size_t pos = 0;
		while ((pos = filename.find ('\\', pos)) != filename.npos)  //lint !e737 //loss of precision, msdev suxx
		{
			filename [pos] = '/';
			++pos;
		}

		pos = 0;

		// erase duplicate slashes
		while ((pos = filename.find ("//", pos)) != filename.npos)  //lint !e737 //loss of precision, msdev suxx
		{
			filename.erase (pos);
		}
	}

	//----------------------------------------------------------------------

	typedef std::pair<uint32, NetworkId> CharacterClusterId;
	
	struct CharacterListInfo
	{
		Unicode::String name;
		std::string     clusterName;
		std::string     planet;
		bool            playedLast;
	};

	typedef stdmap<CharacterClusterId, CharacterListInfo>::fwd CharacterListMap;
	CharacterListMap s_characterListMap;

	bool s_saveScenePlayerNextUpdate = false;
	
	//----------------------------------------------------------------------

	class MyCallback : public MessageDispatch::Callback
	{
	public:
		void onPlayerSetup (const CreatureObject::Messages::PlayerSetup::Payload &)
		{
			s_saveScenePlayerNextUpdate = true;
		}

		void onContainerContentsChanged (const ClientObject::Messages::ContainerMsg & msg)
		{
			ClientObject * const obj = msg.first;

			if (obj == Game::getPlayer ())
			{
				s_saveScenePlayerNextUpdate = true;
			}
		}
	};

	MyCallback s_callback;
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::install ()
{
	s_callback.connect (s_callback,  &MyCallback::onPlayerSetup,              static_cast<CreatureObject::Messages::PlayerSetup *>(0));
	s_callback.connect (s_callback,  &MyCallback::onContainerContentsChanged, static_cast<ClientObject::Messages::AddedToContainer *>(0));
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::remove ()
{
	s_callback.disconnect (s_callback,  &MyCallback::onPlayerSetup, static_cast<CreatureObject::Messages::PlayerSetup *>(0));
	s_callback.disconnect (s_callback,  &MyCallback::onContainerContentsChanged, static_cast<ClientObject::Messages::AddedToContainer *>(0));
}

//----------------------------------------------------------------------

bool CuiCachedAvatarManager::loadAvatar (CreatureObject & avatar, CachedAvatarInfo & info, const NetworkId & id, const std::string & userName, const std::string & clusterName)
{
	return loadCreature (&avatar, info, createPath (&avatar, id, userName, clusterName));
}

//----------------------------------------------------------------------

bool CuiCachedAvatarManager::getCachedAvatarInfo (const NetworkId &id, const std::string & userName, const std::string & clusterName, CachedAvatarInfo & info)
{
	return loadCreature(0, info, createPath (0, id, userName, clusterName));
}

//----------------------------------------------------------------------

bool CuiCachedAvatarManager::saveAvatar (const CreatureObject & avatar, const NetworkId & id, const std::string & userName, const std::string & clusterName)
{
	CachedAvatarInfo info;

	return saveAvatar(avatar, info, id, userName, clusterName);
}

bool CuiCachedAvatarManager::saveAvatar (const CreatureObject & avatar, const CachedAvatarInfo & info, const NetworkId & id, const std::string & userName, const std::string & clusterName)
{
	return saveCreature (avatar, info, createPath (&avatar, id, userName, clusterName));
}

//----------------------------------------------------------------------

bool CuiCachedAvatarManager::loadCreature (CreatureObject * creature, CachedAvatarInfo & info, const std::string & filename)
{
	AbstractFile * file = new StdioFile(filename.c_str(), "rb");

	if (!file->isOpen ())
	{
		delete file;
		file = 0;

		WARNING (true, ("Data file %s does not exist for reading.", filename.c_str ()));
		return false;
	}

	if (!Iff::isValid(filename.c_str()))
	{
		WARNING (true, ("Data file %s is not valid.", filename.c_str ()));
		delete file;
		return false;
	}

	Iff iff;
	iff.open (*file, filename.c_str());

	delete file;
	file = 0;

	if (!iff.enterForm (Tags::CADF, true))
		WARNING (true, ("Data file %s is not a Cached Avatar Data file.", filename.c_str ()));
	else
	{		
		switch (iff.getCurrentName())
		{
		case TAG_0000:
			if (!load_0000(iff, creature, info.m_planetName))
				WARNING (true, ("Unable to load Cached Avatar Data %s.", filename.c_str ()));
			else
				return true;
			break;

		case TAG_0001:
			if (!load_0001(iff, creature, info))
				WARNING (true, ("Unable to load Cached Avatar Data %s.", filename.c_str ()));
			else
				return true;
			break;

		default:
			{
				char tagName[5];				
				ConvertTagToString(iff.getCurrentName(), tagName);
				WARNING (true, ("unsupported Cached Avatar Data version [%s]", tagName));
			}
			break;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiCachedAvatarManager::saveCreature (const CreatureObject & creature, const CachedAvatarInfo & info, const std::string & filename)
{
	if (!creature.isInitialized ())
		return false;

	const size_t lastSlash = filename.rfind ('/');
	IGNORE_RETURN (Os::createDirectories (filename.substr (0, lastSlash).c_str ()));

	Iff iff (8196, true);
	if (!save_0001 (iff, creature, info))
		WARNING (true, ("Unable to create iff for %s", filename.c_str ()));
	else if (!iff.write (filename.c_str (), true))
		WARNING (true, ("Unable to write iff: %s", filename.c_str ()));
	else
		return true;

	return false;
}

//-----------------------------------------------------------------

bool CuiCachedAvatarManager::saveScenePlayer ()
{
	const CreatureObject * const avatar = Game::getPlayerCreature ();
	const PlayerObject * const player = Game::getPlayerObject();

	if (avatar)
	{
		Unicode::String avatarName;
		std::string loginId;
		std::string clusterName;
		NetworkId   id;
		const std::string & planetName = Game::getSceneId ();

		if (Game::getPlayerPath (loginId, clusterName, avatarName, id))
		{
			CachedAvatarInfo info;
			info.m_planetName = planetName;
			info.m_level = avatar->getLevel();

			if(player)
			{
				info.m_skillTemplate = player->getSkillTemplate();
			}

			const bool retval = saveAvatar (*avatar, info, avatar->getNetworkId (), loginId, clusterName);

			const uint32 clusterId = CuiLoginManager::getLastConnectedClusterId ();
			CuiCachedAvatarManager::setCharacterListPlanet     (clusterId, id, planetName);
			CuiCachedAvatarManager::setCharacterListLastPlayed (clusterId, id);
			CuiCachedAvatarManager::saveCharacterList          ();
			return retval;
		}
	}

	return false;
}

//-----------------------------------------------------------------

bool CuiCachedAvatarManager::deleteCache (const CreatureObject & avatar, const NetworkId & id, const std::string & userName, const std::string & clusterName)
{
	const std::string path (createPath (&avatar, id, userName, clusterName));
	UNREF (path);

	//-- @todo delete the file when we have an Os wrapper for it
	return false;
}

//----------------------------------------------------------------------

const std::string CuiCachedAvatarManager::createPath (const CreatureObject * avatar, const NetworkId & id, const std::string & userName, const std::string & clusterName)
{
	NetworkId theId = id;
	if (avatar && theId == NetworkId::cms_invalid)
		theId = avatar->getNetworkId ();

	return Paths::prefix + userName + "/" + clusterName + "/" + theId.getValueString () + Paths::suffix;
}

//----------------------------------------------------------------------

bool CuiCachedAvatarManager::load_0000 (Iff & iff, CreatureObject * creature, std::string & planetName)
{
	UNREF (creature);
	iff.enterForm(TAG_0000);
	
	iff.enterChunk (TAG_DATA);
	{
		if (creature)
		{
			const std::string templateName = iff.read_stdstring ();
			std::string fixedTemplateName (creature->getObjectTemplateName ());
			fixFilename (fixedTemplateName);
			
			if (templateName != fixedTemplateName)
			{
				WARNING (true, ("Mismatched object template names."));
				return false;
			}
			const std::string appearanceString = iff.read_stdstring ();
			creature->setAppearanceData (appearanceString);
		}
	}
	iff.exitChunk (TAG_DATA, true);

	//-- load the planet name
	if (iff.enterChunk (Tags::PLAN, true))
	{
		planetName = iff.read_stdstring ();
		iff.exitChunk (Tags::PLAN);
	}
	else
		planetName.clear ();

	if (creature && iff.enterForm (Tags::WEAR, true))
	{
		while (iff.enterChunk (Tags::ITEM, true))
		{
			const std::string templateName     = iff.read_stdstring ();
			const std::string appearanceString = iff.read_stdstring ();

			Object * const object = ObjectTemplate::createObject (templateName.c_str ());

			if (!object)
			{
				WARNING (true, ("Unable to create cached inventory object '%s'", templateName.c_str ()));
			}
			else
			{

				TangibleObject * tangible = dynamic_cast<TangibleObject *>(object);
				if (!tangible)
				{
					WARNING (true, ("Cached inventory object '%s' is not tangible", templateName.c_str ()));
				}
				else
				{
					tangible->endBaselines ();
					tangible->setAppearanceData (appearanceString);

					const int32 arrangement = static_cast<int32>(iff.read_uint32 ());

					if (!ContainerInterface::transferItemToSlottedContainer (*creature, *tangible, arrangement))
					{
						WARNING (true, ("Unable to transfer inventory object '%s' to player", templateName.c_str ()));
						delete tangible;
						tangible = 0;
					}
				}
			}

			iff.exitChunk (Tags::ITEM, true);
		}
		iff.exitForm (Tags::WEAR);
	}

	iff.exitForm (TAG_0000, true);

	return true;
}

bool CuiCachedAvatarManager::load_0001 (Iff & iff, CreatureObject * creature, CachedAvatarInfo & info)
{
	UNREF (creature);
	iff.enterForm(TAG_0001);

	iff.enterChunk (TAG_DATA);
	{
		if (creature)
		{
			const std::string templateName = iff.read_stdstring ();
			std::string fixedTemplateName (creature->getObjectTemplateName ());
			fixFilename (fixedTemplateName);

			if (templateName != fixedTemplateName)
			{
				WARNING (true, ("Mismatched object template names."));
				return false;
			}
			const std::string appearanceString = iff.read_stdstring ();
			creature->setAppearanceData (appearanceString);
		}
	}
	iff.exitChunk (TAG_DATA, true);

	//-- load the planet name
	if (iff.enterChunk (Tags::PLAN, true))
	{
		info.m_planetName = iff.read_stdstring ();
		iff.exitChunk (Tags::PLAN);
	}
	else
	{
		info.m_planetName.clear ();
	}

	if (iff.enterChunk (Tags::SkillTemplate, true))
	{
		info.m_skillTemplate = iff.read_stdstring ();
		iff.exitChunk (Tags::SkillTemplate);
	}
	else
	{
		info.m_skillTemplate.clear();
	}

	if (iff.enterChunk (Tags::SkillLevel, true))
	{
		info.m_level = iff.read_int32();
		iff.exitChunk(Tags::SkillLevel);
	}
	else
	{
		info.m_level = 0;
	}

	if (creature && iff.enterForm (Tags::WEAR, true))
	{
		while (iff.enterChunk (Tags::ITEM, true))
		{
			const std::string templateName     = iff.read_stdstring ();
			const std::string appearanceString = iff.read_stdstring ();

			Object * const object = ObjectTemplate::createObject (templateName.c_str ());

			if (!object)
			{
				WARNING (true, ("Unable to create cached inventory object '%s'", templateName.c_str ()));
			}
			else
			{

				TangibleObject * tangible = dynamic_cast<TangibleObject *>(object);
				if (!tangible)
				{
					WARNING (true, ("Cached inventory object '%s' is not tangible", templateName.c_str ()));
				}
				else
				{
					tangible->endBaselines ();
					tangible->setAppearanceData (appearanceString);

					const int32 arrangement = static_cast<int32>(iff.read_uint32 ());

					if (!ContainerInterface::transferItemToSlottedContainer (*creature, *tangible, arrangement))
					{
						WARNING (true, ("Unable to transfer inventory object '%s' to player", templateName.c_str ()));
						delete tangible;
						tangible = 0;
					}
				}
			}

			iff.exitChunk (Tags::ITEM, true);
		}
		iff.exitForm (Tags::WEAR);
	}

	iff.exitForm (TAG_0001, true);

	return true;
}

//----------------------------------------------------------------------

bool CuiCachedAvatarManager::save_0000 (Iff & iff, const CreatureObject & creature, const std::string & planetName)
{
	UNREF (creature);

	iff.insertForm (Tags::CADF);
	iff.insertForm (TAG_0000);

	iff.insertChunk (TAG_DATA);
	{
		std::string fixedTemplateName (creature.getObjectTemplateName ());
		fixFilename (fixedTemplateName);
		iff.insertChunkString (fixedTemplateName.c_str ());

		std::string appearanceData;

		//-- TangibleObject::getAppearanceData () would be nicer

		const CustomizationData * const cdata = creature.fetchCustomizationData ();
		if (cdata)
		{
			appearanceData = cdata->writeLocalDataToString ();
			cdata->release ();
		}

		iff.insertChunkString (appearanceData.c_str ());
	}
	iff.exitChunk (TAG_DATA);

	//-- insert planet name
	iff.insertChunk (Tags::PLAN);
	{
		iff.insertChunkString (planetName.c_str ());
	}
	iff.exitChunk (Tags::PLAN);

	//-- insert wearables
	iff.insertForm (Tags::WEAR);
	{
		SlottedContainer * slotted = const_cast<SlottedContainer*>(ContainerInterface::getSlottedContainer (creature));
		if (slotted)
		{
			ContainerIterator containerIterator = slotted->begin();
			for (; containerIterator != slotted->end(); ++containerIterator)
			{
				const CachedNetworkId id = *containerIterator;

				const TangibleObject * tangible = dynamic_cast<const TangibleObject *>(id.getObject ());

				if (!tangible)
					continue;

				if(!tangible->getShouldBakeIntoMesh())
					continue;

	
				const SlottedContainmentProperty * const scp = ContainerInterface::getSlottedContainmentProperty (*tangible);
				NOT_NULL (scp);
				
				const int arrangement = scp->getCurrentArrangement ();
				
				DEBUG_FATAL (arrangement < 0, ("bad arrangement"));
				
				iff.insertChunk (Tags::ITEM);
				{
					std::string fixedTemplateName (tangible->getObjectTemplateName ());
					fixFilename (fixedTemplateName);
					iff.insertChunkString (fixedTemplateName.c_str ());

					//-- TangibleObject::getAppearanceData () would be nicer

					std::string item_appearanceData;
					const CustomizationData * const item_cdata = tangible->fetchCustomizationData ();
					if (item_cdata )
					{
						item_appearanceData = item_cdata->writeLocalDataToString ();
						item_cdata->release ();
					}					
					
					iff.insertChunkString (item_appearanceData.c_str ());
					iff.insertChunkData   (static_cast<uint32>(arrangement));
				}
				iff.exitChunk (Tags::ITEM);
			}
		}
	}
	iff.exitForm (Tags::WEAR);
	
	iff.exitForm (TAG_0000);
	iff.exitForm (Tags::CADF);

	return true;
}

bool CuiCachedAvatarManager::save_0001 (Iff & iff, const CreatureObject & creature, const CachedAvatarInfo & info)
{
	UNREF (creature);

	iff.insertForm (Tags::CADF);
	iff.insertForm (TAG_0001);

	iff.insertChunk (TAG_DATA);
	{
		std::string fixedTemplateName (creature.getObjectTemplateName ());
		fixFilename (fixedTemplateName);
		iff.insertChunkString (fixedTemplateName.c_str ());

		std::string appearanceData;

		//-- TangibleObject::getAppearanceData () would be nicer

		const CustomizationData * const cdata = creature.fetchCustomizationData ();
		if (cdata)
		{
			appearanceData = cdata->writeLocalDataToString ();
			cdata->release ();
		}

		iff.insertChunkString (appearanceData.c_str ());
	}
	iff.exitChunk (TAG_DATA);

	//-- insert planet name
	iff.insertChunk (Tags::PLAN);
	{
		iff.insertChunkString (info.m_planetName.c_str ());
	}
	iff.exitChunk (Tags::PLAN);

	iff.insertChunk (Tags::SkillTemplate);
	{
		iff.insertChunkString (info.m_skillTemplate.c_str ());
	}
	iff.exitChunk (Tags::SkillTemplate);


	iff.insertChunk (Tags::SkillLevel);
	{
		iff.insertChunkData(info.m_level);
	}
	iff.exitChunk (Tags::SkillLevel);

	//-- insert wearables
	iff.insertForm (Tags::WEAR);
	{
		const SkeletalAppearance2* skeleAppearance = dynamic_cast<const SkeletalAppearance2*>(creature.getAppearance());
		if(skeleAppearance && skeleAppearance->getWearableCount())
		{
			// We can now simply walk over our wearables to write them out for these temporary characters.
			for(int i = 0; i < skeleAppearance->getWearableCount(); ++i)
			{

				const TangibleObject * tangible = dynamic_cast<const TangibleObject *>(skeleAppearance->getWearableObject(i));
				if (!tangible)
					continue;

				if(!tangible->getShouldBakeIntoMesh())
					continue;

				const SlottedContainmentProperty * const scp = ContainerInterface::getSlottedContainmentProperty (*tangible);
				NOT_NULL (scp);

				const int arrangement = scp->getCurrentArrangement ();

				if(arrangement < 0)
					continue;

				iff.insertChunk (Tags::ITEM);
				{
					std::string fixedTemplateName (tangible->getObjectTemplateName ());
					fixFilename (fixedTemplateName);
					iff.insertChunkString (fixedTemplateName.c_str ());

					//-- TangibleObject::getAppearanceData () would be nicer

					std::string item_appearanceData;
					const CustomizationData * const item_cdata = tangible->fetchCustomizationData ();
					if (item_cdata )
					{
						item_appearanceData = item_cdata->writeLocalDataToString ();
						item_cdata->release ();
					}					

					iff.insertChunkString (item_appearanceData.c_str ());
					iff.insertChunkData   (static_cast<uint32>(arrangement));
				}
				iff.exitChunk (Tags::ITEM);
			}

			WeaponObject const * weapon = creature.getCurrentWeapon();
			if(weapon)
			{
				const TangibleObject * tangible = weapon->asTangibleObject();
				if (tangible)
				{	
					const SlottedContainmentProperty * const scp = ContainerInterface::getSlottedContainmentProperty (*tangible);
					NOT_NULL (scp);

					const int arrangement = scp->getCurrentArrangement ();

					if(arrangement >= 0)
					{
						iff.insertChunk (Tags::ITEM);
						{
							std::string fixedTemplateName (tangible->getObjectTemplateName ());
							fixFilename (fixedTemplateName);
							iff.insertChunkString (fixedTemplateName.c_str ());

							//-- TangibleObject::getAppearanceData () would be nicer

							std::string item_appearanceData;
							const CustomizationData * const item_cdata = tangible->fetchCustomizationData ();
							if (item_cdata )
							{
								item_appearanceData = item_cdata->writeLocalDataToString ();
								item_cdata->release ();
							}					

							iff.insertChunkString (item_appearanceData.c_str ());
							iff.insertChunkData   (static_cast<uint32>(arrangement));
						}
						iff.exitChunk (Tags::ITEM);
					}
				}
			}
		}
		else
		{
			SlottedContainer * slotted = const_cast<SlottedContainer*>(ContainerInterface::getSlottedContainer (creature));
			if (slotted)
			{
				ContainerIterator containerIterator = slotted->begin();
				for (; containerIterator != slotted->end(); ++containerIterator)
				{
					const CachedNetworkId id = *containerIterator;

					const TangibleObject * tangible = dynamic_cast<const TangibleObject *>(id.getObject ());
					if (!tangible)
						continue;

					if(!tangible->getShouldBakeIntoMesh())
						continue;

					const SlottedContainmentProperty * const scp = ContainerInterface::getSlottedContainmentProperty (*tangible);
					NOT_NULL (scp);

					const int arrangement = scp->getCurrentArrangement ();

					DEBUG_FATAL (arrangement < 0, ("bad arrangement"));

					iff.insertChunk (Tags::ITEM);
					{
						std::string fixedTemplateName (tangible->getObjectTemplateName ());
						fixFilename (fixedTemplateName);
						iff.insertChunkString (fixedTemplateName.c_str ());

						//-- TangibleObject::getAppearanceData () would be nicer

						std::string item_appearanceData;
						const CustomizationData * const item_cdata = tangible->fetchCustomizationData ();
						if (item_cdata )
						{
							item_appearanceData = item_cdata->writeLocalDataToString ();
							item_cdata->release ();
						}					

						iff.insertChunkString (item_appearanceData.c_str ());
						iff.insertChunkData   (static_cast<uint32>(arrangement));
					}
					iff.exitChunk (Tags::ITEM);
				}
			}
		}

	}
	iff.exitForm (Tags::WEAR);

	iff.exitForm (TAG_0001);
	iff.exitForm (Tags::CADF);

	return true;
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::removeAvatar                (const NetworkId & id, uint32 clusterId)
{
	const CharacterClusterId cci (clusterId, id);
	s_characterListMap.erase (cci);
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::addToCharacterList (const uint32 clusterId, const NetworkId & id, const Unicode::String & name, const std::string & planetName)
{
	CharacterListInfo cli;
	cli.name       = name;
	cli.planet     = planetName;
	cli.playedLast = false;
	const CuiLoginManager::ClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo (clusterId);

	if (clusterInfo)
		cli.clusterName = clusterInfo->name;

	const CharacterClusterId cci (clusterId, id);
	s_characterListMap [cci] = cli;
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::setCharacterListPlanet (const uint32 clusterId, const NetworkId & id, const std::string & planet)
{
	if (Game::getSinglePlayer ())
		return;

	const CharacterClusterId cci (clusterId, id);
	const CharacterListMap::iterator it = s_characterListMap.find (cci);

	if (it == s_characterListMap.end ())
		WARNING (true, ("CuiCachedAvatarManager setCharacterPlanet [%d] [%s] could not find character in list", clusterId, id.getValueString ().c_str ()));
	else
		(*it).second.planet = planet;
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::setCharacterListLastPlayed  (const uint32 clusterId, const NetworkId & id)
{
	if (Game::getSinglePlayer ())
		return;

	bool found = false;

	for (CharacterListMap::iterator it = s_characterListMap.begin (); it != s_characterListMap.end (); ++it)
	{
		const CharacterClusterId & cci = (*it).first;
		CharacterListInfo & cli        = (*it).second;

		if (cci.first == clusterId && cci.second == id)
		{
			cli.playedLast = true;
			found = true;
		}
		else
		{
			cli.playedLast = false;
		}
	}

	if (!found)
		WARNING (true, ("CuiCachedAvatarManager setCharacterLastPlayed [%d] [%s] could not find character in list", clusterId, id.getValueString ().c_str ()));
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::saveCharacterList  ()
{
	const std::string & userName = GameNetwork::getUserName ();

	const std::string filename = std::string ("characterlist_") + userName + ".txt";

	std::string str;
	static char buf [1024];
	static const size_t buf_size = sizeof (buf);

	const std::string & launcherAvatarName = ConfigClientGame::getLauncherAvatarName ();

	for (CharacterListMap::iterator it = s_characterListMap.begin (); it != s_characterListMap.end (); ++it)
	{
		const CharacterClusterId & cci = (*it).first;
		CharacterListInfo & cli        = (*it).second;

		const std::string & avatarName = Unicode::wideToNarrow (cli.name);

		const bool wasSelectedFromLauncher = !launcherAvatarName.empty () && !_strnicmp (launcherAvatarName.c_str (), avatarName.c_str (), avatarName.size ());
		
		snprintf (buf, buf_size, "%s,%s (%s),%d,%d\n", cli.clusterName.c_str (), avatarName.c_str (), cli.clusterName.c_str (), wasSelectedFromLauncher ? 1 : 0, cci.first);

		str += buf;
	}

	AbstractFile * const af = StdioFileFactory ().createFile (filename.c_str (), "wb");
	if (!af)
		WARNING (true, ("CuiCachedAvatarManager::saveCharacterList () failed: StdioFileFactory ().createFile (%s) failed", filename.c_str ()));
	else
	{
		if (!af->isOpen ())
		{
			WARNING (true, ("CuiCachedAvatarManager::saveCharacterList () failed to open [%s]", filename.c_str ()));
		}
		else
		{
			af->write (str.size (), str.c_str ());
		}

		delete af;
	}
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::update          (float )
{

	if (s_saveScenePlayerNextUpdate)
	{
		CuiCachedAvatarManager::saveScenePlayer ();
		s_saveScenePlayerNextUpdate = false;
	}
}

//----------------------------------------------------------------------

void CuiCachedAvatarManager::saveScenePlayerNextUpdate  ()
{
	s_saveScenePlayerNextUpdate = true;
}

//======================================================================
