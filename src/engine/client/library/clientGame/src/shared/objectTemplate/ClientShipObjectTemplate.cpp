// ======================================================================
//
// ClientShipObjectTemplate.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientShipObjectTemplate.h"

#include "clientGame/ClientInteriorLayoutManager.h"
#include "clientGame/ShipObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/InteriorLayoutReaderWriter.h"

// ======================================================================

class ClientShipObjectTemplate::PreloadManager
{
public:

	explicit PreloadManager(ClientShipObjectTemplate const * objectTemplate);
	~PreloadManager();

private:
	
	PreloadManager();
	PreloadManager(PreloadManager const &);
	PreloadManager & operator=(PreloadManager const &);

private:

	typedef std::vector<ObjectTemplate const *> PreloadObjectTemplateList;
	PreloadObjectTemplateList m_preloadObjectTemplateList;
};

// ----------------------------------------------------------------------

ClientShipObjectTemplate::PreloadManager::PreloadManager(ClientShipObjectTemplate const * const clientShipObjectTemplate) :
	m_preloadObjectTemplateList()
{
	if(clientShipObjectTemplate && clientShipObjectTemplate->m_interiorLayoutReader)
	{
		InteriorLayoutReaderWriter::CrcStringVector objectTemplateNames;
		clientShipObjectTemplate->m_interiorLayoutReader->getObjectTemplateNames(objectTemplateNames);

		for (size_t i = 0; i < objectTemplateNames.size(); ++i)
		{
			CrcString const & objectTemplateName = *objectTemplateNames[i];
			ObjectTemplate const * const objectTemplate = ObjectTemplateList::fetch(objectTemplateName);
			if (objectTemplate)
			{
				objectTemplate->preloadAssets ();
				m_preloadObjectTemplateList.push_back (objectTemplate);
			}
			else 
				DEBUG_WARNING(true, ("Interior layout file %s (for %s) specifies invalid object template %s", 
				clientShipObjectTemplate->getInteriorLayoutFileName().c_str(), 
				clientShipObjectTemplate->getName(),
				objectTemplateName.getString()));
		}
	}
	else
	{
		WARNING(true, ("ClientShipObjectTemplate::PreloadManager::PreloadManager: clientShipObjectTemplate or it's interior layout reader are invalid\n"));
	}
}

// ----------------------------------------------------------------------

ClientShipObjectTemplate::PreloadManager::~PreloadManager()
{
	for (size_t i = 0; i < m_preloadObjectTemplateList.size(); ++i)
		m_preloadObjectTemplateList[i]->releaseReference();

	m_preloadObjectTemplateList.clear();
}

// ======================================================================

void ClientShipObjectTemplate::install()
{
	InstallTimer const installTimer("ClientShipObjectTemplate::install");

	SharedShipObjectTemplate::install();
	ObjectTemplateList::assignBinding(SharedShipObjectTemplate_tag, create);
}

// ----------------------------------------------------------------------

ObjectTemplate *ClientShipObjectTemplate::create(std::string const & filename)
{
	return new ClientShipObjectTemplate(filename);
}

// ======================================================================

ClientShipObjectTemplate::ClientShipObjectTemplate(std::string const &filename) :
	SharedShipObjectTemplate(filename),
	m_interiorLayoutReader(0),
	m_preloadManager(0)
{
}

// ----------------------------------------------------------------------

ClientShipObjectTemplate::~ClientShipObjectTemplate()
{
	if (m_interiorLayoutReader)
		delete m_interiorLayoutReader;

	if (m_preloadManager)
		delete m_preloadManager;
}

// ----------------------------------------------------------------------

Object *ClientShipObjectTemplate::createObject() const
{
	return new ShipObject(this);
}

// ----------------------------------------------------------------------

void ClientShipObjectTemplate::preloadAssets() const
{
	SharedShipObjectTemplate::preloadAssets();

	if (!m_preloadManager)
		m_preloadManager = new PreloadManager(this);
}

// ----------------------------------------------------------------------

void ClientShipObjectTemplate::garbageCollect() const
{
	SharedShipObjectTemplate::garbageCollect();

	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

// ----------------------------------------------------------------------

InteriorLayoutReaderWriter const * ClientShipObjectTemplate::getInteriorLayout() const
{
	return m_interiorLayoutReader;
}

// ----------------------------------------------------------------------

void ClientShipObjectTemplate::applyInteriorLayout(ShipObject * const shipObject) const
{
	ClientInteriorLayoutManager::applyInteriorLayout(shipObject, m_interiorLayoutReader, getInteriorLayoutFileName().c_str());
}

// ----------------------------------------------------------------------

void ClientShipObjectTemplate::postLoad()
{
	SharedShipObjectTemplate::postLoad();

	//-- Load the interior layout file
	char const * const fileName = getInteriorLayoutFileName().c_str();
	if (fileName && *fileName)
	{
		m_interiorLayoutReader = new InteriorLayoutReaderWriter;
		if (!m_interiorLayoutReader->load(fileName))
			DEBUG_WARNING(true, ("object template %s specified invalid interior layout file %s", getName(), fileName));
	}
}	

// ======================================================================

