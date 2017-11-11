//======================================================================
//
// SwgCuiInventoryContainerDetails.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiInventoryContainerDetails.h"

#include "clientGame/ContainerInterface.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiDragManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "swgClientUserInterface/SwgCuiInventoryContainerDetailsTableModel.h"

#include "UIData.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITable.h"


//======================================================================

namespace SwgCuiInventoryContainerDetailsNamespace
{
	class MyIconCallback : public CallbackReceiver
	{
	public:
		explicit MyIconCallback (SwgCuiInventoryContainerDetails & containerDetails);
		
		virtual void performCallback()
		{
			m_containerDetails->onIconCallback();
		}

		~MyIconCallback()
		{
			m_containerDetails = 0;
		}

	private:
		// Not implemented.
		MyIconCallback();
		MyIconCallback & operator=(MyIconCallback const &);

	private:
		SwgCuiInventoryContainerDetails * m_containerDetails;
	};

	MyIconCallback::MyIconCallback (SwgCuiInventoryContainerDetails & containerDetails) :
	CallbackReceiver (),
	m_containerDetails (&containerDetails)
	{
	}
}

using namespace SwgCuiInventoryContainerDetailsNamespace;

//----------------------------------------------------------------------

SwgCuiInventoryContainerDetails::SwgCuiInventoryContainerDetails (UIPage & page) :
CuiMediator         ("SwgCuiInventoryContainerDetails", page),
UIEventCallback     (),
m_table             (0),
m_tableModel        (new TableModel),
m_containerMediator (0),
m_callback          (new MessageDispatch::Callback),
m_updatingSelection (false),
m_originalRowHeight (0),
m_iconCallback      (0)
{
	m_iconCallback = new MyIconCallback (*this);

	//-- since this mediator is owned by a SwgCuiInventoryContainer, don't fetch it.

	getCodeDataObject (TUITable, m_table, "table");

	m_tableModel->Attach (0);
	getPage ().AddChild (m_tableModel);
	m_table->SetTableModel (m_tableModel);

	m_tableModel->updateTableColumnSizes (*m_table);

	m_originalRowHeight = m_table->GetCellHeight ();
}

//----------------------------------------------------------------------

