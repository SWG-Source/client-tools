// ======================================================================
//
// UITableModelDefault.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UITableModelDefault_H
#define INCLUDED_UITableModelDefault_H

// ======================================================================

#include "UITableModel.h"

//-----------------------------------------------------------------

class UIDataSourceContainer;
class UIDataSource;

//-----------------------------------------------------------------

class UITableModelDefault :
public UITableModel,
public UINotification
{
public:

	static const char * const TypeName;

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString  DataSourceContainer;
	};


	class DataProperties
	{
	public:
		static const UILowerString  Value;
		static const UILowerString  LocalValue;
		static const UILowerString  LocalTooltip;
		static const UILowerString  SortValue;
	};

	UITableModelDefault ();
	~UITableModelDefault ();

	virtual bool                  IsA                    (const UITypeID Type) const;
	virtual const char *          GetTypeName            () const              { return TypeName; }
	virtual UIBaseObject *        Clone                  () const              { return new UITableModelDefault; }

	virtual void                  GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                  GetPropertyNames       (UIPropertyNameVector &, bool forCopy) const;
	virtual void                  GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool                  SetProperty            (const UILowerString & Name, const UIString &Value);
	virtual bool                  GetProperty            (const UILowerString & Name, UIString &Value) const;

	void                          SetDataSourceContainer (UIDataSourceContainer * folder);
	UIDataSourceContainer *       GetDataSourceContainer ();

	virtual void                  Notify                 (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode);

	bool                          GetValueAtText         (int row, int col, UIString     & value)  const;
	bool                          GetValueAtImage        (int row, int col, UIImageStyle *& value) const;
	bool                          GetValueAtBool         (int row, int col, bool & value)          const;
	bool                          GetValueAtInteger      (int row, int col, int & value)           const;
	bool                          GetValueAtFloat        (int row, int col, float & value)         const;
	bool                          GetValueAtWidget       (int row, int col, UIWidget *& widget)    const;

	bool                          SetValueAtText         (int row, int col, const UIString     & value);
	bool                          SetValueAtImage        (int row, int col, const UIImageStyle * value);
	bool                          SetValueAtBool         (int row, int col, bool value);
	bool                          SetValueAtWidget       (int row, int col, const UIWidget * value);
	bool                          SetValueAtInteger      (int row, int col, int value);
	bool                          SetValueAtFloat        (int row, int col, float value);

	bool                          SetSortKeyAtString     (int row, int col, const UIString     & value);
	bool                          SetSortKeyAtInteger    (int row, int col, int value);

	bool                          GetSortKeyAtString     (int row, int col, UITableTypes::CellType type, Unicode::String & value) const;
	bool                          GetSortKeyAtInteger    (int row, int col, UITableTypes::CellType typ, int & value) const;

	virtual bool                  GetLocalizedColumnName    (int col, UIString & str) const;
	virtual bool                  GetLocalizedColumnTooltip (int col, UIString & str) const;

	void                          ClearData              ();
	UIDataSource *                GetColumnDataSource    (const char * name);
	UIDataSource *                GetColumnDataSource    (int col);
	UIData *                      GetCellDataLogical     (int row, int col);
	UIData *                      GetCellDataVisual      (int row, int col, UITableTypes::CellType type = UITableTypes::CT_none);

	const UIDataSource *          GetColumnDataSource    (const char * name) const;
	const UIDataSource *          GetColumnDataSource    (int col) const;
	const UIData *                GetCellDataLogical     (int row, int col) const;
	const UIData *                GetCellDataVisual      (int row, int col, UITableTypes::CellType type = UITableTypes::CT_none) const;

	static void                   ClearData              (UIDataSourceContainer & dsc);

	UIData *                      AppendCell             (int col, const char * const name, const Unicode::String & value);
	void                          ClearTable             ();

	virtual bool                  GetTooltipAt              (int row, int col, UIString & tooltip) const;
	virtual bool                  SetTooltipAt              (int row, int col, const UIString & tooltip);

protected:

	virtual UITableTypes::CellType FindColumnCellType    (int col) const;
	virtual int                    FindColumnCellFlags   (int col) const;
	virtual int                    FindColumnCount       () const;
	virtual int                    FindRowCount          () const;

private:
	UITableModelDefault (const UITableModelDefault & rhs);
	UITableModelDefault & operator= (const UITableModelDefault & rhs);

	UIDataSourceContainer *       mDataContainer;
};

//-----------------------------------------------------------------

inline UIDataSourceContainer *  UITableModelDefault::GetDataSourceContainer ()
{
	return mDataContainer;
}

// ======================================================================

#endif
