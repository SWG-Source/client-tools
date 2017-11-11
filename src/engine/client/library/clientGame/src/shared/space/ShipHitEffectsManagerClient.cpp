//======================================================================
//
// ShipHitEffectsManagerClient.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipHitEffectsManagerClient.h"

#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ShipObject.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipHitEffectsManager.h"
#include "sharedMath/AxialBox.h"
#include "sharedNetworkMessages/MessageQueueShipHit.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

//======================================================================

namespace ShipHitEffectsManagerClientNamespace
{
	/**
	* hitLight/hitMedium/hitHeavy are hits that occur in the damage ranges (.66,1.0], (.33,.66], (0.0,.33) respectively
	* hitEventLight/hitEventMedium/hitEventHeavy are hits that bring the damage level down across one of the above partitions
	*/

	enum HitEvents
	{
		HE_hitLight,
		HE_hitMedium,
		HE_hitHeavy,
		HE_hitEventLight,
		HE_hitEventMedium,
		HE_hitEventHeavy,
		HE_numTypes
	};

	//----------------------------------------------------------------------

	ClientEffectTemplate const * s_effects[ShipHitEffectsManager::HT_numTypes][HE_numTypes];

	struct SoundGroup
	{
	public:

		SoundTemplate const * sounds[ShipHitEffectsManager::HT_numTypes];
		SoundGroup()
		{
			sounds[0] = sounds[1] = sounds[2] = sounds[3] = NULL;
		}

		void release()
		{
			if (NULL != sounds[0])
			{
				SoundTemplateList::release(sounds[0]);
				sounds[0] = NULL;
			}
			if (NULL != sounds[1])
			{
				SoundTemplateList::release(sounds[1]);
				sounds[1] = NULL;
			}
			if (NULL != sounds[2])
			{
				SoundTemplateList::release(sounds[2]);
				sounds[2] = NULL;
			}
			if (NULL != sounds[3])
			{
				SoundTemplateList::release(sounds[3]);
				sounds[3] = NULL;
			}
		}
	};

	typedef stdmap<std::string, SoundGroup>::fwd SoundChassisMap;
	SoundChassisMap s_sounds;
	
	//----------------------------------------------------------------------

	bool s_installed = false;

	void loadClientEffects()
	{
		{
			for (int i = 0; i < ShipHitEffectsManager::HT_numTypes; ++i)
			{
				for (int j = 0; j < HE_numTypes; ++j)
				{
					s_effects[i][j] = NULL;
				}
			}
		}
		
		std::string const & filename = "datatables/space/ship_hit_effects.iff";
		
		DataTable * const dt = DataTableManager::getTable(filename, true);
		
		if (dt == NULL)
		{
			WARNING(true, ("ShipHitEffectsManagerClient no such datatable [%s]", filename.c_str()));
			return;
		}
		
		int numRows = dt->getNumRows();
		
		if (ShipHitEffectsManager::HT_numTypes != numRows)
		{
			WARNING(true, ("ShipHitEffectsManagerClient invalid row count: [%d]", numRows));
		}
		else
		{
			
			numRows = std::min(numRows, static_cast<int>(ShipHitEffectsManager::HT_numTypes));
			
			for (int row = 0; row < numRows; ++row)
			{
				for (int type = 0; type < HE_numTypes; ++type)
				{
					int const tableCol = type + 1;
					std::string const & cefName = dt->getStringValue(tableCol, row);
					
					if (cefName.empty())
					{
						WARNING(true, ("ShipHitEffectsManagerClient empty CEF name for row/col [%d,%d]", row, tableCol));
						continue;
					}
					
					s_effects[row][type] = ClientEffectTemplateList::fetch(CrcLowerString(cefName.c_str()));
					if (NULL == s_effects[row][type])
					{
						WARNING(true, ("ShipHitEffectsManagerClient failed to load CEF [%s] for row/col [%d,%d]", cefName.c_str(), row, tableCol));
						continue;
					}
				}
			}
		}

		DataTableManager::close(filename);
	}

	//----------------------------------------------------------------------

