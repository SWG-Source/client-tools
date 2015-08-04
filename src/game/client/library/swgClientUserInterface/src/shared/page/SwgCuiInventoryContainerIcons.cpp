//======================================================================
//
// SwgCuiInventoryContainerIcons.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiInventoryContainerIcons.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiDragManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "sharedUtility/CallbackReceiver.h"
#include "sharedUtility/Callback.h"

#include "UIData.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIVolumePage.h"

#include <list>

//======================================================================

namespace SwgCuiInventoryContainerIconsNamespace
{
	const std::string cms_directdrop("directdrop");
	//----------------------------------------------------------------------

	class MyIconCallback : public CallbackReceiver
	{
	public:
		explicit MyIconCallback(SwgCuiInventoryContainerIcons & containerIcons);

		virtual void performCallback() 
		{
			m_containerIcons->onIconCallback ();
		}

		~MyIconCallback();

	private:
		// Not Implemented
		MyIconCallback();
		MyIconCallback & operator=(MyIconCallback const &);

	private:
		SwgCuiInventoryContainerIcons * m_containerIcons;
	};
	
	MyIconCallback::MyIconCallback (SwgCuiInventoryContainerIcons & containerIcons) :
	CallbackReceiver (),
	m_containerIcons (&containerIcons)
	{
	}

	MyIconCallback::~MyIconCallback()
	{
		m_containerIcons = 0;
	}

}

using namespace SwgCuiInventoryContainerIconsNamespace;

//----------------------------------------------------------------------


SwgCuiInventoryContainerIcons::SwgCuiInventoryContainerIcons (UIPage & page) :
CuiMediator         ("SwgCuiInventoryContainerIcons", page),
UIEventCallback     (),
m_volume            (0),
m_callback          (new MessageDispatch::Callback),
m_containerMediator (0),
m_updatingSelection (false),
m_originalIconSize  (0),
m_iconCallback      (0)
{
	m_iconCallback = new MyIconCallback (*this);

	//-- since this mediator is owned by a SwgCuiInventoryContainer, don't fetch it.

	getCodeDataObject (TUIVolumePage, m_volume, "volume");
	m_volume->Clear ();

	m_originalIconSize = m_volume->GetCellSize ().x;
}

//----------------------------------------------------------------------

SwgCuiInventoryContainerIcons::~SwgCuiInventoryContainerIcons ()
{
	DEBUG_FATAL (m_containerMediator, ("SwgCuiInventoryContainerIcons was destroyed with an outstanding m_containerMediator pointer"));

	// Release the container mediator.
	setContainerMediator(0);

	// Make lint happy.
	m_containerMediator = 0; 

	delete m_iconCallback;
	m_iconCallback = 0;

	delete m_callback;
	m_callback = 0;

	m_volume = 0;
}

//----------------------------------------------------------------------


