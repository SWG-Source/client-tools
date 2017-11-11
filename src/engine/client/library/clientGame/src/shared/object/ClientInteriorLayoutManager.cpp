// ======================================================================
//
// ClientInteriorLayoutManager.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientInteriorLayoutManager.h"

#include "clientGame/TangibleObject.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/ObjectWatcherList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedUtility/InteriorLayoutReaderWriter.h"

// ======================================================================

namespace ClientInteriorLayoutManagerNamespace
{
	bool ms_disableLazyInteriorLayoutCreation;
	bool ms_logApplyInteriorLayoutCreates;

	void remove();
}

using namespace ClientInteriorLayoutManagerNamespace;

// ======================================================================

void ClientInteriorLayoutManager::install(bool const disableLazyInteriorLayoutCreation)
{
	InstallTimer const installTimer("ClientInteriorLayoutManager::install");

	ms_disableLazyInteriorLayoutCreation = ConfigFile::getKeyBool("ClientGame/ClientInteriorLayoutManager", "disableLazyInteriorLayoutCreation", disableLazyInteriorLayoutCreation);

	DebugFlags::registerFlag(ms_logApplyInteriorLayoutCreates, "ClientGame/ClientInteriorLayoutManager", "logApplyInteriorLayoutCreates");

	ExitChain::add(remove, "ClientInteriorLayoutManagerNamespace::remove");
}

// ----------------------------------------------------------------------

void ClientInteriorLayoutManagerNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_logApplyInteriorLayoutCreates);
}

// ----------------------------------------------------------------------

void ClientInteriorLayoutManager::update()
{
	if (ms_disableLazyInteriorLayoutCreation)
		return;

	//-- find all visible cells
	RenderWorld::CellPropertyList const & cellPropertyList = RenderWorld::getVisibleCells();
	for (size_t i = 0; i < cellPropertyList.size(); ++i)
	{
		CellProperty const * const cellProperty = cellPropertyList[i];
		if (!cellProperty || cellProperty == CellProperty::getWorldCellProperty())
			continue;

		//-- Verify that the cell is in the world
		if (!cellProperty->getOwner().isInWorld())
			continue;

		//-- If we have already applied the interior layout, skip this cell
		if (cellProperty->getAppliedInteriorLayout())
			continue;

		//-- Mark that we have applied the layout
		cellProperty->setAppliedInteriorLayout();

		//-- Get the portal property's owner
		PortalProperty const * const portalProperty = cellProperty->getPortalProperty();
		if (!portalProperty)
			continue;

		Object & owner = const_cast<Object &>(portalProperty->getOwner());

		//-- Only tangible objects can have interior layouts
		TangibleObject * const tangibleObject = owner.asClientObject() ? owner.asClientObject()->asTangibleObject() : 0;
		if (!tangibleObject)
			continue;

		InteriorLayoutReaderWriter const * const interiorLayout = tangibleObject->getInteriorLayout();
		if (!interiorLayout)
			continue;

		TemporaryCrcString const cellName(cellProperty->getCellName(), true);
		for (int i = 0; i < interiorLayout->getNumberOfObjects(cellName); ++i)
		{
			CrcString const & objectTemplateName = interiorLayout->getObjectTemplateName(cellName, i);
			Transform const & transform_o2p = interiorLayout->getTransform_o2p(cellName, i);

			//-- Create the object
			ClientObject * const interiorObject = safe_cast<ClientObject *>(ObjectTemplateList::createObject(objectTemplateName));
			if (interiorObject)
			{
				DEBUG_REPORT_LOG(ms_logApplyInteriorLayoutCreates, ("ilf created [%s]\n", objectTemplateName.getString()));

				tangibleObject->addClientOnlyInteriorLayoutObject(interiorObject);

				interiorObject->setParentCell(const_cast<CellProperty *>(cellProperty));
				CellProperty::setPortalTransitionsEnabled(false);
					interiorObject->setTransform_o2p(transform_o2p);
				CellProperty::setPortalTransitionsEnabled(true);

				RenderWorld::addObjectNotifications(*interiorObject);

				interiorObject->endBaselines();
				interiorObject->addToWorld();
			}
			else
				DEBUG_WARNING(true, ("Object template %s specified building layout %s which specified invalid interior object template name %s.  Object will be skipped.", tangibleObject->getObjectTemplateName(), interiorLayout->getFileName().getString(), objectTemplateName.getString()));
		}
	}
}

// ----------------------------------------------------------------------

void ClientInteriorLayoutManager::applyInteriorLayout(TangibleObject * const tangibleObject, InteriorLayoutReaderWriter const * const interiorLayout, char const * const fileName)
{
	UNREF(fileName);

	if (!ms_disableLazyInteriorLayoutCreation)
		return;

	if (!tangibleObject)
		return;

	if (!interiorLayout)
		return;

	//-- Are there any objects in the interior layout file?
	int const totalNumberOfObjects = interiorLayout->getNumberOfObjects();
	if (totalNumberOfObjects == 0)
		return;

	//-- Does the tangible object have a priority?
	PortalProperty * const portalProperty = tangibleObject->getPortalProperty();
	if (!portalProperty)
		return;

	//-- Add the objects in the interior layout file to the object
	for (int i = 0; i < interiorLayout->getNumberOfCellNames(); ++i)
	{
		CrcString const & cellName = interiorLayout->getCellName(i);

		CellProperty * const cellProperty = portalProperty->getCell(cellName.getString());
		if (cellProperty)
		{
			int const numberOfObjects = interiorLayout->getNumberOfObjects(cellName);
			for (int j = 0; j < numberOfObjects; ++j)
			{
				CrcString const & objectTemplateName = interiorLayout->getObjectTemplateName(cellName, j);
				Transform const & transform_o2p = interiorLayout->getTransform_o2p(cellName, j);

				//-- Create the object
				ClientObject * const object = safe_cast<ClientObject *>(ObjectTemplateList::createObject(objectTemplateName));
				if (object)
				{
					DEBUG_REPORT_LOG(ms_logApplyInteriorLayoutCreates, ("ilf created [%s]\n", objectTemplateName.getString()));

					tangibleObject->addClientOnlyInteriorLayoutObject(object);

					object->setParentCell(cellProperty);
					CellProperty::setPortalTransitionsEnabled(false);
						object->setTransform_o2p(transform_o2p);
					CellProperty::setPortalTransitionsEnabled(true);

					RenderWorld::addObjectNotifications(*object);

					object->endBaselines();
					object->addToWorld();
				}
				else
					DEBUG_WARNING(true, ("Object template %s specified building layout %s which specified invalid interior object template name %s.  Object will be skipped.", tangibleObject->getObjectTemplateName(), fileName, objectTemplateName.getString()));
			}
		}
		else
			DEBUG_WARNING(true, ("Object template %s specified building layout %s which specified invalid cell name %s.  Object will be skipped.", tangibleObject->getObjectTemplateName(), fileName, cellName.getString()));
	}
}

// ======================================================================