	void loadSounds()
	{
		std::string const & filename = "datatables/space/ship_hit_sounds.iff";
		
		DataTable * const dt = DataTableManager::getTable(filename, true);
		
		if (dt == NULL)
		{
			WARNING(true, ("ShipHitEffectsManagerClient no such datatable [%s]", filename.c_str()));
			return;
		}
		
		int numRows = dt->getNumRows();
		
		{
			for (int row = 0; row < numRows; ++row)
			{
				SoundGroup sg;

				std::string const & chassisSoundName = dt->getStringValue(0, row);

				for (int type = 0; type < ShipHitEffectsManager::HT_numTypes; ++type)
				{
					int const tableCol = type + 1;

					std::string const & sndName = dt->getStringValue(tableCol, row);

					if (sndName.empty())
					{
						WARNING(true, ("ShipHitEffectsManagerClient empty SND name for row/col [%d,%d]", row, tableCol));
						continue;
					}
					
					sg.sounds[type] = SoundTemplateList::fetch(sndName.c_str());

					if (NULL == sg.sounds[type])
					{
						WARNING(Audio::isEnabled(), ("ShipHitEffectsManagerClient failed to load SND [%s] for row/col [%d,%d]", sndName.c_str(), row, tableCol));
						continue;
					}
				}

				s_sounds[chassisSoundName] = sg;
			}
		}

		DataTableManager::close(filename);
	}

	//----------------------------------------------------------------------

	void load()
	{
		if (s_installed)
			return;

		loadClientEffects();
		loadSounds();
	}

	//----------------------------------------------------------------------

	int getIntegrityPartition(float f)
	{
		static float const integrityPartitions[3] = {0.66f, 0.33f, 0.0f};
		
		for (int i = 0; i < 3; ++i)
		{
			if (f > integrityPartitions[i])
				return i;
		}

		return -1;
	}

	//----------------------------------------------------------------------

	Vector findPlayPoint_o(Object const & obj, Vector const & up_o)
	{
		Vector shotDirection_o = -up_o;

		float const radius = obj.getTangibleExtent().getRadius();

		Vector start_o = shotDirection_o * (-2.0f * radius);
		Vector end_o = shotDirection_o * (2.0f * radius);
		
		bool found = false;
		if (NULL != obj.getAppearance())
		{
			CollisionInfo cinfo;
			if (obj.getAppearance()->collide(start_o, end_o, CollideParameters::cms_default, cinfo))
			{
				end_o = cinfo.getPoint();
				found = true;
			}
		}

		if (!found)
			return Vector::zero;

		return end_o;
	}

	//----------------------------------------------------------------------

	SoundTemplate const * findSoundTemplateForShip(ShipObject const & ship, ShipHitEffectsManager::HitType ht)
	{
		uint32 const chassisCrc = ship.getChassisType();
		ShipChassis const * const chassis = ShipChassis::findShipChassisByCrc(chassisCrc);

		std::string const & shipSoundType = chassis ? chassis->getHitSoundGroup() : ""; 

		SoundChassisMap::const_iterator it = s_sounds.find(shipSoundType);
		if (it == s_sounds.end() && !shipSoundType.empty())
			it = s_sounds.find("");

		if (it == s_sounds.end())
			return NULL;

		SoundGroup const & sg = (*it).second;
		return sg.sounds[ht];
	}

	//----------------------------------------------------------------------


}

using namespace ShipHitEffectsManagerClientNamespace;

//----------------------------------------------------------------------


void ShipHitEffectsManagerClient::install()
{
	InstallTimer const installTimer("ShipHitEffectsManagerClient::install");

	ExitChain::add(ShipHitEffectsManagerClient::remove, "ShipHitEffectsManagerClient::remove");
	load();
	s_installed = true;
}

//----------------------------------------------------------------------

void ShipHitEffectsManagerClient::remove()
{
	DEBUG_FATAL(!s_installed, ("ShipHitEffectsManagerClient not installed"));

	for (int i = 0; i < ShipHitEffectsManager::HT_numTypes; ++i)
	{
		for (int j = 0; j < HE_numTypes; ++j)
		{
			if (NULL != s_effects[i][j])
			{
				s_effects[i][j]->release();
				s_effects[i][j] = NULL;
			}
		}
	}

	{
		for (SoundChassisMap::iterator it = s_sounds.begin(); it != s_sounds.end(); ++it)
		{
			SoundGroup & sg = (*it).second;
			sg.release();
		}

		s_sounds.clear();
	}
	
	s_installed = false;
}

