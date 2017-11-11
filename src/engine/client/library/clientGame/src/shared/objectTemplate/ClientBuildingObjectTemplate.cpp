// ======================================================================
//
// ClientBuildingObjectTemplate.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientBuildingObjectTemplate.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/ClientInteriorLayoutManager.h"
#include "clientGame/ClientLotManagerNotification.h"
#include "clientGame/ClientStructureFootprintObject.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplateList.h"
#include "sharedUtility/InteriorLayoutReaderWriter.h"

// ======================================================================
// ClientBuildingObjectTemplate::PreloadManager
// ======================================================================

class ClientBuildingObjectTemplate::PreloadManager
{
public:

	explicit PreloadManager (const ClientBuildingObjectTemplate* objectTemplate);
	~PreloadManager ();

private:
	
	PreloadManager ();
	PreloadManager (const PreloadManager&);
	PreloadManager& operator= (const PreloadManager&);

private:

	typedef std::vector<const ObjectTemplate*> PreloadObjectTemplateList;
	PreloadObjectTemplateList m_preloadObjectTemplateList;
};

// ----------------------------------------------------------------------

ClientBuildingObjectTemplate::PreloadManager::PreloadManager (const ClientBuildingObjectTemplate* const clientBuildingObjectTemplate) :
	m_preloadObjectTemplateList ()
{
	InteriorLayoutReaderWriter::CrcStringVector objectTemplateNames;
	clientBuildingObjectTemplate->m_interiorLayoutReader->getObjectTemplateNames(objectTemplateNames);

	for (size_t i = 0; i < objectTemplateNames.size(); ++i)
	{
		CrcString const & objectTemplateName = *objectTemplateNames[i];
		const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (objectTemplateName);
		if (objectTemplate)
		{
			objectTemplate->preloadAssets ();
			m_preloadObjectTemplateList.push_back (objectTemplate);
		}
		else 
			DEBUG_WARNING(true, ("Interior layout file %s (for %s) specifies invalid object template %s", 
			clientBuildingObjectTemplate->getInteriorLayoutFileName().c_str(), 
			clientBuildingObjectTemplate->getName(),
			objectTemplateName.getString()));
	}
}

// ----------------------------------------------------------------------

ClientBuildingObjectTemplate::PreloadManager::~PreloadManager ()
{
	uint i;
	for (i = 0; i < m_preloadObjectTemplateList.size (); ++i)
		m_preloadObjectTemplateList [i]->releaseReference ();

	m_preloadObjectTemplateList.clear ();
}

// ======================================================================
// STATIC PUBLIC ClientBuildingObjectTemplate
// ======================================================================

void ClientBuildingObjectTemplate::install (const bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientBuildingObjectTemplate::install ");

	SharedBuildingObjectTemplate::install (allowDefaultTemplateParams);

	//-- replace the shared create function with ours
	ObjectTemplateList::assignBinding (SharedBuildingObjectTemplate_tag, create);
}	

// ======================================================================
// PUBLIC ClientBuildingObjectTemplate
// ======================================================================

ClientBuildingObjectTemplate::ClientBuildingObjectTemplate (const std::string& filename) :
	SharedBuildingObjectTemplate (filename),
	m_interiorLayoutReader (new InteriorLayoutReaderWriter),
	m_preloadManager (0)
{
}

// ----------------------------------------------------------------------

ClientBuildingObjectTemplate::~ClientBuildingObjectTemplate ()
{
	delete m_interiorLayoutReader;

	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}	

// ----------------------------------------------------------------------

Object* ClientBuildingObjectTemplate::createObject () const
{
	Object* const object = new BuildingObject (this);

	if (getStructureFootprint ())
	{
		DEBUG_WARNING (getNoBuildRadius () > 0.f, ("ClientBuildingObjectTemplate [%s] non-zero (%1.2f) noBuildRadius found on object with structure footprint", getName (), getNoBuildRadius ()));

		object->addNotification (ClientLotManagerNotification::getInstance ());

		if (getUseStructureFootprintOutline ())
			object->addChildObject_o (new ClientStructureFootprintObject (*getStructureFootprint ()));
	}

	return object;
}	

// ----------------------------------------------------------------------

void ClientBuildingObjectTemplate::preloadAssets() const
{
	SharedBuildingObjectTemplate::preloadAssets();

	if (!m_preloadManager)
		m_preloadManager = new PreloadManager (this);
}

// ----------------------------------------------------------------------

void ClientBuildingObjectTemplate::garbageCollect() const
{
	SharedBuildingObjectTemplate::garbageCollect();

	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

// ----------------------------------------------------------------------

InteriorLayoutReaderWriter const * ClientBuildingObjectTemplate::getInteriorLayout() const
{
	return m_interiorLayoutReader;
}

// ----------------------------------------------------------------------

void ClientBuildingObjectTemplate::applyInteriorLayout (BuildingObject* const buildingObject) const
{
	ClientInteriorLayoutManager::applyInteriorLayout(buildingObject, m_interiorLayoutReader, getInteriorLayoutFileName().c_str());
}

// ======================================================================
// PROTECTED ClientBuildingObjectTemplate
// ======================================================================

void ClientBuildingObjectTemplate::postLoad ()
{
	//-- chain up to parent.
	SharedBuildingObjectTemplate::postLoad ();

	//-- load the layout
	const char* const fileName = getInteriorLayoutFileName ().c_str ();
	if (fileName && *fileName && !m_interiorLayoutReader->load (fileName))
		DEBUG_WARNING (true, ("object template %s specified invalid building layout %s", getName (), fileName));
}	

// ======================================================================
// STATIC PRIVATE ClientBuildingObjectTemplate
// ======================================================================

ObjectTemplate* ClientBuildingObjectTemplate::create (const std::string& filename)
{
	return new ClientBuildingObjectTemplate (filename);
}	

// ======================================================================

