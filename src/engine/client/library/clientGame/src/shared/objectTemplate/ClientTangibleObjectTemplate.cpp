// ======================================================================
//
// ClientTangibleObjectTemplate.cpp - A wrapper around SharedTangibleObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientTangibleObjectTemplate.h"

#include "clientGame/TangibleObject.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace ClientTangibleObjectTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum ColumnData
	{
		CD_componentCrc,
		CD_sharedObjectTemplateName,
		CD_hardPointName
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::pair<PersistentCrcString*, PersistentCrcString*>     Component;
	typedef std::map<int, Component>                                  ComponentMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const char* const ms_componentMapFileName = "datatables/crafting/component.iff";
	bool              ms_installed;
	ComponentMap*     ms_componentMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove ();
	void createComponentMap ();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ClientTangibleObjectTemplateNamespace;

// ======================================================================
// STATIC PUBLIC ClientTangibleObjectTemplate
// ======================================================================

void ClientTangibleObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientTangibleObjectTemplate::install");

	DEBUG_FATAL (ms_installed, ("ClientTangibleObjectTemplate::install: already installed"));

	SharedTangibleObjectTemplate::install(allowDefaultTemplateParams);

	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedTangibleObjectTemplate_tag, create);

	//-- load the component map
	createComponentMap ();

	//-- add to the exit chain
	ExitChain::add (ClientTangibleObjectTemplateNamespace::remove, "ClientTangibleObjectTemplateNamespace::remove");
}

// ----------------------------------------------------------------------

void ClientTangibleObjectTemplate::applyComponents (Object* const object, const std::set<int>& componentList)
{
	for (std::set<int>::iterator i = componentList.begin(); i != componentList.end() ; ++i)
	{
		const int crc = *i;

		ComponentMap::iterator iter = ms_componentMap->find (crc);
		if (iter != ms_componentMap->end ())
		{
			//-- get the data
			const PersistentCrcString* const sharedObjectTemplateFileName = iter->second.first;
			const PersistentCrcString* const hardPointName                = iter->second.second;

			//-- fetch the object template
			const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (sharedObjectTemplateFileName->getString ());
			if (!objectTemplate)
			{
				DEBUG_WARNING (true, ("ClientTangibleObjectTemplate::applyComponents: could not open object template [%s]", object->getNetworkId ().getValueString ().c_str (), sharedObjectTemplateFileName->getString ()));
				continue;
			}

			//-- create the child object
			ClientObject * const childObject = safe_cast<ClientObject *> (objectTemplate->createObject ());
			RenderWorld::addObjectNotifications (*childObject);
			objectTemplate->releaseReference ();
			
			//-- get the base appearance object the object
			Transform hardpointTransform;

			const Appearance* const appearance = object->getAppearance ();
			if (appearance)
			{
				if (!appearance->findHardpoint (*hardPointName, hardpointTransform))
					DEBUG_WARNING (true, ("ClientTangibleObjectTemplate::applyComponents - missing hardpoint [%s] on object [%s]", hardPointName->getString (), object->getAppearance ()->getAppearanceTemplate ()->getName ()));
			}
			else
				DEBUG_WARNING (true, ("ClientTangibleObjectTemplate::applyComponents: base object [%s] (%s) has no appearance", object->getNetworkId ().getValueString ().c_str (), object->getObjectTemplate ()->getName ()));

			childObject->setTransform_o2p (hardpointTransform);
			childObject->endBaselines ();
			object->addChildObject_o (childObject);
		}
		else
			DEBUG_WARNING (true, ("ClientTangibleObjectTemplate::applyComponents: object [%s] could not attach component with crc %i, it doesn't exist in the data table", object->getNetworkId ().getValueString ().c_str (), crc));
	}
}

// ======================================================================
// PUBLIC ClientTangibleObjectTemplate
// ======================================================================

ClientTangibleObjectTemplate::ClientTangibleObjectTemplate(const std::string & filename) :
	SharedTangibleObjectTemplate(filename)
{
}

// ----------------------------------------------------------------------

ClientTangibleObjectTemplate::~ClientTangibleObjectTemplate()
{
}

// ----------------------------------------------------------------------

Object * ClientTangibleObjectTemplate::createObject() const
{
	return new TangibleObject(this);
}

// ======================================================================
// STATIC PRIVATE ClientTangibleObjectTemplate
// ======================================================================

void ClientTangibleObjectTemplateNamespace::remove()
{
	//-- delete all the components of the component map
	ComponentMap::iterator iter = ms_componentMap->begin ();
	for (; iter != ms_componentMap->end (); ++iter)
	{
		delete iter->second.first;
		delete iter->second.second;
	}

	delete ms_componentMap;
	ms_componentMap = 0;
}

// ----------------------------------------------------------------------

ObjectTemplate * ClientTangibleObjectTemplate::create(const std::string & filename)
{
	return new ClientTangibleObjectTemplate(filename);
}

// ----------------------------------------------------------------------

void ClientTangibleObjectTemplateNamespace::createComponentMap ()
{
	ms_componentMap = new ComponentMap;

	Iff iff;
	if (iff.open (ms_componentMapFileName, true))
	{
		DataTable dataTable;
		dataTable.load (iff);

		const int numberOfRows = dataTable.getNumRows ();
		int row;
		for (row = 0; row < numberOfRows; ++row)
		{
			const int         crc                      = dataTable.getIntValue (CD_componentCrc, row);
			const char* const sharedObjectTemplateName = dataTable.getStringValue (CD_sharedObjectTemplateName, row);
			const char* const hardPointName            = dataTable.getStringValue (CD_hardPointName, row);

			PersistentCrcString* const sharedObjectTemplateNameFileName = new PersistentCrcString (sharedObjectTemplateName, true);
			PersistentCrcString* const hardPointFileName                = new PersistentCrcString (hardPointName, true);

			ComponentMap::iterator iter = ms_componentMap->find (crc);
			DEBUG_FATAL (iter != ms_componentMap->end (), ("component map [%s] already has an entry for crc %i (row %i)", ms_componentMapFileName, crc, row));
			(*ms_componentMap) [crc] = std::make_pair (sharedObjectTemplateNameFileName, hardPointFileName);
		}
	}
}

// ======================================================================