//----------------------------------------------------------------------

void ShipHitEffectsManagerClient::handleShipHit(ShipObject & ship, MessageQueueShipHit const & msg)
{
	DEBUG_FATAL(!s_installed, ("ShipHitEffectsManagerClient not installed"));

	if (msg.type < 0 || msg.type >= ShipHitEffectsManager::HT_numTypes)
	{
		WARNING(true, ("ShipHitEffectsManagerClient invalid hit type [%d] for ship [%s]", msg.type, ship.getNetworkId().getValueString().c_str()));
		return;
	}

	//-- bogus data from the server
	if (msg.integrity > msg.previousIntegrity)
		return;

	int const integrityPartition = getIntegrityPartition(msg.integrity);
	int const previousIntegrityPartition = getIntegrityPartition(msg.previousIntegrity);

	HitEvents hitEvent = HE_numTypes;

	//-- we crossed the integrity partition, play the event
	//-- this is very similar to CDF damage level handling
	if (integrityPartition == -1)
	{
		hitEvent = HE_hitEventHeavy;
	}
	else if (integrityPartition > previousIntegrityPartition)
	{
		hitEvent = static_cast<HitEvents>(static_cast<int>(HE_hitEventLight + (integrityPartition - 1)));
	}
	else
	{
		hitEvent = static_cast<HitEvents>(static_cast<int>(HE_hitLight + integrityPartition));
	}
	
	Vector up_o = msg.up_w;
	
	//-- sanity check our up vector
	if (up_o.magnitudeSquared() < 0.001f)
	{
		up_o = Vector::unitY;
	}
	else
	{	
		Vector up_o = ship.rotate_w2o(up_o);
		up_o.normalize();
	}
	
	Vector const & playPoint_o = msg.hitLocation_o;
	
	//-- first play the CEF

	bool cefOk = true;

	//-- check to see if shield event can be played yet on the ship
	if (ShipHitEffectsManager::HT_shield == msg.type)
	{
		bool const isEvent = hitEvent == HE_hitEventLight || hitEvent == HE_hitEventMedium || hitEvent == HE_hitEventHeavy;

		if (!isEvent)
		{
			if (!ship.isTimerShieldHitServerExpired())
			{
				cefOk = false;
			}
			else
			{
				ship.resetTimerShieldHitServer();
			}
		}
		else 
		{
			if (!ship.isTimerShieldEventServerExpired())
			{
				cefOk = false;
			}
			else
			{
				ship.resetTimerShieldEventServer();
				ship.resetTimerShieldHitServer();
			}
		}
	}

	if (cefOk)
	{		
		ClientEffectTemplate const * cefTemplate = s_effects[msg.type][hitEvent];	
		if (NULL != cefTemplate)
		{		
			Transform t;
			t.move_p(playPoint_o);
			t.setLocalFrameKJ_p(Vector::perpendicular(up_o), up_o);
			
			ClientEffect * const cef = cefTemplate->createClientEffect(&ship, t);
			
			if (ShipHitEffectsManager::HT_shield == msg.type)
			{
				float shieldScale = clamp(1.0f, ship.getTangibleExtent().getRadius(), 50.0f);
				cef->setUniformScale(shieldScale);
			}
			else
			{
				float effectScale = clamp(1.0f, ship.getTangibleExtent().getRadius() * 0.05f, 20.0f);
				cef->setUniformScale(effectScale);
			}
			
			cef->execute();
			delete cef;
		}
	}

	//-- now play the sound

	SoundTemplate const * const st = findSoundTemplateForShip(ship, static_cast<ShipHitEffectsManager::HitType>(msg.type));
	if (NULL != st)
	{
		int const got = ship.getGameObjectType();

		//@todo: change this when we can Audio::attachSound() with a local position

		if (SharedObjectTemplate::GOT_ship_capital == got ||
			SharedObjectTemplate::GOT_ship_station == got)
		{
			Audio::playSound(st->getCrcName().getString(), ship.rotateTranslate_o2w(playPoint_o), CellProperty::getWorldCellProperty());
		}
		else
		{
			Audio::attachSound(st->getCrcName().getString(), &ship, NULL);
		}
	}
}

//======================================================================
