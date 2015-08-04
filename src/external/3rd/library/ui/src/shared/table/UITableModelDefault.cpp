// ======================================================================
//
// UITableModelDefault.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UITableModelDefault.h"

#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"
#include "UnicodeUtils.h"

#include <cassert>
#include <list>
#include <vector>

//----------------------------------------------------------------------

const char * const UITableModelDefault::TypeName                 = "TableModelDefault";

const UILowerString  UITableModelDefault::PropertyName::DataSourceContainer = UILowerString ("DataSourceContainer");

const UILowerString  UITableModelDefault::DataProperties::Value        = UILowerString ("Value");
const UILowerString  UITableModelDefault::DataProperties::LocalValue   = UILowerString ("LocalValue");
const UILowerString  UITableModelDefault::DataProperties::LocalTooltip = UILowerString ("LocalTooltip");
const UILowerString  UITableModelDefault::DataProperties::SortValue    = UILowerString ("SortValue");

//======================================================================================
#define _TYPENAME UITableModelDefault

namespace UITableModelDefaultNamespace
{
	namespace Properties
	{
		namespace ColumnDataSource
		{
			const UILowerString localLabel   = UILowerString ("LocalLabel");
			const UILowerString label        = UILowerString ("Label");
			const UILowerString tooltip      = UILowerString ("Tooltip");
			const UILowerString localTooltip = UILowerString ("LocalTooltip");
			const UILowerString type         = UILowerString ("Type");
			const UILowerString flags        = UILowerString ("Flags");
		}
	}

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DataSourceContainer, "", T_object),
	_GROUPEND(Basic, 1, 0);
	//================================================================

	bool isIntegerCompatibleType( UITableTypes::CellType const type )
	{
		return ( type == UITableTypes::CT_integer || type == UITableTypes::CT_percent || type == UITableTypes::CT_delimitedInteger );
	}
}
using namespace UITableModelDefaultNamespace;
//======================================================================================

UITableModelDefault::UITableModelDefault () :
UITableModel (),
mDataContainer (0)
{
}

//-----------------------------------------------------------------

UITableModelDefault::~UITableModelDefault ()
{
	SetDataSourceContainer (0);
}

//-----------------------------------------------------------------

bool UITableModelDefault::IsA( const UITypeID Type ) const
{
	return (Type == TUITableModelDefault) || UITableModel::IsA (Type);
}

//-----------------------------------------------------------------
void UITableModelDefault::GetLinkPropertyNames( UIPropertyNameVector & in ) const
{
	in.push_back (PropertyName::DataSourceContainer);

	UITableModel::GetLinkPropertyNames (in);

}

//----------------------------------------------------------------------

void UITableModelDefault::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UITableModel::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UITableModelDefault::GetPropertyNames( UIPropertyNameVector & in, bool forCopy ) const
{
	in.push_back (PropertyName::DataSourceContainer);

	UITableModel::GetPropertyNames (in, forCopy);
}

//-----------------------------------------------------------------

bool UITableModelDefault::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::DataSourceContainer)
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIDataSourceContainer );
		if( object || Value.empty() )
		{
			SetDataSourceContainer ( static_cast<UIDataSourceContainer *>(object) );
			return true;
		}
	}

	return UITableModel::SetProperty (Name, Value);
}

//-----------------------------------------------------------------

bool UITableModelDefault::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::DataSourceContainer)
	{
		if( mDataContainer )
		{
			GetPathTo( Value, mDataContainer );
			return true;
		}
	}

	return UITableModel::GetProperty (Name, Value);
}


//-----------------------------------------------------------------

void UITableModelDefault::SetDataSourceContainer (UIDataSourceContainer * container)
{
	if (mDataContainer)
		mDataContainer->StopListening (this);

	if (AttachMember (mDataContainer, container))
	{
		fireColumnsChanged ();
		fireDataChanged ();
	}

	if (mDataContainer)
		mDataContainer->Listen (this);
}

//-----------------------------------------------------------------

void UITableModelDefault::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code code )
{
	UI_UNREF (code);
	UI_UNREF (ContextObject);

	if (NotifyingObject == mDataContainer)
	{
		fireColumnsChanged ();

		/*
		if (ContextObject && ContextObject->GetParent () == mDataContainer && ContextObject->IsA (TUIDataSource))
		{
			fireColumnsChanged ();
		}
		else
		{
			fireColumnsChanged ();
		}
		*/
		fireDataChanged  ();
	}
}

//----------------------------------------------------------------------

UIDataSource * UITableModelDefault::GetColumnDataSource (int col)
{
	if (!mDataContainer || col < 0 || col >= static_cast<int>(mDataContainer->GetChildCount ()))
		return 0;

	UIBaseObject * const object = mDataContainer->GetChildByPositionLinear (col);
	if (!object)
		return 0;

	if (!object->IsA (TUIDataSource))
		return 0;

	return static_cast<UIDataSource *>(object);
}

//----------------------------------------------------------------------

