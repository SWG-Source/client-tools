// ======================================================================
//
// SwgCuiInventoryContainerDetails.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiInventoryContainerDetails_H
#define INCLUDED_SwgCuiInventoryContainerDetails_H

// ======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "UITableModel.h"

// ======================================================================

class CachedNetworkId;
class CallbackReceiver;
class ClientObject;
class CuiWidget3dObjectListViewer;
class Object;
class SwgCuiInventoryContainer;
class SwgCuiInventoryContainerDetailsTableModel;
class UIButton;
class UITable;
class UIText;
class UITextStyle;
class UIVolumePage;
class VolumeContainer;

namespace MessageDispatch
{
	class Callback;
};

namespace SwgCuiInventoryContainerDetailsNamespace
{
	class MyIconCallback;
}

//-----------------------------------------------------------------

class SwgCuiInventoryContainerDetails:
public CuiMediator,
public UIEventCallback
{
public:

	typedef SwgCuiInventoryContainerDetailsTableModel TableModel;

	explicit                   SwgCuiInventoryContainerDetails (UIPage & page);

	void                       activateInContainer       (SwgCuiInventoryContainer * containerMediator);

	bool                       OnMessage                 (UIWidget *context, const UIMessage & msg );
	void                       OnGenericSelectionChanged (UIWidget * context);

	ClientObject *             findDropDestination       (CuiWidget3dObjectListViewer * viewer, std::string & slotname);

	void                       onListReset               (const bool &);
	void                       onSelection               (const std::pair<int, ClientObject *> & payload);

	void                       setType                   (int type);
	int                        getType                   () const;

	void                       onIconCallback            ();

	void setSortOnColumnState(int column, UITableModel::SortDirection direction);
	bool getSortOnColumnState(int & column, UITableModel::SortDirection & direction);
	int getDefaultSortColumn() const;

	void setDragable(bool const dragable);

protected:
	void                       performActivate           ();
	void                       performDeactivate         ();

private:
	                          ~SwgCuiInventoryContainerDetails ();
	                           SwgCuiInventoryContainerDetails ();
	                           SwgCuiInventoryContainerDetails (const SwgCuiInventoryContainerDetails & rhs);
	SwgCuiInventoryContainerDetails & operator= (const SwgCuiInventoryContainerDetails & rhs);

	void                       setContainerMediator (SwgCuiInventoryContainer * containerMediator);

	void                       applyStates        ();
	void                       applySelection     (bool visual);
	void                       updateSelection    ();

	UITable *                  m_table;

	TableModel *               m_tableModel;

	SwgCuiInventoryContainer *  m_containerMediator;
	MessageDispatch::Callback * m_callback;

	bool                        m_updatingSelection;

	long                        m_originalRowHeight;
	SwgCuiInventoryContainerDetailsNamespace::MyIconCallback * m_iconCallback;
};

// ======================================================================

#endif

