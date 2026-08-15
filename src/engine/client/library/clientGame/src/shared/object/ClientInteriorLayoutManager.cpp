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

	bool ms_disableInteriorLayouts;

	// CONSULT-46 fix #2: max interior-layout objects created per frame. 0 = unlimited
	// (byte-identical original behavior). >0 spreads a cell's create burst across frames to
	// smooth the zone-in stutter. Resume cursor lives on CellProperty (no dangling pointers).
	int ms_maxInteriorCreatesPerFrame;

	void remove();
}

using namespace ClientInteriorLayoutManagerNamespace;

// ======================================================================

void ClientInteriorLayoutManager::install(bool const disableLazyInteriorLayoutCreation)
{
	InstallTimer const installTimer("ClientInteriorLayoutManager::install");

	ms_disableLazyInteriorLayoutCreation = ConfigFile::getKeyBool("ClientGame/ClientInteriorLayoutManager", "disableLazyInteriorLayoutCreation", disableLazyInteriorLayoutCreation);

	ms_disableInteriorLayouts = !ConfigFile::getKeyBool("ClientGame", "useInteriorLayoutFiles", true);

	// CONSULT-46 fix #2: per-frame interior-layout create budget (0 = unlimited = original).
	ms_maxInteriorCreatesPerFrame = ConfigFile::getKeyInt("ClientGame/ClientInteriorLayoutManager", "maxInteriorCreatesPerFrame", 0);

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
	if (ms_disableLazyInteriorLayoutCreation || ms_disableInteriorLayouts)
		return;

	// CONSULT-46 fix #2: per-frame create budget shared across ALL cells this frame
	// (ms_maxInteriorCreatesPerFrame; 0 = unlimited = original behavior). When spent we stop and
	// resume next frame from each cell's persisted created-count cursor (lives on CellProperty;
	// reset by removeFromWorld on unload -> no dangling pointer, no missing/duplicate objects).
	bool const throttled = (ms_maxInteriorCreatesPerFrame > 0);
	int        remainingBudget = ms_maxInteriorCreatesPerFrame;

	//-- find all visible cells
	RenderWorld::CellPropertyList const & cellPropertyList = RenderWorld::getVisibleCells();
	for (size_t i = 0; i < cellPropertyList.size(); ++i)
	{
		if (throttled && remainingBudget <= 0)
			break;   // budget spent this frame; unapplied cells stay eligible -> resume next frame

		CellProperty const * const cellProperty = cellPropertyList[i];
		if (!cellProperty || cellProperty == CellProperty::getWorldCellProperty())
			continue;

		//-- Verify that the cell is in the world
		if (!cellProperty->getOwner().isInWorld())
			continue;

		//-- If we have already applied the interior layout, skip this cell
		if (cellProperty->getAppliedInteriorLayout())
			continue;

		//-- Get the portal property's owner
		PortalProperty const * const portalProperty = cellProperty->getPortalProperty();
		if (!portalProperty)
		{
			cellProperty->setAppliedInteriorLayout();   // nothing to create -> latch done (orig behavior)
			continue;
		}

		Object & owner = const_cast<Object &>(portalProperty->getOwner());

		//-- Only tangible objects can have interior layouts
		TangibleObject * const tangibleObject = owner.asClientObject() ? owner.asClientObject()->asTangibleObject() : 0;
		if (!tangibleObject)
		{
			cellProperty->setAppliedInteriorLayout();
			continue;
		}

		InteriorLayoutReaderWriter const * const interiorLayout = tangibleObject->getInteriorLayout();
		if (!interiorLayout)
		{
			cellProperty->setAppliedInteriorLayout();
			continue;
		}

		TemporaryCrcString const cellName(cellProperty->getCellName(), true);
		int const numberOfObjects = interiorLayout->getNumberOfObjects(cellName);

		//-- Resume at the persisted cursor (0 first time; advanced if a prior frame ran out of
		//   budget mid-cell; reset to 0 by CellProperty::removeFromWorld on unload -> no dup/dangle).
		int objectIndex = cellProperty->getInteriorLayoutCreatedCount();
		for (; objectIndex < numberOfObjects; ++objectIndex)
		{
			if (throttled && remainingBudget <= 0)
				break;

			CrcString const & objectTemplateName = interiorLayout->getObjectTemplateName(cellName, objectIndex);
			Transform const & transform_o2p = interiorLayout->getTransform_o2p(cellName, objectIndex);

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

			if (throttled)
				--remainingBudget;
		}

		//-- Persist progress; latch "applied" ONLY when the whole cell is complete.
		cellProperty->setInteriorLayoutCreatedCount(objectIndex);
		if (objectIndex >= numberOfObjects)
			cellProperty->setAppliedInteriorLayout();
	}
}

// ----------------------------------------------------------------------

void ClientInteriorLayoutManager::applyInteriorLayout(TangibleObject * const tangibleObject, InteriorLayoutReaderWriter const * const interiorLayout, char const * const fileName)
{
	UNREF(fileName);

	if (!ms_disableLazyInteriorLayoutCreation || ms_disableInteriorLayouts)
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
