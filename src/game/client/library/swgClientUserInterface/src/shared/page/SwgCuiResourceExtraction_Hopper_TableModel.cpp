//======================================================================
//
// SwgCuiResourceExtractionHopperTableModel.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Hopper_TableModel.h"

#include "sharedFoundation/CrcString.h"
#include "sharedObject/CachedNetworkId.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGraphics/Texture.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "swgClientUserInterface/SwgCuiSurvey.h"

#include "UITable.h"

//======================================================================


namespace SwgCuiResourceExtractionHopperTableModelNamespace
{
	namespace ColumnInfo
	{
		enum Id
		{
			I_none,
			I_icon,
			I_name,
			I_efficiency,
			I_count
		};
		
		struct ColumnData
		{
			StringId            name;
			UITableTypes::CellType cellType;

			ColumnData ()
			: name (),
			  cellType (UITableTypes::CT_none)
			{
			}

			ColumnData (const StringId & _name, UITableTypes::CellType _cellType)
			: name (_name),
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
			
			data [I_none]       = ColumnData (StringId ("NONE"),                  UITableTypes::CT_none);
			data [I_icon]       = ColumnData (CuiStringIds::res_hopper_icon,      UITableTypes::CT_widget);
			data [I_name]       = ColumnData (CuiStringIds::res_hopper_name,      UITableTypes::CT_text);
			data [I_efficiency] = ColumnData (CuiStringIds::res_hopper_amount,    UITableTypes::CT_integer);
		}

		namespace HopperSelf
		{
			const int numColumns = I_count-1;
			const Id ids [numColumns] =
			{
				I_icon,
				I_name,
				I_efficiency
			};
		}

		Id findColumnId (SwgCuiResourceExtraction_Hopper::TableModel::Type type, int col)
		{
			DEBUG_FATAL (col < 0, ("bad col"));

			switch (type)
			{
			case SwgCuiResourceExtraction_Hopper::TableModel::T_hopperSelf:
				DEBUG_FATAL (col >= ColumnInfo::HopperSelf::numColumns, ("bad col"));
				return ColumnInfo::HopperSelf::ids [col];
			default:
				return I_none;
			}
		}
	}
}

using namespace SwgCuiResourceExtractionHopperTableModelNamespace;

//----------------------------------------------------------------------

SwgCuiResourceExtraction_Hopper::TableModel::TableModel (SwgCuiResourceExtraction_Hopper & ownerMediator)
: UITableModel(),
  m_type (T_hopperSelf),
  m_ownerMediator (ownerMediator)
{
	ColumnInfo::install ();
}

//----------------------------------------------------------------------

