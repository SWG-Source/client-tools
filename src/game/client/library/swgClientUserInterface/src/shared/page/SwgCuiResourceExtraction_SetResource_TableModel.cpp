//======================================================================
//
// SwgCuiResourceExtraction_SetResource_TableModel.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_SetResource_TableModel.h"

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


namespace SwgCuiResourceExtractionSetResourceTableModelNamespace
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
			
			data [I_none]       = ColumnData (StringId ("NONE"),                   UITableTypes::CT_none);
			data [I_icon]       = ColumnData (CuiStringIds::res_hopper_icon,       UITableTypes::CT_widget);
			data [I_name]       = ColumnData (CuiStringIds::res_hopper_name,       UITableTypes::CT_text);
			data [I_efficiency] = ColumnData (CuiStringIds::res_hopper_efficiency, UITableTypes::CT_integer);
		}

		namespace SetResourceSelf
		{
			const int numColumns = I_count-1;
			const Id ids [numColumns] =
			{
				I_icon,
				I_name,
				I_efficiency
			};
		}

		Id findColumnId (SwgCuiResourceExtraction_SetResource::TableModel::Type type, int col)
		{
			DEBUG_FATAL (col < 0, ("bad col"));

			switch (type)
			{
			case SwgCuiResourceExtraction_SetResource::TableModel::T_SetResourceSelf:
				DEBUG_FATAL (col >= ColumnInfo::SetResourceSelf::numColumns, ("bad col"));
				return ColumnInfo::SetResourceSelf::ids [col];
			default:
				return I_none;
			}
		}
	}
}

using namespace SwgCuiResourceExtractionSetResourceTableModelNamespace;

//----------------------------------------------------------------------

SwgCuiResourceExtraction_SetResource::TableModel::TableModel (SwgCuiResourceExtraction_SetResource & ownerMediator)
: UITableModel(),
  m_type (T_SetResourceSelf),
  m_ownerMediator (ownerMediator)
{
	SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::install ();
}

//----------------------------------------------------------------------

SwgCuiResourceExtraction_SetResource::TableModel::~TableModel ()
{
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_SetResource::TableModel::GetSortKeyAtInteger(int visualRow, int col, UITableTypes::CellType type, int & result) const
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

bool SwgCuiResourceExtraction_SetResource::TableModel::GetValueAtText   (int visualRow, int col, UIString & value) const
{
	//convert from possibly user-sorted view to original logical sort order
	const int row = GetLogicalDataRowIndex (visualRow);

	NetworkId resource = m_ownerMediator.getResourceAtPosition(row);

	const SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::Id id = SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::findColumnId (m_type, col);

	switch (id)
	{
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_name:
		{
			Unicode::String name;
			IGNORE_RETURN(ResourceTypeManager::createTypeDisplayLabel(resource, name));
			value = name;
		}
		break;
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_icon:
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_efficiency:
		DEBUG_WARNING(true, ("Mismatched data/table data (asked for text, but it isn't)"));
		break;
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_none:
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_SetResource::TableModel::GetValueAtBool   (int, int, bool &) const
{
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_SetResource::TableModel::GetValueAtInteger(int visualRow, int col, int & value) const
{
	//convert from possibly user-sorted view to original logical sort order
	const int row = GetLogicalDataRowIndex (visualRow);

	const SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::Id id = SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::findColumnId (m_type, col);

	switch (id)
	{
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_efficiency:
		{
			value = m_ownerMediator.getResourceEfficiencyAtPosition(row);
		}
		break;
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_icon:
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_name:
		DEBUG_WARNING(true, ("Mismatched data/table data (asked for int, but it isn't)"));
		break;
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_none:
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_count:
	default:
		return false;
	}
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_SetResource::TableModel::GetValueAtWidget (int visualRow, int col, UIWidget *& value) const
{
	//convert from possibly user-sorted view to original logical sort order
	const int row = GetLogicalDataRowIndex (visualRow);
	const SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::Id id = SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::findColumnId (m_type, col);

	switch (id)
	{
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_icon:
		{
			CuiWidget3dObjectListViewer * viewer = m_ownerMediator.getResourceIconAtPosition(row);
			value = viewer;
		}
		break;
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_name:
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_efficiency:
		DEBUG_WARNING(true, ("Mismatched data/table data (asked for widget, but it isn't)"));
		break;
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_none:
	case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_count:
	default:
		return false;
	}
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_SetResource::TableModel::GetLocalizedColumnName (int col, UIString & str) const
{
	if (col < 0 || col >= GetColumnCount ())
		return false;

	using namespace SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo;

	switch (m_type)
	{
	case T_SetResourceSelf:
		str = data [SetResourceSelf::ids [col]].name.localize ();
		break;
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiResourceExtraction_SetResource::TableModel::GetLocalizedColumnTooltip (int col, UIString & str) const
{
	return GetLocalizedColumnName (col, str);
}

//----------------------------------------------------------------------

UITableTypes::CellType SwgCuiResourceExtraction_SetResource::TableModel::FindColumnCellType     (int col) const
{
	if (col < 0 || col >= FindColumnCount ())
		return UITableTypes::CT_none;

	using namespace SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo;

	switch (m_type)
	{
	case T_SetResourceSelf:
		return data [SetResourceSelf::ids [col]].cellType;
	default:
		return UITableTypes::CT_none;
	}
}

//----------------------------------------------------------------------

int SwgCuiResourceExtraction_SetResource::TableModel::FindColumnCount        () const
{
	using namespace SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo;

	switch (m_type)
	{
	case T_SetResourceSelf:
		return SetResourceSelf::numColumns;
	default:
		return 0;
	}
}

//----------------------------------------------------------------------

int SwgCuiResourceExtraction_SetResource::TableModel::FindRowCount           () const
{
	return static_cast<int>(m_ownerMediator.getNumResources());
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_SetResource::TableModel::setType (Type type)
{
	m_type = type;
	fireColumnsChanged ();
	fireDataChanged ();
}

//----------------------------------------------------------------------

UIWidget * SwgCuiResourceExtraction_SetResource::TableModel::GetDragWidgetAt        (int, int )
{
	return NULL;
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_SetResource::TableModel::updateTableColumnSizes (UITable & table) const
{
	//-- copy vector
	UITable::ColumnSizeInfoVector csiv = table.GetColumnSizeInfo ();
	const int columnCount = FindColumnCount ();
	csiv.resize (static_cast<unsigned int>(columnCount));

	int numToDivide = columnCount;

	long widthToDivide = table.GetTotalColumnAvailableWidth ();

	for (int i = 0; i < columnCount; ++i)
	{
		const SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::Id id = SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::findColumnId (T_SetResourceSelf, i);
		UITable::ColumnSizeInfo & csi = csiv [static_cast<unsigned int>(i)];

		switch (id)
		{
		case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_icon:
			csi.constant = true;
			csi.width    = 80;
			widthToDivide -= csi.width;
			--numToDivide;
			break;
		case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_name:
			break;
		case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_efficiency:
			csi.constant = true;
			csi.width    = 80;
			widthToDivide -= csi.width;
			--numToDivide;
			break;
		case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_none:
		case SwgCuiResourceExtractionSetResourceTableModelNamespace::ColumnInfo::I_count:
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





