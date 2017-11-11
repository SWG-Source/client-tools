//======================================================================
//
// SwgCuiInventoryContainerDetailsTableModel.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiInventoryContainerDetailsTableModel.h"

#include "UITable.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"

//======================================================================


namespace
{
	namespace ColumnInfo
	{
		enum Id
		{
			I_none,
			I_icon,
			I_name,
			I_type,
			I_volume,
			I_equipped,
			I_planet,
			I_count
		};

		struct ColumnData
		{
			StringId            name;
			UITableTypes::CellType cellType;

			ColumnData () :
				name (),
				cellType (UITableTypes::CT_none)
			{
			}

			ColumnData (const StringId & _name, UITableTypes::CellType _cellType) :
				name (_name),
				cellType (_cellType)
			{
				}
		};

		ColumnData data [I_count];

		//-- install the data
		void install ()
		{
			static bool installed = false;

			if (installed)
				return;

			installed = true;

			data [I_none]       = ColumnData (StringId ("NONE"),                   UITableTypes::CT_none);
			data [I_icon]       = ColumnData (CuiStringIds::inv_details_icon,      UITableTypes::CT_widget);
			data [I_name]       = ColumnData (CuiStringIds::inv_details_name,      UITableTypes::CT_text);
			data [I_type]       = ColumnData (CuiStringIds::inv_details_type,      UITableTypes::CT_text);
			data [I_volume]     = ColumnData (CuiStringIds::inv_details_volume,    UITableTypes::CT_integer);
			data [I_equipped]   = ColumnData (CuiStringIds::inv_details_equipped,  UITableTypes::CT_bool);
			data [I_planet]     = ColumnData (CuiStringIds::inv_details_planet,    UITableTypes::CT_text);
		}

		namespace InventorySelf
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_icon,
				I_name,
				I_type,
				I_volume
			};
		}

		namespace Waypoints
		{
			const int numColumns = 3;
			const Id ids [numColumns] =
			{
				I_icon,
				I_name,
				I_planet
			};
		}

		namespace Poi
		{
			const int numColumns = 3;
			const Id ids [numColumns] =
			{
				I_icon,
				I_name,
				I_planet
			};
		}

		namespace CraftIngredients
		{
			const int numColumns = 3;
			const Id ids [numColumns] =
			{
				I_icon,
				I_name,
				I_type
			};
		}

		namespace DraftSchematics
		{
			const int numColumns = 3;
			const Id ids [numColumns] =
			{
				I_icon,
				I_name,
				I_type
			};
		}

		Id findColumnId (int type, int col)
		{
			DEBUG_FATAL (col < 0, ("bad col"));

			const SwgCuiInventoryContainer::Type ctype = static_cast<SwgCuiInventoryContainer::Type>(type);

			switch (ctype)
			{
			case SwgCuiInventoryContainer::T_inventorySelf:
			case SwgCuiInventoryContainer::T_datapadSelf:
				DEBUG_FATAL (col >= ColumnInfo::InventorySelf::numColumns, ("bad col"));
				return ColumnInfo::InventorySelf::ids [col];
			case SwgCuiInventoryContainer::T_craftIngredients:
				DEBUG_FATAL (col >= ColumnInfo::CraftIngredients::numColumns, ("bad col"));
				return ColumnInfo::CraftIngredients::ids [col];
			case SwgCuiInventoryContainer::T_draftSchematics:
				DEBUG_FATAL (col >= ColumnInfo::DraftSchematics::numColumns, ("bad col"));
				return ColumnInfo::DraftSchematics::ids [col];
			case SwgCuiInventoryContainer::T_waypointsSelf:
				DEBUG_FATAL (col >= ColumnInfo::Waypoints::numColumns, ("bad col"));
				return ColumnInfo::Waypoints::ids [col];
			case SwgCuiInventoryContainer::T_poi:
				DEBUG_FATAL (col >= ColumnInfo::Poi::numColumns, ("bad col"));
				return ColumnInfo::Poi::ids [col];
			default:
				return I_none;
			}
		}

		int findNameColumn(ColumnInfo::Id const * const ids, int idSize)
		{
			for (int i = 0; i < idSize; ++i)
			{
				if (ids[i] == ColumnInfo::I_name)
				{
					return i;
				}
			}
			return 0;
		}
	}
}

//----------------------------------------------------------------------

SwgCuiInventoryContainerDetailsTableModel::SwgCuiInventoryContainerDetailsTableModel () :
UITableModel        (),
m_type              (static_cast<int>(SwgCuiInventoryContainer::T_inventorySelf)),
m_containerMediator (0)
{
	ColumnInfo::install ();
	SetName ("SwgCuiInventoryContainerDetailsTableModel");
}

//----------------------------------------------------------------------

