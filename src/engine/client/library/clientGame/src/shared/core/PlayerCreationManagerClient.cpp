//======================================================================
//
// PlayerCreationManagerClient.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerCreationManagerClient.h"

#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/Container.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"

#include <algorithm>

//======================================================================

void PlayerCreationManagerClient::install ()
{
	InstallTimer const installTimer("PlayerCreationManagerClient::install");

	if (!PlayerCreationManager::isInstalled ())
		PlayerCreationManager::install (true);
}

//----------------------------------------------------------------------

void PlayerCreationManagerClient::remove ()
{
	if (PlayerCreationManager::isInstalled ())
		PlayerCreationManager::remove ();
}

//----------------------------------------------------------------------

bool PlayerCreationManagerClient::setupPlayer(CreatureObject & obj, const std::string & profession)
{
	AttribVector attribs;
	const SkillVector * skills   = 0;
	const EqVector * eq          = 0;

	const char * const sharedObjectTemplateNameStr = obj.getObjectTemplateName ();

	if (!sharedObjectTemplateNameStr)
		return false;

	const std::string sharedObjectTemplateName (sharedObjectTemplateNameStr);

	if (!getDefaults(sharedObjectTemplateName, profession, attribs, skills, eq))
	{
		WARNING (true, ("PlayerCreationManagerClient error loading for %s, %s", sharedObjectTemplateName.c_str (), profession.c_str ()));
		return false;
	}

	//----------------------------------------------------------------------
	//-- setup skills

	//-- clear out old skills

	const CreatureObject::SkillList currentSkillsCopy = obj.getSkills ();

	{
		for (CreatureObject::SkillList::const_iterator it = currentSkillsCopy.begin (); it != currentSkillsCopy.end (); ++it)
		{
			const SkillObject * const skill = *it;
			if (skill)
				obj.clientRevokeSkill (*skill);
		}
	}

	if (skills)
	{
		for (SkillVector::const_iterator it = skills->begin (); it != skills->end (); ++it)
		{
			const std::string & skillName = *it;

			const SkillObject * skill = SkillManager::getInstance ().getSkill (*it);
			if (skill)
				obj.clientGrantSkill   (*skill);
			else
				WARNING (true, ("PlayerCreationManagerClient Bad skill [%s] for profesion [%s]", skillName.c_str (), profession.c_str ()));
		}
	}

	//----------------------------------------------------------------------
	//-- setup attribs

	if (!attribs.empty())
	{
		int i = 0;
		for (AttribVector::const_iterator it = attribs.begin (); it != attribs.end (); ++it)
		{
			obj.clientResetAttribute (i++, static_cast<short>(*it), static_cast<short>(*it));
		}
	}

	//----------------------------------------------------------------------
	//-- setup equipment

	strip (obj);

	if (ConfigClientGame::getCharacterCreationLoadoutsEnabled ())
	{
		if (eq)
		{
			for (EqVector::const_iterator it = eq->begin (); it != eq->end (); ++it)
			{
				const EqInfo & eqi = *it;

				int arrangement = eqi.arrangement;

				ClientObject * const item = safe_cast<ClientObject*>(ObjectTemplate::createObject (eqi.sharedTemplateName.c_str ()));

				if (!item)
					WARNING (true, ("Invalid equipment template: '%s'", eqi.sharedTemplateName.c_str ()));
				else
				{
					//-- the loadout is always client-only
					item->endBaselines ();
					SlottedContainer * const slotted = ContainerInterface::getSlottedContainer(obj);
					if (slotted)
					{
						//Bypass the loadout arrangement and use the first available.
						Container::ContainerErrorCode tmp = Container::CEC_Success;
						if (!slotted->getFirstUnoccupiedArrangement(*item, arrangement, tmp))
						{
							WARNING (true, ("Unable to equip item: [%s] for profession [%s]: no unoccupied arrangement", item->getObjectTemplateName (), profession.c_str ()));
							delete item;
							continue;
						}

						if (!ContainerInterface::transferItemToSlottedContainer (obj, *item, arrangement))
						{
							WARNING (true, ("Unable to equip item: [%s] for profession [%s]: transfer failed", item->getObjectTemplateName (), profession.c_str ()));
							delete item;
							continue;
						}
					}
					else
						WARNING (true, ("PlayerCreationManagerClient player profession [%s] had no slotted container", profession.c_str ()));
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

void PlayerCreationManagerClient::strip (CreatureObject & creature)
{
	Container * const container = ContainerInterface::getContainer (creature);
	if (container)
	{
		typedef std::vector<Object *> ObjectVector;
		static ObjectVector ov;
		ov.clear ();
		ov.reserve (container->getNumberOfItems ());

		for (ContainerIterator it = container->begin (); it != container->end (); ++it)
		{
			CachedNetworkId & id = *it;
			ClientObject * const obj = safe_cast<ClientObject *>(id.getObject ());
			if (obj)
			{
				SlottedContainmentProperty * const prop = ContainerInterface::getSlottedContainmentProperty (*obj);
				if (prop)
				{
					bool canManipulate = true;
					const int numSlots = prop->getNumberOfSlots (prop->getCurrentArrangement ());
					for (int i = 0; i < numSlots; ++i)
					{
						if (!ContainerInterface::canPlayerManipulateSlot (prop->getSlotId (prop->getCurrentArrangement (), i)))
						{
							canManipulate = false;
							break;
						}
					}

					if (!canManipulate)
						continue;

					Container::ContainerErrorCode tmp = Container::CEC_Success;
					if (!container->remove (*obj, tmp))
					{
						WARNING (true, ("Unable to remove object"));
					}

					ov.push_back (obj);

					//-- it probably shouldn't be necessary to do this.
					//-- This is required to force TangibleObject's m_containerChangeMap to be correct for
					//-- the next alter of creature

					if (obj->alter    (1.0f))
						obj->conclude ();

				}
			}
		}

		//-- The TangibleObject m_containerChangeMap refers to the removal of objects
		//-- before deleting the objects, we must force the TangibleObject creature to
		//-- process the changes

		if (creature.alter    (1.0f))
			creature.conclude ();

		//-- it should now be safe to delete the previously worn objects
		std::for_each (ov.begin (), ov.end (), PointerDeleter ());
		ov.clear ();
	}
}

//----------------------------------------------------------------------

std::string PlayerCreationManagerClient::findDefaultProfession (const std::string & categoryName)
{
	static StringVector sv;
	PlayerCreationManager::getProfessionVector (sv, categoryName);

	if (sv.empty ())
		return std::string ();
	else
		return sv.front ();
}

//======================================================================