const UIDataSource * UITableModelDefault::GetColumnDataSource (int col) const
{
	return const_cast<UITableModelDefault *>(this)->GetColumnDataSource (col);
}

//----------------------------------------------------------------------

UIData * UITableModelDefault::GetCellDataLogical (int row, int col)
{
	UIDataSource * const ds = GetColumnDataSource (col);

	if (!ds)
		return 0;

	if (row < 0 || row >= static_cast<int>(ds->GetChildCount ()))
		return 0;

	return ds->GetChildByPosition (row);
}

//----------------------------------------------------------------------

const UIData * UITableModelDefault::GetCellDataLogical (int row, int col) const
{
	return const_cast<UITableModelDefault *>(this)->GetCellDataLogical (row, col);
}

//----------------------------------------------------------------------

const UIData * UITableModelDefault::GetCellDataVisual      (int row, int col, UITableTypes::CellType type) const
{
	return const_cast<UITableModelDefault *>(this)->GetCellDataVisual (row, col, type);
}

//----------------------------------------------------------------------

UIData * UITableModelDefault::GetCellDataVisual (int row, int col, UITableTypes::CellType type)
{
	if (row < 0 || row >= FindRowCount ())
		return 0;

	const UITableTypes::CellType colType = GetColumnCellType (col);
	const bool integerCompatibleType = ( isIntegerCompatibleType(colType) && isIntegerCompatibleType(type) );

	if (type != UITableTypes::CT_none && colType != type && !integerCompatibleType)
	{
		assert (false);
		return 0;
	}

	row = GetLogicalDataRowIndex (row);
	assert (row >= 0);

	return GetCellDataLogical (row, col);
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetValueAtText   (int row, int col, UIString     & value) const
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_text);

	if (data)
		return data->GetProperty (DataProperties::LocalValue, value) || data->GetProperty (DataProperties::Value, value);

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetValueAtImage  (int row, int col, UIImageStyle *& value) const
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_image);

	if (data)
	{
		UIString str;
		if (data->GetProperty (DataProperties::Value, str))
		{
			value = static_cast<UIImageStyle *>(data->GetObjectFromPath (str, TUIImageStyle));
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetValueAtBool   (int row, int col, bool & value) const
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_bool);

	if (data)
		return data->GetPropertyBoolean (DataProperties::Value, value);

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetValueAtInteger(int row, int col, int & value) const
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_integer);

	if (data)
	{
		long l = 0;
		if (data->GetPropertyLong (DataProperties::Value, l))
		{
			value = static_cast<int>(l);
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetValueAtFloat  (int row, int col, float & value) const
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_float);

	if (data)
		return data->GetPropertyFloat (DataProperties::Value, value);


	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetValueAtWidget (int row, int col, UIWidget *& widget)     const
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_widget);

	if (data)
	{
		Unicode::String value;
		if (data->GetProperty (DataProperties::Value, value))
		{
			widget = static_cast<UIWidget *>(data->GetObjectFromPath (value, TUIWidget));
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::SetValueAtText         (int row, int col, const UIString     & value)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_text);

	if (data)
		return data->SetProperty (DataProperties::Value, value);

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::SetValueAtImage        (int row, int col, const UIImageStyle * value)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_image);

	if (data)
	{
		UIString str;
		data->GetPathTo (str, value);
		return data->SetProperty (DataProperties::Value, str);
	}

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::SetValueAtBool         (int row, int col, bool value)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_bool);

	if (data)
		return data->SetPropertyBoolean (DataProperties::Value, value);

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::SetValueAtWidget       (int row, int col, const UIWidget * value)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_widget);

	if (data)
	{
		UIString str;
		data->GetPathTo (str, value);
		return data->SetProperty (DataProperties::Value, str);
	}

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::SetValueAtInteger      (int row, int col, int value)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_integer);

	if (data)
		return data->SetPropertyInteger (DataProperties::Value, value);

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::SetValueAtFloat        (int row, int col, float value)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_float);

	if (data)
		return data->SetPropertyFloat (DataProperties::Value, value);

	return false;
}
//----------------------------------------------------------------------

