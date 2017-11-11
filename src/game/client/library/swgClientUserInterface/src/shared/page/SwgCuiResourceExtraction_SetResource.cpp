//======================================================================
//
// SwgCuiResourceExtraction_SetResource.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_SetResource.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_SetResource_TableModel.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Quantity.h"

#include "clientGame/ClientInstallationSynchronizedUi.h"
#include "clientGame/Game.h"
#include "clientGame/InstallationObject.h"
#include "clientGame/ResourceIconManager.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiResourceManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"

#include "UnicodeUtils.h"

#include <algorithm>
#include <list>


//======================================================================

namespace SwgCuiResourceExtraction_SetResourceNamespace
{
	const float maxRange = 32.0f;
}

using namespace SwgCuiResourceExtraction_SetResourceNamespace;

//======================================================================

SwgCuiResourceExtraction_SetResource::SwgCuiResourceExtraction_SetResource (UIPage & page)
: CuiMediator           ("SwgCuiResourceExtraction_SetResource", page),
  UIEventCallback       (),
  m_setResourceTable    (0),
  m_setResourceContents (new HopperContentsVector),
  m_resourceTableModel  (0),
  m_buttonOk            (0),
  m_buttonCancel        (0),
  m_resourceIcons       (),
  m_resources           (),
  m_names               (),
  m_efficiencies        ()
{
	getCodeDataObject (TUITable,      m_setResourceTable,       "SetResourceTable");

	getCodeDataObject (TUIButton,     m_buttonOk,               "buttonOk");
	getCodeDataObject (TUIButton,     m_buttonCancel,           "buttonCancel");

	m_resourceTableModel = new TableModel(*this);
	m_resourceTableModel->Attach (0);
	m_resourceTableModel->SetName("SetResourceTableModel");
	getPage().AddChild(m_resourceTableModel);
	m_setResourceTable->SetTableModel (m_resourceTableModel);
	m_resourceTableModel->updateTableColumnSizes (*m_setResourceTable);
	m_setResourceTable->SetCellHeight(48);

	m_resourceTableModel->fireDataChanged();
	m_resourceTableModel->fireColumnsChanged();

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));
}

//----------------------------------------------------------------------