SwgCuiResourceExtraction_Hopper::TableModel::~TableModel ()
{
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_Hopper::TableModel::GetValueAtText   (int visualRow, int col, UIString & value)  const
{
	const int row = GetLogicalDataRowIndex (visualRow);

	NetworkId resource = m_ownerMediator.getResourceAtPosition(row);
	if(resource == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Can't get resource at row %d", row));
		return false;
	}

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	switch (id)
	{
	case ColumnInfo::I_name:
		{
			Unicode::String name;
			IGNORE_RETURN(ResourceTypeManager::createTypeDisplayLabel(resource, name));
			value = name;
		}
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_icon:
	case ColumnInfo::I_efficiency:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_Hopper::TableModel::GetValueAtBool   (int, int, bool &) const
{
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_Hopper::TableModel::GetValueAtInteger(int visualRow, int col, int & value) const
{
	const int row = GetLogicalDataRowIndex (visualRow);

	NetworkId resource = m_ownerMediator.getResourceAtPosition(row);

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	switch (id)
	{
	case ColumnInfo::I_efficiency:
		{
			value = static_cast<int>(floor(m_ownerMediator.getHopperSizeForResource(resource)));
			DEBUG_WARNING(value == 0, ("Can't get resource hopper size at row %d", row));
		}
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_icon:
	case ColumnInfo::I_name:
	case ColumnInfo::I_count:
	default:
		return false;
	}
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_Hopper::TableModel::GetValueAtWidget (int visualRow, int col, UIWidget *& value) const
{
	const int row = GetLogicalDataRowIndex (visualRow);
	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	switch (id)
	{
	case ColumnInfo::I_icon:
		{
			CuiWidget3dObjectListViewer * viewer = m_ownerMediator.getResourceIconAtPosition(row);
			if(!viewer)
			{
				DEBUG_WARNING(true, ("Can't get resource icon at row %d", row));
				value = 0;
				return false;
			}
			value = viewer;
		}
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_name:
	case ColumnInfo::I_efficiency:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_Hopper::TableModel::GetSortKeyAtInteger(int visualRow, int col, UITableTypes::CellType type, int & result) const
{
	const int row = GetLogicalDataRowIndex (visualRow);

	//sort resource icons by template name
	if(type == UITableTypes::CT_widget)
	{
		NetworkId resource = m_ownerMediator.getResourceAtPosition(row);
		CuiWidget3dObjectListViewer* viewer = m_ownerMediator.getResourceIconAtPosition(row);
		Object* o = viewer->getRenderObjectByIndex(0);
		if(o)
		{
			const Texture* t = ResourceIconManager::fetchTextureForType(resource);
			const CrcString & s = t->getCrcString();
			result = s.getCrc();
			t->release();
			return true;
		}
		return false;
	}
	else
		return UITableModel::GetSortKeyAtInteger(visualRow, col, type, result);
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_Hopper::TableModel::GetLocalizedColumnName (int col, UIString & str) const
{
	if (col < 0 || col >= GetColumnCount ())
		return false;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case T_hopperSelf:
		str = data [HopperSelf::ids [col]].name.localize ();
		break;
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_Hopper::TableModel::GetLocalizedColumnTooltip (int col, UIString & str) const
{
	return GetLocalizedColumnName (col, str);
}

//----------------------------------------------------------------------

UITableTypes::CellType SwgCuiResourceExtraction_Hopper::TableModel::FindColumnCellType     (int col) const
{
	if (col < 0 || col >= FindColumnCount ())
		return UITableTypes::CT_none;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case T_hopperSelf:
		return data [HopperSelf::ids [col]].cellType;
	default:
		return UITableTypes::CT_none;
	}
}

//----------------------------------------------------------------------

int SwgCuiResourceExtraction_Hopper::TableModel::FindColumnCount        () const
{
	using namespace ColumnInfo;

	switch (m_type)
	{
	case T_hopperSelf:
		return HopperSelf::numColumns;
	default:
		return 0;
	}
}

//----------------------------------------------------------------------

int SwgCuiResourceExtraction_Hopper::TableModel::FindRowCount           () const
{
	return static_cast<int>(m_ownerMediator.getNumResources());
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::TableModel::setType (Type type)
{
	m_type = type;
	fireColumnsChanged ();
	fireDataChanged ();
}

//----------------------------------------------------------------------

UIWidget * SwgCuiResourceExtraction_Hopper::TableModel::GetDragWidgetAt        (int, int )
{
	return NULL;
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::TableModel::updateTableColumnSizes (UITable & table) const
{
	//-- copy vector
	UITable::ColumnSizeInfoVector csiv = table.GetColumnSizeInfo ();
	const int columnCount = FindColumnCount ();
	csiv.resize (static_cast<unsigned int>(columnCount));

	int numToDivide = columnCount;

	long widthToDivide = table.GetTotalColumnAvailableWidth ();

	for (int i = 0; i < columnCount; ++i)
	{
		const ColumnInfo::Id id = ColumnInfo::findColumnId (T_hopperSelf, i);
		UITable::ColumnSizeInfo & csi = csiv [static_cast<unsigned int>(i)];

		switch (id)
		{
		case ColumnInfo::I_icon:
			csi.constant = true;
			csi.width    = 80;
			widthToDivide -= csi.width;
			--numToDivide;
			break;
		case ColumnInfo::I_name:
			break;
		case ColumnInfo::I_efficiency:
			csi.constant = true;
			csi.width    = 80;
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
				UITable::ColumnSizeInfo & csi2 = *it;
				csi2.proportion = prop;
			}
		}
	}

	table.SetColumnSizeInfo (csiv);
}

//======================================================================