bool UITableModelDefault::GetLocalizedColumnName (int col, UIString & str) const
{
	const UIDataSource * const ds = GetColumnDataSource (col);
	if (ds)
	{
		return ds->GetProperty (Properties::ColumnDataSource::localLabel, str) || ds->GetProperty (Properties::ColumnDataSource::label, str);
	}
	return 0;
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetLocalizedColumnTooltip (int col, UIString & str) const
{
	const UIDataSource * const ds = GetColumnDataSource (col);
	if (ds)
	{
		return ds->GetProperty (Properties::ColumnDataSource::localTooltip, str) || ds->GetProperty (Properties::ColumnDataSource::tooltip, str);
	}
	return 0;
}

//----------------------------------------------------------------------

int UITableModelDefault::FindRowCount () const
{
	int max = 0;
	for(int i = 0; i < FindColumnCount(); ++i)
	{
		const UIDataSource * const ds = GetColumnDataSource (i);
		int colCount = ds ? ds->GetChildCount () : 0;
		if(colCount > max)
			max = colCount;
	}

	return max;

}

//----------------------------------------------------------------------

UITableTypes::CellType UITableModelDefault::FindColumnCellType (int col) const
{
	const UIDataSource * const ds = GetColumnDataSource (col);
	if (ds)
	{
		UIString value;
		if (ds->GetProperty (Properties::ColumnDataSource::type, value))
		{
			for (int i = 0; i < UITableTypes::CT_numTypes; ++i)
			{
				if (Unicode::caseInsensitiveCompare (value, UITableTypes::CellTypeNames [i]))
					return static_cast<UITableTypes::CellType>(i);
			}
		}
	}

	return UITableTypes::CT_none;
}

//----------------------------------------------------------------------

int UITableModelDefault::FindColumnCellFlags (int col) const
{
	int result = 0;
	const UIDataSource * const ds = GetColumnDataSource (col);
	if (ds)
	{
		UIString value;
		if (ds->GetProperty (Properties::ColumnDataSource::flags, value))
		{
			result = UIUtils::hextoi<UIString::value_type>(value.c_str());
		}
	}

	return result;
}

//----------------------------------------------------------------------

int UITableModelDefault::FindColumnCount () const
{
	return mDataContainer ? mDataContainer->GetChildCount () : 0;
}

//----------------------------------------------------------------------

void UITableModelDefault::ClearData (UIDataSourceContainer & dsc)
{
	const UIDataSourceContainer::DataSourceBaseList & dsbl = dsc.GetDataSourceBaseList();

	for (UIDataSourceContainer::DataSourceBaseList::const_iterator it = dsbl.begin (); it != dsbl.end (); ++it)
	{
		if ((*it)->IsA (TUIDataSource))
		{
			UIDataSource * const ds = static_cast<UIDataSource *>(*it);
			ds->Clear ();
		}
	}
}

//----------------------------------------------------------------------

void UITableModelDefault::ClearData ()
{
	if (!mDataContainer)
		return;

	mDataContainer->StopListening (this);

	ClearData (*mDataContainer);

	mDataContainer->Listen (this);
}

//----------------------------------------------------------------------

UIDataSource * UITableModelDefault::GetColumnDataSource (const char * name)
{
	assert (name);

	if (!mDataContainer)
		return 0;

	UIBaseObject * const object = mDataContainer->GetChild (name);

	if (!object->IsA (TUIDataSource))
		return 0;

	return static_cast<UIDataSource *>(object);
}

//----------------------------------------------------------------------

const UIDataSource * UITableModelDefault::GetColumnDataSource (const char * name) const
{
	return const_cast<UITableModelDefault *>(this)->GetColumnDataSource (name);
}

//----------------------------------------------------------------------

UIData * UITableModelDefault::AppendCell (int col, const char * const name, const Unicode::String & value)
{
	UIDataSource * const ds = GetColumnDataSource (col);
	if (ds)
	{
		UIData * const data = new UIData;
		if (name)
			data->SetName (name);
		data->SetProperty (UITableModelDefault::DataProperties::Value, value);
		ds->AddChild (data);
		return data;
	}

	return 0;
}

//----------------------------------------------------------------------

void UITableModelDefault::ClearTable()
{
	ClearData();
	fireColumnsChanged();
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetTooltipAt              (int row, int col, UIString & tooltip) const
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_none);

	if (data)
	{
		if (data->GetProperty (DataProperties::LocalTooltip, tooltip))
			return true;
	}

	return UITableModel::GetTooltipAt (row, col, tooltip);
}

//----------------------------------------------------------------------

bool UITableModelDefault::SetTooltipAt              (int row, int col, const UIString & tooltip)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_none);

	if (data)
	{
		return data->SetProperty (DataProperties::LocalTooltip, tooltip);
	}

	return false;

}

//----------------------------------------------------------------------

bool UITableModelDefault::SetSortKeyAtString       (int row, int col, const UIString     & value)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_none);

	if (data)
	{
		return data->SetProperty (DataProperties::SortValue, value);
	}

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::SetSortKeyAtInteger    (int row, int col, int value)
{
	UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_none);

	if (data)
	{
		return data->SetPropertyInteger (DataProperties::SortValue, value);
	}

	return false;
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetSortKeyAtString       (int row, int col, UITableTypes::CellType type, Unicode::String & value) const 
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_none);

	if (data)
	{
		if (data->GetProperty (DataProperties::SortValue, value))
			return true;
	}

	return UITableModel::GetSortKeyAtString (row, col, type, value);
}

//----------------------------------------------------------------------

bool UITableModelDefault::GetSortKeyAtInteger    (int row, int col, UITableTypes::CellType type, int & value) const 
{
	const UIData * const data = GetCellDataVisual (row, col, UITableTypes::CT_none);

	if (data)
	{
		if (data->GetPropertyInteger (DataProperties::SortValue, value))
			return true;
	}

	return UITableModel::GetSortKeyAtInteger (row, col, type, value);
}

// ======================================================================