SwgCuiResourceExtraction_SetResource::~SwgCuiResourceExtraction_SetResource ()
{
	delete m_setResourceContents;
	m_setResourceContents = 0;

	m_resourceTableModel->Detach (0);
	m_resourceTableModel = 0;

	m_buttonOk               = 0;
	m_buttonCancel           = 0;
	m_setResourceTable       = 0;
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_SetResource::performActivate ()
{
	update ();

	CuiResourceManager::requestResourceData ();
	m_setResourceTable->AddCallback         (this);
	m_buttonOk->AddCallback                 (this);
	m_buttonCancel->AddCallback             (this);

	getPage().SetFocus();

	InstallationObject * const harvester = CuiResourceManager::getHarvesterInstallation ();
	if(!harvester)
	{
		DEBUG_WARNING(true, ("No harvester"));
		return;
	}

	setAssociatedObjectId(harvester->getNetworkId());
	setMaxRangeFromObject(maxRange);

	m_resourceTableModel->fireDataChanged   ();
	m_resourceTableModel->fireColumnsChanged();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_SetResource::performDeactivate ()
{
	m_setResourceTable->RemoveCallback (this);
	m_buttonOk->RemoveCallback         (this);
	m_buttonCancel->RemoveCallback     (this);
	clearResources ();

	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_SetResource::OnButtonPressed    (UIWidget *context)
{
	if (context == m_buttonOk)
	{
		int visualRow = m_setResourceTable->GetLastSelectedRow();
		int row       = m_resourceTableModel->GetLogicalDataRowIndex (visualRow);
		if (row >= 0 && row < static_cast<int>(m_resources.size()))
		{
			NetworkId resource = m_resources[static_cast<unsigned int>(row)];

			Object * const player = Game::getPlayer ();
			InstallationObject * const harvester = CuiResourceManager::getHarvesterInstallation ();

			if (harvester && player)
				harvester->selectHarvesterResource (*player, resource);
		}
		deactivate ();
	}
	if (context == m_buttonCancel)
	{
		deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_SetResource::update ()
{
	const CuiResourceManager::Synchronized * sync = CuiResourceManager::getSynchronized ();

	clearResources();

	std::vector<NetworkId> pools = sync->getResourcePools();
	for(std::vector<NetworkId>::iterator i = pools.begin(); i != pools.end(); ++i)
	{
		const CuiResourceManager::ResourceInfo* info = CuiResourceManager::findResourceInfo(*i);
		if(info)
			addResource(*i, info->name, info->efficiency);
	}

	m_resourceTableModel->fireDataChanged();
	m_resourceTableModel->fireColumnsChanged();

	//auto select the top row if nothing is selected
	int numSelectedRows = 0;
	m_setResourceTable->GetPropertyInteger (UITable::PropertyName::SelectedRowCount, numSelectedRows);
	if(numSelectedRows == 0)
	{
		m_setResourceTable->SelectRow(0);
	}
}

//----------------------------------------------------------------------

NetworkId SwgCuiResourceExtraction_SetResource::getResourceAtPosition (int index)
{
	if(index >= getNumResources())
	{
		DEBUG_WARNING(true, ("Asked for for resource index %s when max is %d", index, getNumResources()));
		return NetworkId::cms_invalid;
	}
	return m_resources[static_cast<unsigned int>(index)];
}

//----------------------------------------------------------------------

int SwgCuiResourceExtraction_SetResource::getResourceEfficiencyAtPosition (int index)
{
	if(index >= getNumResources())
	{
		DEBUG_WARNING(true, ("Asked for for resource efficiency index %s when max is %d", index, getNumResources()));
		return 0;
	}
	return m_efficiencies[static_cast<unsigned int>(index)];
}

//----------------------------------------------------------------------

Unicode::String SwgCuiResourceExtraction_SetResource::getResourceNameAtPosition (int index)
{
	if(index >= getNumResources())
	{
		DEBUG_WARNING(true, ("Asked for for resource name index %s when max is %d", index, getNumResources()));
		return Unicode::String();
	}
	return m_names[static_cast<unsigned int>(index)];
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer * SwgCuiResourceExtraction_SetResource::getResourceIconAtPosition (int index)
{
	if(index >= getNumResources())
	{
		DEBUG_WARNING(true, ("Asked for for resource icon index %s when max is %d", index, getNumResources()));
		return NULL;
	}
	return m_resourceIcons[static_cast<unsigned int>(index)];
}

//----------------------------------------------------------------------

int SwgCuiResourceExtraction_SetResource::getNumResources () const
{
	return static_cast<int>(m_resources.size());
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_SetResource::clearResources ()
{
	m_resources.clear();
	m_names.clear();
	//Nulls can theoretically exist in this vector, since we want to maintain vectors of the same size
	for(std::vector<CuiWidget3dObjectListViewer*>::iterator i = m_resourceIcons.begin (); i != m_resourceIcons.end (); ++i)
	{
		if(*i)
			(*i)->Detach(0);
	}
	m_resourceIcons.clear();
	m_efficiencies.clear();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_SetResource::addResource(const NetworkId& resourceId, const Unicode::String& name, int efficiency)
{
	m_resources.push_back(resourceId);
	m_names.push_back(name);
	m_efficiencies.push_back(efficiency);

	Object* obj = ResourceIconManager::getObjectForType(resourceId);
	CuiWidget3dObjectListViewer * viewer = 0;
	if(obj)
	{
		viewer = CuiIconManager::createObjectIcon(*obj, 0);
		viewer->SetDragable             (false);
		viewer->SetContextCapable       (false, false);
		viewer->setRotateSpeed          (0.0f);
		viewer->Attach (0);
		m_resourceIcons.push_back(viewer);
	}
	else
	{
		DEBUG_WARNING(true, ("Could not build an icon object for resource %d", resourceId.getValue()));
		m_resourceIcons.push_back(NULL);
	}
}

//-----------------------------------------------------------------------

bool SwgCuiResourceExtraction_SetResource::OnMessage(UIWidget *context, const UIMessage & msg)
{
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if(context == m_setResourceTable)
			m_buttonOk->Press();
	}

	return true;
}

//----------------------------------------------------------------------