void SwgCuiInventoryContainerIcons::setContainerMediator(SwgCuiInventoryContainer * containerMediator)
{
	if (containerMediator != m_containerMediator) 
	{
		if (m_containerMediator)
			m_containerMediator->release();

		m_containerMediator = containerMediator;

		if (m_containerMediator)
			m_containerMediator->fetch();
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::activateInContainer (SwgCuiInventoryContainer * containerMediator)
{
	setContainerMediator(containerMediator);

	activate();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::performActivate ()
{
	reset();

	NOT_NULL (m_containerMediator);
	m_callback->connect (m_containerMediator->getTransceiverListReset (), *this, &SwgCuiInventoryContainerIcons::onListReset);
	m_callback->connect (m_containerMediator->getTransceiverSelection (), *this, &SwgCuiInventoryContainerIcons::onSelection);

	m_volume->AddCallback (this);

	setIsUpdating (true);

	const long iconSize = static_cast<long>(static_cast<float>(m_originalIconSize) * CuiPreferences::getObjectIconSize ());
	m_volume->SetCellSize (UISize (iconSize, iconSize));

	CuiPreferences::getObjectIconCallback ().attachReceiver (*m_iconCallback);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::performDeactivate ()
{
	CuiPreferences::getObjectIconCallback ().detachReceiver (*m_iconCallback);

	setIsUpdating (false);

	NOT_NULL (m_containerMediator);
	
	m_containerMediator->clearViewers();

	m_volume->Clear();
	m_volume->RemoveCallback (this);

	m_callback->disconnect (m_containerMediator->getTransceiverListReset (), *this, &SwgCuiInventoryContainerIcons::onListReset);
	m_callback->disconnect (m_containerMediator->getTransceiverSelection (), *this, &SwgCuiInventoryContainerIcons::onSelection);

	setContainerMediator(0);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::OnHoverIn  (UIWidget *context)
{
	CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(context);
	if (viewer)
	{
		viewer->setDrawName    (true);
		viewer->setTextFadeOut (CuiWidget3dObjectListViewer::TOT_bottom, false);
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::OnHoverOut (UIWidget *context)
{
	CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(context);
	if (viewer)
	{
		if (!CuiPreferences::getShowIconNames () && context != m_volume->GetLastSelectedChild ())
			viewer->setTextFadeOut (CuiWidget3dObjectListViewer::TOT_bottom, true);
	}
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerIcons::OnMessage       (UIWidget *context, const UIMessage & msg )
{
	if (!m_containerMediator)
		return true;

	CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(context);
	
	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if (viewer && viewer->GetParent () == m_volume)
		{
			if (m_containerMediator->handleDoubleClick (*viewer))
				return false;
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::ContextRequest)
	{
		if (viewer && viewer->GetParent () == m_volume)
		{
			m_volume->SetSelection(viewer);
			return m_containerMediator->handleRadialMenu (*viewer, context->GetWorldLocation () + msg.MouseCoords);
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragStart)
	{
		if (context == m_volume)
		{
			if (msg.DragObject)
			{
				const CuiDragInfo cdinfo (*msg.DragObject);

				if (m_containerMediator->preHandleDragStart (cdinfo))
					return false;
			}
		}
	}

	//----------------------------------------------------------------------
	
	else if (msg.Type == UIMessage::DragEnd)
	{
		if (msg.DragTarget != context)
			return true;
		
		if (msg.DragObject)
		{
			CuiDragInfo cdinfo (*msg.DragObject);
			
			if (m_containerMediator->preHandleDrop (cdinfo))
				return false;

			if (cdinfo.type == CuiDragInfoTypes::CDIT_object)
			{
				std::string slotname;
				ClientObject * destination = findDropDestination (*context, slotname);
				if(slotname == cms_directdrop)
				{
					slotname.clear();
					//Disallow straight dragging and dropping on to container icons
					if(ContainerInterface::getVolumeContainer (*destination))
					{
						if(!CuiPreferences::getDragOntoContainers())
						{						
							//Perform the re-ordering functionality without triggering change of containment
							// (that's what the last "false" means)
							m_containerMediator->handleDrop (cdinfo, *destination, slotname, false);
						}
						else
						{
							m_containerMediator->handleDrop (cdinfo, *destination, slotname, true);
						}
						return false;
					}
					else
					{
						//Do the server stuff, but ask if we end up deciding on "/giveItem"
						// (that's what the last "true" means)
						m_containerMediator->handleDrop (cdinfo, *destination, slotname, true, true);
						return false;
					}
				}
				if (destination)
				{
					m_containerMediator->handleDrop (cdinfo, *destination, slotname);
				}
			}
		}
		
		return false;
	}
	
	//----------------------------------------------------------------------
	
	else if (msg.Type == UIMessage::DragOver)
	{
		context->SetDropFlagOk (true);
		
		
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

ClientObject * SwgCuiInventoryContainerIcons::findDropDestination (UIWidget & context, std::string & slotname)
{
	NOT_NULL (m_containerMediator);

	slotname.clear ();

	if (&context == m_volume)
	{
		slotname = m_containerMediator->getContainerSlotName ();
		return m_containerMediator->getContainerObject ();
	}
	else
	{
		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(&context);

		if (viewer)
		{
			slotname = cms_directdrop;
			return safe_cast<ClientObject *>(viewer->getLastObject ());
		}
	}

	return 0;
}

//----------------------------------------------------------------------

void  SwgCuiInventoryContainerIcons::OnVolumePageSelectionChanged (UIWidget * context)
{
	if (!m_containerMediator)
		return;

	if (context != m_volume)
		return;

	UIWidget * const child = m_volume->GetLastSelectedChild ();
	CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(child);

	if (viewer)
		viewer->setTextFadeOut (CuiWidget3dObjectListViewer::TOT_bottom, false);
	
	{
		const UIBaseObject::UIObjectList & olist = m_volume->GetChildrenRef ();
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			CuiWidget3dObjectListViewer * const child_viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(*it);
			if (child_viewer)
			{
				if (child_viewer != viewer )
					child_viewer->setTextFadeOut (CuiWidget3dObjectListViewer::TOT_bottom, !CuiPreferences::getShowIconNames ());
			}
		}
	}
	
	if (m_updatingSelection)
		return;

	UIVolumePage const * const volumePage = getVolumePage();
	bool const multiSelection = (volumePage != 0) ? volumePage->GetMultiSelection() : false;
	bool const doMultiSelection = (multiSelection) ? volumePage->GetDoMultiSelection() : false;

	ClientObject * const obj = viewer ? dynamic_cast<ClientObject *>(viewer->getLastObject ()) : 0;

	SwgCuiInventoryContainer::ObjectWatcherVector owv = m_containerMediator->getSelection();

	if (!doMultiSelection)
	{
		owv.clear();
	}

	if (obj != 0)
	{
		SwgCuiInventoryContainer::ObjectWatcher watcher(obj);
		SwgCuiInventoryContainer::ObjectWatcherVector::iterator ii = std::find(owv.begin(), owv.end(), watcher);

		if (ii != owv.end())
		{
			if ((multiSelection) && (*(ii) == watcher))
			{
				IGNORE_RETURN(owv.erase(ii));
			}
		}
		else
		{
			owv.push_back(watcher);
		}
	}

	m_containerMediator->setSelection(owv);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::reset ()
{
	NOT_NULL (m_containerMediator);

	updateIcons     ();
	updateSelection ();

	const ClientObject * const containerObj = m_containerMediator->getContainerObject ();

	if (containerObj)
		m_volume->SetLocalTooltip (containerObj->getLocalizedName ());
	else
		m_volume->SetLocalTooltip (Unicode::String ());
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::onListReset (const bool &)
{
	updateIcons     ();
	updateSelection ();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::onSelection (const std::pair<int, ClientObject *> & payload)
{
	UNREF (payload);
	updateSelection ();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::updateIcons()
{
	m_volume->Clear();
	NOT_NULL(m_containerMediator);

	// Get list of objects in the viewer to look for orphans.
	// The copy is intentional.
	SwgCuiInventoryContainer::ViewerMap currentViewerObjectMap = m_containerMediator->getViewers();

	// Only update icon container contents if necessary.
	bool volumeRequiresPacking = true;

	// Now iterate over current list of objects in the container.
	// Erase them from the set and add the icon if the object is valid.
	SwgCuiInventoryContainer::ObjectWatcherVector const & containedObjects = m_containerMediator->getObjects();
	for (SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator itContained = containedObjects.begin (); itContained != containedObjects.end (); ++itContained)
	{
		SwgCuiInventoryContainer::ObjectWatcher const & containedObjectWatcher = *itContained;

		ClientObject * const obj = containedObjectWatcher.getPointer();
		
		if (obj)
		{
			addObjectIcon(*obj);
			currentViewerObjectMap.erase(CachedNetworkId(obj->getNetworkId()));
			volumeRequiresPacking = true;
		}
	}

	// The remaining objects in the map are orphans and should be removed from the container.
	for (SwgCuiInventoryContainer::ViewerMap::iterator itOrphanedViewer = currentViewerObjectMap.begin(); itOrphanedViewer != currentViewerObjectMap.end(); ++itOrphanedViewer)
	{
		CuiWidget3dObjectListViewer * const viewer = itOrphanedViewer->second;
		if (viewer && (viewer->GetParent() == m_volume))
		{
			m_volume->RemoveChild(viewer);
			m_containerMediator->removeViewer(itOrphanedViewer->first, viewer);
			volumeRequiresPacking = true;
		}
	}

	// Update the volume.
	if (volumeRequiresPacking) 
	{
		m_volume->SetPackDirty(true);
	}
}


//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::updateSelection ()
{
	NOT_NULL (m_containerMediator);

	m_updatingSelection = true;

	SwgCuiInventoryContainer::ObjectWatcherVector const & selection = m_containerMediator->getSelection();

	if (!selection.empty())
	{
		SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator ii = selection.begin();
		SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator iiEnd = selection.end();
		
		for (; ii != iiEnd; ++ii)
		{
			SwgCuiInventoryContainer::ObjectWatcher const & watcher = *ii;
			ClientObject * const object = watcher.getPointer();

			UIWidget * const widget = (object != 0) ? m_containerMediator->getViewerForObject(*object, false) : 0;

			if ((widget == 0) || (widget->GetParent() == m_volume))
			{
				// first time through clear all selections
				m_volume->SetSelection(widget, (ii == selection.begin()));
				OnVolumePageSelectionChanged(m_volume);
			}
		}
	}
	else
	{
		m_volume->SetSelectionIndex(-1);
	}

	m_updatingSelection = false;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::addObjectIcon(ClientObject & obj)
{
	NOT_NULL(m_containerMediator);

	CuiWidget3dObjectListViewer * const viewer = NON_NULL(m_containerMediator->getViewerForObject(obj));

	if (viewer) 
	{
		viewer->setDrawName(CuiPreferences::getShowIconNames());
	
		if (!viewer->GetParent()) 
		{
			m_volume->AddChild(viewer);
			viewer->Link();

			if(CuiPreferences::getRotateInventoryObjects())
				viewer->setRotateSpeed(1.0f);
			else
			{
				viewer->setRotateSpeed(0.0f);
				viewer->SetRotation(0.0f);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::onIconCallback ()
{
	const long iconSize = static_cast<long>(static_cast<float>(m_originalIconSize) * CuiPreferences::getObjectIconSize ());
	m_volume->SetCellSize (UISize (iconSize, iconSize));

	const UIBaseObject::UIObjectList & olist = m_volume->GetChildrenRef ();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		CuiWidget3dObjectListViewer * const child_viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(*it);
		if (child_viewer)
		{
			if(CuiPreferences::getRotateInventoryObjects())
				child_viewer->setRotateSpeed(1.0f);
			else
			{
				child_viewer->setRotateSpeed(0.0f);
				child_viewer->SetRotation(0.0f);
			}
		}
	}

	OnVolumePageSelectionChanged (m_volume);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerIcons::openSelectedRadial()
{
	if(m_containerMediator && m_volume)
	{
		UIWidget *widget = m_volume->GetLastSelectedChild();
		CuiWidget3dObjectListViewer *viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(widget);
		if(viewer)
		{
			UIPoint p = viewer->GetWorldLocation();
			p.x += viewer->GetWidth() / 2;
			p.y += viewer->GetHeight() / 2;
			m_containerMediator->handleRadialMenu (*viewer, p);
		}
	}
}

//======================================================================