SwgCuiInventoryContainerDetails::~SwgCuiInventoryContainerDetails ()
{
	DEBUG_FATAL (m_containerMediator, ("SwgCuiInventoryContainerDetails was destroyed with an outstanding m_containerMediator pointer"));
	
	// Release the mediator.
	setContainerMediator(0);
	
	// Make lint happy.
	m_containerMediator = 0; 

	delete m_iconCallback;
	m_iconCallback = 0;

	delete m_callback;
	m_callback = 0;

	m_table->SetTableModel (0);
	m_table = 0;

	m_tableModel->Detach (0);
	m_tableModel = 0;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::setContainerMediator(SwgCuiInventoryContainer * containerMediator)
{
	if (m_containerMediator != containerMediator) 
	{
		if (m_containerMediator)
			m_containerMediator->release ();

		m_containerMediator = containerMediator;

		if (m_containerMediator)
			m_containerMediator->fetch ();
	
		m_tableModel->setContainerMediator(m_containerMediator);
		
		const long iconSize = static_cast<long>(static_cast<float>(m_originalRowHeight) * CuiPreferences::getObjectIconSize ());
		m_table->SetCellHeight (iconSize);
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::activateInContainer (SwgCuiInventoryContainer * containerMediator)
{
	setContainerMediator(containerMediator);

	activate();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::performActivate ()
{
	CuiPreferences::getObjectIconCallback ().attachReceiver (*m_iconCallback);

	m_tableModel->fireColumnsChanged ();
	m_tableModel->fireDataChanged ();

	NOT_NULL (m_containerMediator);
	m_callback->connect (m_containerMediator->getTransceiverListReset (), *this, &SwgCuiInventoryContainerDetails::onListReset);
	m_callback->connect (m_containerMediator->getTransceiverSelection (), *this, &SwgCuiInventoryContainerDetails::onSelection);

	updateSelection ();
	applySelection  (true);

	m_table->AddCallback (this);

	const SwgCuiInventoryContainer::ViewerMap & vm = m_containerMediator->getViewers ();

	for (SwgCuiInventoryContainer::ViewerMap::const_iterator it = vm.begin (); it != vm.end (); ++it)
	{
		CuiWidget3dObjectListViewer * const viewer = (*it).second;
		NOT_NULL (viewer);
		viewer->setDrawName (false);
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::performDeactivate ()
{
	CuiPreferences::getObjectIconCallback ().detachReceiver (*m_iconCallback);

	m_table->RemoveCallback (this);

	NOT_NULL (m_containerMediator);
	m_callback->disconnect (m_containerMediator->getTransceiverListReset (), *this, &SwgCuiInventoryContainerDetails::onListReset);
	m_callback->disconnect (m_containerMediator->getTransceiverSelection (), *this, &SwgCuiInventoryContainerDetails::onSelection);

	SwgCuiInventoryContainer::ViewerMap const & vm = m_containerMediator->getViewers();

	for (SwgCuiInventoryContainer::ViewerMap::const_iterator it = vm.begin (); it != vm.end (); ++it)
	{
		CuiWidget3dObjectListViewer * const viewer = (*it).second;
		NOT_NULL(viewer);
		if (viewer != 0)
		{
			viewer->setDrawName (true);
		}
	}

	applyStates ();

	setContainerMediator(0);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::applyStates ()
{
	NOT_NULL (m_containerMediator);

	const int count = static_cast<int>(m_containerMediator->getObjects ().size ());

	SwgCuiInventoryContainer::IntVector iv;
	iv.reserve (count);

	for (int i = 0; i < count; ++i)
	{
		const int index = m_tableModel->GetLogicalDataRowIndex (i);

		if (index < 0)
			return;

		iv.push_back (index);
	}

	m_containerMediator->setObjectSorting (iv);

	applySelection (false);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::applySelection (bool visual)
{
	NOT_NULL (m_containerMediator);

	m_updatingSelection = true;

	const UITable::LongVector & sel = m_table->GetSelectedRows ();

	SwgCuiInventoryContainer::ObjectWatcherVector owv;
	owv.reserve (sel.size ());

	const SwgCuiInventoryContainer::ObjectWatcherVector & objects = m_containerMediator->getObjects ();
	const int numObjects = objects.size ();

	for (UITable::LongVector::const_iterator it = sel.begin (); it != sel.end (); ++it)
	{
		const int index = static_cast<int>(*it);

		if (index >= 0 && index < numObjects)
		{
			if (visual)
			{
				const int logicalIndex = m_tableModel->GetLogicalDataRowIndex (index);

				if (logicalIndex >= 0)
					owv.push_back (objects [logicalIndex]);
			}
			else
				owv.push_back (objects [index]);
		}
	}

	m_containerMediator->setSelection (owv);
	m_updatingSelection = false;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerDetails::OnMessage       (UIWidget *context, const UIMessage & msg )
{
	if (!m_containerMediator)
		return true;

	if (context == m_table)
	{
		long rowVisual = 0;
		if (!m_table->GetCellFromPoint (msg.MouseCoords, &rowVisual, 0))
			return true;

		const long rowLogical = m_tableModel->GetLogicalDataRowIndex (rowVisual);

		CuiWidget3dObjectListViewer * const viewer = m_containerMediator->getWidgetAtPosition (rowLogical);

		//----------------------------------------------------------------------

		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			if (viewer)
			{
				m_containerMediator->handleDoubleClick (*viewer);
				return false;
			}
		}

		//----------------------------------------------------------------------

		else if (msg.Type == UIMessage::ContextRequest)
		{
			if (viewer)
			{
				return m_containerMediator->handleRadialMenu (*viewer, context->GetWorldLocation () + msg.MouseCoords);
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

				if (cdinfo.type == CuiDragInfoTypes::CDIT_object)
				{
					std::string slotname;
					ClientObject * destination = findDropDestination (viewer, slotname);
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
			context->SetDropFlagOk (false);

			if (msg.DragObject)
			{
				CuiDragInfo cdinfo (*msg.DragObject);

				if (cdinfo.type == CuiDragInfoTypes::CDIT_object)
				{
					std::string slotname;
					ClientObject * destination = findDropDestination (viewer, slotname);

					if (destination && destination != cdinfo.objectId.getObject ())
					{
						if (CuiDragManager::isDropOk (cdinfo, *destination, slotname))
							context->SetDropFlagOk (true);
					}
				}
			}

			return false;
		}

		return true;
	}

	return true;
}

//----------------------------------------------------------------------

ClientObject * SwgCuiInventoryContainerDetails::findDropDestination (CuiWidget3dObjectListViewer * viewer, std::string & slotname)
{
	NOT_NULL (m_containerMediator);

	slotname.clear ();

	if (viewer)
	{
		ClientObject * const obj = safe_cast<ClientObject *>(viewer->getLastObject ());

		if (obj && ContainerInterface::getVolumeContainer (*obj))
			return obj;
	}

	slotname = m_containerMediator->getContainerSlotName ();
	return m_containerMediator->getContainerObject ();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::onListReset (const bool &)
{
	m_tableModel->fireColumnsChanged ();
	m_tableModel->fireDataChanged ();
	updateSelection ();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::onSelection (const std::pair<int, ClientObject *> & payload)
{
	UNREF (payload);
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_table)
	{
		if (m_updatingSelection)
			return;

		applySelection (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::updateSelection ()
{
	NOT_NULL (m_containerMediator);

	m_updatingSelection = true;

	const SwgCuiInventoryContainer::ObjectWatcherVector & objects = m_containerMediator->getObjects ();
	const SwgCuiInventoryContainer::ObjectWatcherVector & sel     = m_containerMediator->getSelection ();

	m_table->SelectRow (-1);

	if (!sel.empty ())
	{
		const SwgCuiInventoryContainer::ObjectWatcher & watcher = sel.back ();
		const SwgCuiInventoryContainer::ObjectWatcherVector::const_iterator it = std::find (objects.begin (), objects.end (), watcher);

		if (it != objects.end ())
		{
			const int logicalIndex = std::distance (objects.begin (), it);
			const int visualRow = m_tableModel->GetVisualDataRowIndex (logicalIndex);

			if (visualRow >= 0)
				m_table->AddRowSelection (visualRow);
		}
	}

	m_updatingSelection = false;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::setType (int type)
{
	m_tableModel->setType (type);
}

//----------------------------------------------------------------------

int SwgCuiInventoryContainerDetails::getType () const
{
	return m_tableModel->getType ();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::onIconCallback ()
{
	const long iconSize = static_cast<long>(static_cast<float>(m_originalRowHeight) * CuiPreferences::getObjectIconSize ());

	if (m_table->GetCellHeight () != iconSize)
	{
		m_table->SetCellHeight (iconSize);
		m_tableModel->fireColumnsChanged ();
		m_tableModel->fireDataChanged ();
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::setSortOnColumnState(int column, UITableModel::SortDirection direction)
{
	m_tableModel->sortOnColumn(column, direction);
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerDetails::getSortOnColumnState(int & column, UITableModel::SortDirection & direction)
{
	return m_tableModel->getSortOnColumnState(column, direction);
}

//----------------------------------------------------------------------

int SwgCuiInventoryContainerDetails::getDefaultSortColumn() const
{
	return m_tableModel->getDefaultSortColumn();
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetails::setDragable(bool const dragable)
{
	if (m_table)
		m_table->SetDragable(dragable);
}

//======================================================================