SwgCuiInventoryContainerDetailsTableModel::~SwgCuiInventoryContainerDetailsTableModel ()
{
	m_containerMediator = 0;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetailsTableModel::setContainerMediator      (SwgCuiInventoryContainer * containerMediator)
{
	if (containerMediator)
		containerMediator->fetch ();

	if (m_containerMediator)
		m_containerMediator->release ();

	m_containerMediator = containerMediator;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerDetailsTableModel::GetValueAtText   (int visualRow, int col, UIString     & value)  const
{
	if (!m_containerMediator)
		return false;

	const int row = GetLogicalDataRowIndex (visualRow);

	ClientObject * const obj = m_containerMediator->getObjectAtPosition (row);

	if (!obj)
		return false;

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_name:
		value = obj->getLocalizedName ();
		break;
	case ColumnInfo::I_type:
		{
			const int got       = obj->getGameObjectType ();
			const int superType = GameObjectTypes::getMaskedType (got);
			if (got == superType)
				value = obj->getGameObjectTypeLocalizedName ();
			else
			{
				value = GameObjectTypes::getLocalizedName (superType);
				value.push_back (',');
				value.push_back (' ');
				value += obj->getGameObjectTypeLocalizedName ();
			}
		}
		break;

	case ColumnInfo::I_planet:
		{
			const ClientWaypointObject* const wp = dynamic_cast<const ClientWaypointObject* const>(obj);
			if(wp)
				value = StringId("planet_n", wp->getPlanetName()).localize();
		}
		break;

	case ColumnInfo::I_none:
	case ColumnInfo::I_icon:
	case ColumnInfo::I_volume:
	case ColumnInfo::I_equipped:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerDetailsTableModel::GetValueAtBool   (int visualRow, int col, bool & value)          const
{
	if (!m_containerMediator)
		return false;

	const int row = GetLogicalDataRowIndex (visualRow);

	ClientObject * const obj = m_containerMediator->getObjectAtPosition (row);

	if (!obj)
		return false;

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_equipped:
		value = true;
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_icon:
	case ColumnInfo::I_name:
	case ColumnInfo::I_type:
	case ColumnInfo::I_volume:
	case ColumnInfo::I_planet:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerDetailsTableModel::GetValueAtInteger(int visualRow, int col, int & value)           const
{
	if (!m_containerMediator)
		return false;

	const int row = GetLogicalDataRowIndex (visualRow);

	ClientObject * const obj = m_containerMediator->getObjectAtPosition (row);

	if (!obj)
		return false;

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_volume:
		value = std::max (1, obj->getVolume ());
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_icon:
	case ColumnInfo::I_name:
	case ColumnInfo::I_type:
	case ColumnInfo::I_equipped:
	case ColumnInfo::I_planet:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerDetailsTableModel::GetValueAtWidget (int visualRow, int col, UIWidget *& value)     const
{
	if (!m_containerMediator)
		return false;

	const int row = GetLogicalDataRowIndex (visualRow);

	ClientObject * const obj = m_containerMediator->getObjectAtPosition (row);

	if (!obj)
		return false;

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_icon:
		value = m_containerMediator->getWidgetAtPosition (row);
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_name:
	case ColumnInfo::I_type:
	case ColumnInfo::I_volume:
	case ColumnInfo::I_equipped:
	case ColumnInfo::I_planet:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerDetailsTableModel::GetLocalizedColumnName (int col, UIString & str) const
{
	if (col < 0 || col >= GetColumnCount ())
		return false;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case SwgCuiInventoryContainer::T_inventorySelf:
	case SwgCuiInventoryContainer::T_datapadSelf:
		str = data [InventorySelf::ids [col]].name.localize ();
		break;
	case SwgCuiInventoryContainer::T_craftIngredients:
		str = data [CraftIngredients::ids [col]].name.localize ();
		break;
	case SwgCuiInventoryContainer::T_draftSchematics:
		str = data [DraftSchematics::ids [col]].name.localize ();
		break;
	case SwgCuiInventoryContainer::T_waypointsSelf:
		str = data [Waypoints::ids [col]].name.localize ();
		break;
	case SwgCuiInventoryContainer::T_poi:
		str = data [Poi::ids [col]].name.localize ();
		break;
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryContainerDetailsTableModel::GetLocalizedColumnTooltip (int col, UIString & str) const
{
	return GetLocalizedColumnName (col, str);
}

//----------------------------------------------------------------------

UITableTypes::CellType SwgCuiInventoryContainerDetailsTableModel::FindColumnCellType     (int col) const
{
	if (col < 0 || col >= FindColumnCount ())
		return UITableTypes::CT_none;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case SwgCuiInventoryContainer::T_inventorySelf:
	case SwgCuiInventoryContainer::T_datapadSelf:
		return data [InventorySelf::ids [col]].cellType;
	case SwgCuiInventoryContainer::T_craftIngredients:
		return data [CraftIngredients::ids [col]].cellType;
	case SwgCuiInventoryContainer::T_draftSchematics:
		return data [DraftSchematics::ids [col]].cellType;
	case SwgCuiInventoryContainer::T_waypointsSelf:
		return data [Waypoints::ids [col]].cellType;
	case SwgCuiInventoryContainer::T_poi:
		return data [Poi::ids [col]].cellType;
	default:
		return UITableTypes::CT_none;
	}
}

//----------------------------------------------------------------------

int SwgCuiInventoryContainerDetailsTableModel::FindColumnCount        () const
{
	using namespace ColumnInfo;

	switch (m_type)
	{
	case SwgCuiInventoryContainer::T_inventorySelf:
	case SwgCuiInventoryContainer::T_datapadSelf:
		return InventorySelf::numColumns;
	case SwgCuiInventoryContainer::T_craftIngredients:
		return CraftIngredients::numColumns;
	case SwgCuiInventoryContainer::T_draftSchematics:
		return DraftSchematics::numColumns;
	case SwgCuiInventoryContainer::T_waypointsSelf:
		return Waypoints::numColumns;
	case SwgCuiInventoryContainer::T_poi:
		return Poi::numColumns;
	default:
		return 0;
	}
}

//----------------------------------------------------------------------

int SwgCuiInventoryContainerDetailsTableModel::FindRowCount           () const
{
	if (!m_containerMediator)
		return 0;

	return static_cast<int>(m_containerMediator->getObjects ().size ());
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetailsTableModel::setType (int type)
{
	m_type = type;

	fireColumnsChanged ();
	fireDataChanged ();
}

//----------------------------------------------------------------------

UIWidget * SwgCuiInventoryContainerDetailsTableModel::GetDragWidgetAt        (int visualRow, int )
{
	if (!m_containerMediator)
		return 0;

	const int logicalRow = GetLogicalDataRowIndex (visualRow);
	CuiWidget3dObjectListViewer * const dragWidget = m_containerMediator->getWidgetAtPosition (logicalRow);

	if (dragWidget)
	{
		UIPoint dummy;
		return dragWidget->GetCustomDragWidget (UIPoint::zero, dummy);
	}

	return dragWidget;
}

//----------------------------------------------------------------------

void SwgCuiInventoryContainerDetailsTableModel::updateTableColumnSizes (UITable & table) const
{
	//-- copy vector
	UITable::ColumnSizeInfoVector csiv = table.GetColumnSizeInfo ();
	const int columnCount = FindColumnCount ();
	csiv.resize (columnCount);

	int numToDivide = columnCount;

	long widthToDivide = table.GetTotalColumnAvailableWidth ();

	for (int i = 0; i < columnCount; ++i)
	{
		const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, i);
		UITable::ColumnSizeInfo & csi = csiv [i];

		switch (id)
		{
		case ColumnInfo::I_icon:
			csi.constant = true;
			csi.width    = table.GetCellHeight () * 2L;
			widthToDivide -= csi.width;
			--numToDivide;
			break;
		case ColumnInfo::I_name:
		case ColumnInfo::I_type:
		case ColumnInfo::I_planet:
			break;
		case ColumnInfo::I_volume:
			csi.constant = true;
			csi.width    = table.GetCellHeight () * 2L;
			widthToDivide -= csi.width;
			--numToDivide;
			break;
		case ColumnInfo::I_equipped:
			csi.constant = true;
			csi.width    = table.GetCellHeight ();
			widthToDivide -= csi.width;
			--numToDivide;
			break;
		case ColumnInfo::I_none:
		case ColumnInfo::I_count:
		default:
			break;
		}

		if (numToDivide)
		{
			const float prop = 1.0f / static_cast<float>(numToDivide);

			for (UITable::ColumnSizeInfoVector::iterator it = csiv.begin (); it != csiv.end (); ++it)
			{
				UITable::ColumnSizeInfo & csi = *it;
				csi.proportion = prop;
			}
		}
	}

	table.SetColumnSizeInfo (csiv);
}

//----------------------------------------------------------------------

int SwgCuiInventoryContainerDetailsTableModel::getDefaultSortColumn() const
{
	switch (m_type)
	{
		case SwgCuiInventoryContainer::T_inventorySelf:
		case SwgCuiInventoryContainer::T_datapadSelf:
			return ColumnInfo::findNameColumn(ColumnInfo::InventorySelf::ids, ColumnInfo::InventorySelf::numColumns);
		case SwgCuiInventoryContainer::T_craftIngredients:
			return ColumnInfo::findNameColumn(ColumnInfo::CraftIngredients::ids, ColumnInfo::CraftIngredients::numColumns);
		case SwgCuiInventoryContainer::T_draftSchematics:
			return ColumnInfo::findNameColumn(ColumnInfo::DraftSchematics::ids, ColumnInfo::DraftSchematics::numColumns);
		case SwgCuiInventoryContainer::T_waypointsSelf:
			return ColumnInfo::findNameColumn(ColumnInfo::Waypoints::ids, ColumnInfo::Waypoints::numColumns);
		case SwgCuiInventoryContainer::T_poi:
			return ColumnInfo::findNameColumn(ColumnInfo::Poi::ids, ColumnInfo::Poi::numColumns);
		default:
			break;
	}

	return 0;
}

//======================================================================
